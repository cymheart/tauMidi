#include"Compressor.h"

namespace tauFX
{
	Compressor::Compressor()
	{
		leftCompressor = new dsignal::Compressor();
		rightCompressor = new  dsignal::Compressor();
	}

	Compressor::~Compressor()
	{
		DEL(leftCompressor);
		DEL(rightCompressor);
	}

	void Compressor::Clear()
	{
		leftCompressor->Clear();
		rightCompressor->Clear();
	}

	//设置采样频率
	void Compressor::SetSampleFreq(float freq)
	{
		leftCompressor->SetSampleFreq(freq);
		rightCompressor->SetSampleFreq(freq);
	}

	//设置Attack时长
	void Compressor::SetAttackSec(float sec)
	{
		leftCompressor->SetAttackSec(sec);
		rightCompressor->SetAttackSec(sec);
	}

	//设置Release时长
	void Compressor::SetReleaseSec(float sec)
	{
		leftCompressor->SetReleaseSec(sec);
		rightCompressor->SetReleaseSec(sec);
	}

	//设置比值
	void Compressor::SetRadio(float radio)
	{
		leftCompressor->SetRadio(radio);
		rightCompressor->SetRadio(radio);
	}

	//设置门限
	void Compressor::SetThreshold(float threshold)
	{
		leftCompressor->SetThreshold(threshold);
		rightCompressor->SetThreshold(threshold);

	}

	//设置拐点的软硬
	void Compressor::SetKneeWidth(float width)
	{
		leftCompressor->SetKneeWidth(width);
		rightCompressor->SetKneeWidth(width);
	}

	//设置增益补偿
	void Compressor::SetMakeupGain(float gain)
	{
		leftCompressor->SetMakeupGain(gain);
		rightCompressor->SetMakeupGain(gain);
	}

	//设置是否自动增益补偿
	void Compressor::SetAutoMakeupGain(bool isAuto)
	{
		leftCompressor->SetAutoMakeupGain(isAuto);
		rightCompressor->SetAutoMakeupGain(isAuto);
	}

	//计算系数
	void Compressor::CalculateCoefficients()
	{
		leftCompressor->CalculateCoefficients();
		rightCompressor->CalculateCoefficients();
	}


	void Compressor::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = leftCompressor->Process(leftChannelSamples[i]);
			rightChannelSamples[i] = rightCompressor->Process(rightChannelSamples[i]);
		}
	}

	void Compressor::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		if (numChannels == 2) {
			for (int i = 0; i < channelSampleCount * numChannels; i += 2)
			{
				synthStream[i] = leftCompressor->Process(synthStream[i]);
				synthStream[i + 1] = rightCompressor->Process(synthStream[i + 1]);
			}
		}
		else {
			for (int i = 0; i < channelSampleCount; i++)
			{
				synthStream[i] = leftCompressor->Process(synthStream[i]);
			}
		}
	}
}
