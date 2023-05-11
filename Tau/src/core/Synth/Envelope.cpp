#include"Envelope.h"
#include"UnitTransform.h"
namespace tau
{
	Envelope::Envelope(EnvelopeType envType)
		:type(envType)
	{
	}

	Envelope::~Envelope()
	{

	}

	void Envelope::Clear()
	{
		amp = 1;
		delaySec = 0;
		attackSec = 0.001f;
		sustainY = 1;
		releaseSec = 0.001f;
		keyToHold = 0;
		keyToDecay = 0;
		curtStage = EnvStage::Stop;
		openSec = 0;
		onKey = false;
		noteKeyNum = 60;
		offKeySec = 0;
		baseSec = 0;
		realDecaySec = 0;
		realHoldSec = 0;
		curtValue = 0;
		curtSec = 0;
		isFastRelease = false;
		oldSec = -1;
		oldOutput = 0;
	}

	// 启动
	void Envelope::OnKey(int noteKey, float sec)
	{
		curtStage = EnvStage::Delay;
		onKey = true;
		noteKeyNum = noteKey;
		openSec = sec;
		offKeySec = 0;
		Reset(openSec);
	}


	// 松开按键
	// <param name="sec">松开按键的时间点，秒</param>
	void Envelope::OffKey(float sec, float releaseSec)
	{
		if (onKey == false)
		{
			if (releaseSec < 0)
				return;

			StageRangeInfo& range = stageRangeInfo[(int)EnvStage::Release];
			if (range.xmax < releaseSec ||
				abs(releaseSec - range.xmax) < 0.0001)
				return;

			isFastRelease = true;
			SetStageRangeInfo(range, 0, releaseSec, 0, curtValue);
			offKeySec = sec - openSec;
			return;
		}

		onKey = false;
		offKeySec = sec - openSec;
		//
		StageRangeInfo& range = stageRangeInfo[(int)EnvStage::Release];
		if (curtStage == EnvStage::Delay)
		{
			SetStageRangeInfo(range, 0, 0, 0, 0);
		}
		else
		{
			if (releaseSec >= 0 &&
				releaseSec < range.xmax)
			{
				isFastRelease = true;
				SetStageRangeInfo(range, 0, releaseSec, 0, curtValue);
			}
			else
			{
				SetStageRangeInfo(range, 0, range.xmax, 0, curtValue);
			}
		}
	}


	// 根据时间点获取包络线的值
	// <param name="sec">秒</param>
	float Envelope::GetEnvValue(float sec)
	{
	//	if (sec == oldSec)
	//		return oldOutput;

		curtSec = sec;
		if (curtStage == EnvStage::Stop)
		{
			curtValue = 0;
			return curtValue;
		}

		bool isSustainStage = true;
		//按下按键
		if (onKey)
		{
			sec = sec + baseSec - openSec;

			if (curtStage != EnvStage::Sustain)
			{
				for (int i = (int)curtStage; i < (int)EnvStage::Sustain; i++)
				{
					if (sec >= stageRangeInfo[i].xmin && sec <= stageRangeInfo[i].xmax)
					{
						curtStage = (EnvStage)i;
						curtValue = ComputeStageValueY(curtStage, sec);
						isSustainStage = false;

					}
				}
			}

			if (isSustainStage == true)
			{
				curtStage = EnvStage::Sustain;
				curtValue = ComputeStageValueY(curtStage, sec);
			}
		}
		else
		{
			//在包络线的任何阶段松开按键，都将直接进入释音阶段   
			sec = sec - offKeySec;
			curtStage = EnvStage::Release;
			if (sec <= stageRangeInfo[(int)curtStage].xmax)
			{
				curtValue = ComputeStageValueY(curtStage, sec);
			}
			else
			{
				curtStage = EnvStage::Stop;
				curtValue = 0;
			}
		}


		oldSec = sec;
		oldOutput = curtValue;;

		return curtValue;
	}


