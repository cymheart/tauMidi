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
		stage = EnvStage::Stop;
		delaySec = 0;
		attackSec = 0.001f;
		sustain = 0;
		releaseSec = 0.001f;
		orgReleaseSec = -1;
		keyToHold = 0;
		keyToDecay = 0;
		openSec = 0;
		onKey = false;
		noteKeyNum = 60;
		baseSec = 0;
		realDecaySec = 0;
		realHoldSec = 0;
		curtValue = -100;
		curtSec = 0;
		oldSec = -1;
		oldOutput = 0;
		holdTimecents = -12000;
		decayTimecents = -12000;
	}

	
	// 生成包络线
	void Envelope::Create()
	{
		Reset();

		onKey = false;
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

		if (type == EnvelopeType::Vol) {
			sustainMin = -144;
			sustainMax = 0;
		}else {
			sustainMin = 0;
			sustainMax = 1;
		}

		sustain = max(sustain, sustainMin);
		sustain = min(sustain, sustainMax);

		//当包络线处于延音阶段时，采样音会一直在某个采样段循环播放，直到松开按键后，将进入释音阶段
		//此时声音的时间会重新以0点为参考
		//注意在包络线的任何阶段松开按键，都将直接进入释音阶段
		attackEndSec = delaySec + attackSec;
		holdEndSec = attackEndSec + realHoldSec;
		decayEndSec = holdEndSec + realDecaySec;
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


	// 启动
	void Envelope::OnKey(int noteKey, float sec)
	{
		if (stage != EnvStage::Stop)
			return;

		stage = EnvStage::Delay;
		onKey = true;
		noteKeyNum = noteKey;
		openSec = sec;
		
		if(orgReleaseSec > 0)
			releaseSec = orgReleaseSec;

		Reset(openSec);
		//
		GetEnvValue(sec);
	}


	// 松开按键
	// <param name="sec">松开按键的时间点，秒</param>
	// <param name="releaseSec">重设按键释放时长</param>
	void Envelope::OffKey(float sec, float resetReleaseSec)
	{
		if (stage == EnvStage::Stop)
			return;

		stage = EnvStage::Release;

		if (onKey) {
			onKey = false;
			if (resetReleaseSec < 0)
				return;

			if (orgReleaseSec < 0)
				orgReleaseSec = releaseSec;
			releaseSec = resetReleaseSec;
		}


		//当按键已经松开后，下面代码只调整键释放时长
		if (resetReleaseSec < 0)
			return;

		sec = baseSec + sec - openSec;
		//当要重设的释放时长大于目前释放时长时，将不从设释放时长
		if (releaseEndSec - sec < resetReleaseSec)
			return;

		//根据当前值和时间点，动态设置释放时间范围和值范围
		releaseStartSec = sec;
		releaseMaxEnvValue = curtValue;
		releaseEndSec = releaseStartSec + resetReleaseSec;	
	}


	// 根据时间点获取包络线的值
	float Envelope::GetEnvValue(float sec)
	{

		if (stage == EnvStage::Stop)
			return sustainMin;

		curtValue = sustainMin;
		curtSec = baseSec + sec - openSec;

		if (onKey) 
		{
			if (curtSec < delaySec) { //Delay阶段
				stage = EnvStage::Delay;
				curtValue = sustainMin;
			}
			else if (curtSec < attackEndSec) //attack阶段
			{
				stage = EnvStage::Attack;
				float t = (curtSec - delaySec) / (attackEndSec - delaySec);
				curtValue = sustainMin + (sustainMax - sustainMin) * t;

			}
			else if (curtSec < holdEndSec) //hold阶段
			{
				stage = EnvStage::Hold;
				curtValue = sustainMax;
			}
			else if (curtSec < decayEndSec) //decay阶段
			{
				stage = EnvStage::Decay;
				float t = (curtSec - holdEndSec) / (decayEndSec - holdEndSec);
				curtValue = sustainMax + (sustain - sustainMax) * t;
			}
			else {  //sustain阶段
				stage = EnvStage::Sustain;
				curtValue = sustain;
			}

			//根据当前值和时间点，动态设置释放时间范围和值范围
			releaseStartSec = curtSec;
			releaseMaxEnvValue = curtValue;
			releaseEndSec = releaseStartSec + releaseSec;		
		}
		else if(curtSec < releaseEndSec){  //release阶段
			stage = EnvStage::Release;
			float t = (curtSec - releaseStartSec) / releaseSec;
			curtValue = releaseMaxEnvValue + (sustainMin - releaseMaxEnvValue) * t;

			if (curtValue == sustainMin)
				stage = EnvStage::Stop;
		}
		else if (curtValue == sustainMin) {
			stage = EnvStage::Stop;
		}

		return curtValue;
	}
}
