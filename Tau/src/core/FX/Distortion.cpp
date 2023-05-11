#include"Distortion.h"

namespace tauFX
{
	Distortion::Distortion()
	{
		leftDistortion = new dsignal::Distortion();
		rightDistortion = new dsignal::Distortion();

		leftDistortion->Init();
		rightDistortion->Init();
	}

	Distortion::~Distortion()
	{
		DEL(leftDistortion);
		DEL(rightDistortion);
	}

	void Distortion::SetType(DistortionType type)
	{
		leftDistortion->SetType((dsignal::DistortionType)type);
		rightDistortion->SetType((dsignal::DistortionType)type);
	}

	/** Set the amount of drive
		\param drive Works from 0-1
	*/
	void Distortion::SetDrive(float drive)
	{
		leftDistortion->SetDrive(drive);
		rightDistortion->SetDrive(drive);
	}

	//设置干湿混合度[0,1]
	void Distortion::SetMix(float mix)
	{
		leftDistortion->SetMix(mix);
		rightDistortion->SetMix(mix);
	}

	//设置阈值[0,1], 作用于Clip,Overdrive类型
	void Distortion::SetThreshold(float threshold)
	{
		leftDistortion->SetThreshold(threshold);
		rightDistortion->SetThreshold(threshold);
	}

	//设置阈值, 作用于Clip,Overdrive, Fuzz类型
	//正阈值：[0,1]
	//负阈值: [-1,0]
	void Distortion::SetThreshold(float positiveThreshold, float negativeThreshold)
	{
		leftDistortion->SetThreshold(positiveThreshold, negativeThreshold);
		rightDistortion->SetThreshold(positiveThreshold, negativeThreshold);
	}

	//设置Soft宽度[0,&], 作用于Overdrive类型
	//默认值0.2
	void Distortion::SetSoftWidth(float w)
	{
		leftDistortion->SetSoftWidth(w);
		rightDistortion->SetSoftWidth(w);
	}

	//设置整流值[0,1]
		//rectif 参数值在 0.5 以下时将执行半波整流，在 0.5 和 1 之间时将执行全波整流。
		//半波整流用于让负振幅部分保持在特定电平之上；如图例，可以通过 rectif 参数逐步将起始点 -1 调整为 0，
		//值为 0 时不会应用半波整流，值为 0.5 时则不允许负振幅低于 0，即完全削波。
		//全波整流会更进一步，随着 rectif 参数值上升，输入的负信号部分将逐步变为正信号。
		//值为 1 时将实现全波整流，负信号部分的极性将完全反转。
		//它能起到让基频翻倍的作用，可用于实现“高八度”效果,即全波整流后，所得到的波形分解的基波频率将会提升1倍，频率提升1倍，即音调提升一个8度
	void  Distortion::SetRectif(float rectif)
	{
		leftDistortion->SetRectif(rectif);
		rightDistortion->SetRectif(rectif);
	}

	//设置滤波器过渡位置[0,1]
	void  Distortion::SetFilterAlpha(float a)
	{
		leftDistortion->SetFilterAlpha(a);
		rightDistortion->SetFilterAlpha(a);
	}

	//设置频率点
	void  Distortion::SetFilterFreq(float freqHZ)
	{
		leftDistortion->SetFilterFreq(freqHZ);
		rightDistortion->SetFilterFreq(freqHZ);
	}

	//设置Q
	void  Distortion::SetFilterQ(float q)
	{
		leftDistortion->SetFilterQ(q);
		rightDistortion->SetFilterQ(q);
	}

	//开启前置滤波器
	void  Distortion::EnablePreFilter()
	{
		leftDistortion->EnablePreFilter();
		rightDistortion->EnablePreFilter();
	}


	//开启后置滤波器
	void  Distortion::EnablePostFilter()
	{
		leftDistortion->EnablePostFilter();
		rightDistortion->EnablePostFilter();
	}


	//设置不开启滤波器
	void  Distortion::NotEnableFilter()
	{
		leftDistortion->NotEnableFilter();
		rightDistortion->NotEnableFilter();
	}


	void Distortion::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = leftDistortion->Process(leftChannelSamples[i]);
			rightChannelSamples[i] = rightDistortion->Process(rightChannelSamples[i]);
		}
	}

	void Distortion::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		if (numChannels == 2) {
			for (int i = 0; i < channelSampleCount * numChannels; i += 2)
			{
				synthStream[i] = leftDistortion->Process(synthStream[i]);
				synthStream[i + 1] = rightDistortion->Process(synthStream[i + 1]);
			}
		}
		else {
			for (int i = 0; i < channelSampleCount; i++)
			{
				synthStream[i] = leftDistortion->Process(synthStream[i]);
			}
		}
	}
}