	// 生成包络线
	void Envelope::Create()
	{
		Reset();

		curtStage = EnvStage::Stop;
		onKey = false;
		offKeySec = 0;
		openSec = 0;
		curtValue = 0;
	}


	// 重设包络线
	// 并根据给定的当前时间点，设置包络线所处阶段
	// <param name="sec">时间点</param>
	void Envelope::Reset(float sec)
	{
		CalRealHoldSec();
		CalRealDecaySec();
		baseSec = 0;
		curtSec = sec;
		isFastRelease = false;

		//当包络线处于延音阶段时，采样音会一直在某个采样段循环播放，直到松开按键后，将进入释音阶段
		//此时声音的时间会重新以0点为参考
		//注意在包络线的任何阶段松开按键，都将直接进入释音阶段
		float attackEndSec = delaySec + attackSec;
		float holdEndSec = attackEndSec + realHoldSec;
		float decayEndSec = holdEndSec + realDecaySec;
		SetStageRangeInfo(stageRangeInfo[(int)(EnvStage::Delay)], 0, delaySec, 0, 0);
		SetStageRangeInfo(stageRangeInfo[(int)(EnvStage::Attack)], delaySec, attackEndSec, 0, 1);
		SetStageRangeInfo(stageRangeInfo[(int)(EnvStage::Hold)], attackEndSec, holdEndSec, 1, 1);
		SetStageRangeInfo(stageRangeInfo[(int)(EnvStage::Decay)], holdEndSec, decayEndSec, sustainY, 1);
		SetStageRangeInfo(stageRangeInfo[(int)(EnvStage::Sustain)], 0, 0, sustainY, sustainY);
		SetStageRangeInfo(stageRangeInfo[(int)(EnvStage::Release)], 0, releaseSec, 0, sustainY);


		//根据给定的时间点，设置包络线所处阶段
		SetCurtStage(sec);
	}


    // 范围：-1200（每键号减少 1 个八度时间）至 1200（每键号增加 1 个八度时间）。
	// 修正后的HoldEnv时间(timecents) = HoldEnv + (KeynumToEnvHold * (OnKeyNumber - 60))
	// 键号 60（中央 C）：保持时间不变。
	// 键号 > 60：保持时间随键号升高按比例缩短（若 KeynumToEnvHold 为正值）。
	// 键号 < 60：保持时间随键号降低按比例延长（若 KeynumToEnvHold 为正值）。
	//设计意图：
	//模拟真实乐器特性：例如钢琴高音区衰减更快,低音区衰减更慢。
	//动态音色控制：结合 HoldEnv 和 KeynumToEnvDecay，实现音色随音高变化的复杂动态响应。
	void Envelope::CalRealHoldSec()
	{
		if (keyToHold == 0 || onKey == false)
		{
			realHoldSec = UnitTransform::TimecentsToSec(holdTimecents);
			return;
		}

		short n = holdTimecents + keyToHold * (noteKeyNum - 60);
		realHoldSec = UnitTransform::TimecentsToSec(n);
	}


	// 键位对音量包络衰减时间的调节
	// MIDI 键号升高时，音量包络衰减时间缩短的速率（以 时间厘/键位单位 为单位）
	// 单位：时间厘/键位（Timecents per KeyNumber），每单位键号变化对应的时间厘调整量
	// 范围：-1200 至 1200，负值表示键号升高时衰减时间延长，正值表示缩短。
	// 默认值：0 → 衰减时间不随键号变化，始终采用 DecayEnv的设定值。
	// 键号 60（中央 C） 的衰减时间始终不变（由 DecayVolEnv 直接控制）。
	// 修正后的DecayEnv时间(timecents) = DecayEnv + (KeynumToEnvDecay * (OnKeyNumber - 60))
	// 结合 SustainEnv（持续电平）和 ReleaseEnv（释放时间），可模拟钢琴、吉他等乐器的自然衰减特性。
	// 例如，高音区快速衰减至 50% 电平，低音区缓慢衰减至 20% 电平。
	void Envelope::CalRealDecaySec()
	{
		if (keyToDecay == 0 || onKey == false)
		{
			realDecaySec = UnitTransform::TimecentsToSec(decayTimecents);
			return;
		}

		short n = decayTimecents + keyToDecay * (noteKeyNum - 60);
		realDecaySec = UnitTransform::TimecentsToSec(n);
	}


