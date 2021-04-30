#include"Envelope.h"
#include"UnitTransform.h"
namespace ventrue
{
	Envelope::Envelope()
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
		holdSec = 0.001f;
		decaySec = 0.001f;
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


	// 按键的keyToHold系数影响保持时长
	// keyToHold为百分比值，比如： 取1即为100%, 取0.25即为25%
	// 按一个八度差，音时长改变2倍
	// newHoldSec = 2^(keyToHold * nOctave) * holdSec
	// 例如:
	// 当前按键键号为40，则相对基键60，相差60 - 40 = 20个半音，即为 20/12 = 1.66667个Octave，即nOcatve = 1.66667
	// keyToHold = 0.5, holdSec = 1.3,
	// 则 newHoldSec = 2^(0.5 * 1.66667) * 1.3 = 2.31634s
	// 再例:
	// 当前按键键号为80，则相对基键60，相差60 - 80 = -20个半音，即为 -20/12 = -1.66667个Octave，即nOcatve = -1.66667
	// keyToHold = 0.5, holdSec = 1.3,
	// 则 newHoldSec = 2^(0.5 * -1.66667) * 1.3 = 0.729599s
	void Envelope::CalRealHoldSec()
	{
		if (keyToHold == 0 || onKey == false)
		{
			realHoldSec = holdSec;
			return;
		}

		float nOctave = (float)(60.0 - (float)noteKeyNum) / 12.0f;
		realHoldSec = (float)pow(2, keyToHold * nOctave) * holdSec;
	}


	// 按键的keyToDecay系数影响衰减时长
	// keyToDecay为百分比值，比如： 取1即为100%, 取0.25即为25%
	// 按一个八度差，音时长改变2倍
	// newDecaySec = 2^(keyToDecay * nOctave) * decaySec
	// 例如:
	// 当前按键键号为40，则相对基键60，相差60 - 40 = 20个半音，即为 20/12 = 1.66667个Octave，即nOcatve = 1.66667
	// keyToDecay = 0.5, decaySec = 1.3,
	// 则 newDecaySec = 2^(0.5 * 1.66667) * 1.3 = 2.31634s
	// 再例:
	// 当前按键键号为61，则相对基键60，相差60 - 61 = -1个半音，即为 -1/12 = -0.0833333个Octave，即nOcatve = -0.0833333
	// keyToDecay = 1, decaySec = 1,
	// 则 newDecaySec = 2^(1 * -0.0833333) * 1 = 0.943874s
	void Envelope::CalRealDecaySec()
	{
		if (keyToDecay == 0 || onKey == false)
		{
			realDecaySec = decaySec;
			return;
		}

		float nOctave = (60.0f - (float)noteKeyNum) / 12.0f;
		realDecaySec = (float)pow(2, keyToDecay * nOctave) * decaySec;
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
				y = pow(xNormal, 4);
			else
				y = 1;

			break;

		case EnvStage::Hold:
			y = 1;
			break;

		case EnvStage::Sustain:
			y = range.ymax;
			break;

		case EnvStage::Decay:
		case EnvStage::Release:

			//y = xNormal < 1 ? pow(10.0f, -(xNormal * 200 * 0.05f)) : 0;
			//if (!isFastRelease)
			//	y = xNormal < 1 ? FastPow2(-(xNormal * 20)) : 0;
			//else
			//	y = 1 - xNormal;
			y = xNormal < 1 ? FastPow2(-(xNormal * 20)) : 0;
			y = range.ymin + y * range.yRangeWidth;
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
