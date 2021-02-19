#include"EffectCompressor.h"

namespace ventrue
{
	EffectCompressor::EffectCompressor()
	{
		leftCompressor = new Compressor();
		rightCompressor = new Compressor();
	}

	EffectCompressor::~EffectCompressor()
	{
		DEL(leftCompressor);
		DEL(rightCompressor);
	}

	void EffectCompressor::Clear()
	{
		leftCompressor->Clear();
		rightCompressor->Clear();
	}

	//设置采样频率
	void EffectCompressor::SetSampleFreq(float freq)
	{
		leftCompressor->SetSampleFreq(freq);
		rightCompressor->SetSampleFreq(freq);
	}

	//设置Attack时长
	void EffectCompressor::SetAttackSec(float sec)
	{
		leftCompressor->SetAttackSec(sec);
		rightCompressor->SetAttackSec(sec);
	}

	//设置Release时长
	void EffectCompressor::SetReleaseSec(float sec)
	{
		leftCompressor->SetReleaseSec(sec);
		rightCompressor->SetReleaseSec(sec);
	}

	//设置比值
	void EffectCompressor::SetRadio(float radio)
	{
		leftCompressor->SetRadio(radio);
		rightCompressor->SetRadio(radio);
	}

	//设置门限
	void EffectCompressor::SetThreshold(float threshold)
	{
		leftCompressor->SetThreshold(threshold);
		rightCompressor->SetThreshold(threshold);

	}

	//设置拐点的软硬
	void EffectCompressor::SetKneeWidth(float width)
	{
		leftCompressor->SetKneeWidth(width);
		rightCompressor->SetKneeWidth(width);
	}

	//设置增益补偿
	void EffectCompressor::SetMakeupGain(float gain)
	{
		leftCompressor->SetMakeupGain(gain);
		rightCompressor->SetMakeupGain(gain);
	}

	//设置是否自动增益补偿
	void EffectCompressor::SetAutoMakeupGain(bool isAuto)
	{
		leftCompressor->SetAutoMakeupGain(isAuto);
		rightCompressor->SetAutoMakeupGain(isAuto);
	}

	//计算系数
	void EffectCompressor::CalculateCoefficients()
	{
		leftCompressor->CalculateCoefficients();
		rightCompressor->CalculateCoefficients();
	}


	void EffectCompressor::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = leftCompressor->Tick(leftChannelSamples[i]);
			rightChannelSamples[i] = rightCompressor->Tick(rightChannelSamples[i]);
		}
	}
}