	//  根据给定的时间点，设置包络线所处阶段
	// <param name="sec">时间点</param>
	void Envelope::SetCurtStage(float sec)
	{
		curtSec = sec;
		if (sec - openSec < 0)
		{
			curtStage = EnvStage::Stop;
			return;
		}

		if (onKey == false)
		{
			sec -= offKeySec;
			StageRangeInfo& range = stageRangeInfo[(int)EnvStage::Release];
			if (sec >= range.xmin && sec <= range.xmax)
				curtStage = EnvStage::Release;
			else
				curtStage = EnvStage::Stop;
		}
		else
		{
			sec -= openSec;
			for (int i = (int)EnvStage::Delay; i < (int)EnvStage::Sustain; i++)
			{
				if (sec >= stageRangeInfo[i].xmin && sec <= stageRangeInfo[i].xmax)
				{
					curtStage = (EnvStage)i;
					return;
				}
			}

			curtStage = EnvStage::Sustain;
		}
	}


	float Envelope::ComputeStageValueY(EnvStage stage, float x)
	{
		StageRangeInfo range = stageRangeInfo[(int)stage];
		//float xNormal = (x - range.xmin) / (range.xmax - range.xmin)
		float xNormal = 0;
		if (range.xRangeWidth != 0)
			xNormal = (x - range.xmin) * range.xRangeWidthInv;

		float y;

		switch (stage)
		{
		case EnvStage::Delay:
			y = 0;
			break;

		case EnvStage::Attack:
			//y = xNormal < 1 ? 1 - pow(10.0f, -(xNormal * 20 * 0.05f)) : 1;
			if (attackSec < 0.4f)
				y = powf(xNormal, 4);
			else
				y = xNormal;

			break;

		case EnvStage::Hold:
			y = 1;
			break;

		case EnvStage::Sustain:
			y = range.ymax;
			break;

		case EnvStage::Decay:
			if (type == EnvelopeType::Vol)
				y = xNormal < 1 ? FastPow2(-(xNormal * 12)) : 0;
			else
				y = 1 - powf(xNormal, 0.8);

			y = range.ymin + y * range.yRangeWidth;
			break;
		case EnvStage::Release:
		{
			//y = xNormal < 1 ? pow(10.0f, -(xNormal * 200 * 0.05f)) : 0;
			//if (!isFastRelease)
			//	y = xNormal < 1 ? FastPow2(-(xNormal * 20)) : 0;
			//else
			//y = 1 - xNormal;

			//float x = 2 * xNormal - 1;
			//y = (-(6 * x) / (1 + 5 * abs(x)) - 1) * 0.5f + 1;

			y = xNormal < 1 ? powf(2, -(xNormal * 15)) : 0;
			y = range.ymin + y * range.yRangeWidth;

		}
		break;


		default:
			y = 0;
			break;
		}

		return y;
	}


	void Envelope::SetStageRangeInfo(StageRangeInfo& range, float xmin, float xmax, float ymin, float ymax)
	{
		range.xmin = xmin;
		range.xmax = xmax;
		range.xRangeWidth = xmax - xmin;
		if (range.xRangeWidth == 0) { range.xRangeWidthInv = 9999999; }
		else { range.xRangeWidthInv = 1 / range.xRangeWidth; }
		range.ymin = ymin;
		range.ymax = ymax;
		range.yRangeWidth = ymax - ymin;
	}

}
