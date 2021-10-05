#ifndef _EffectCompressor_h_
#define _EffectCompressor_h_

#include "dsignal/Compressor.h"
#include"TauEffect.h"


namespace tauFX
{
	class DLL_CLASS Compressor : public TauEffect
	{
	public:
		Compressor(Synther* synther = nullptr);
		virtual ~Compressor();

		void Clear();

		//设置采样频率
		void SetSampleFreq(float freq);

		//设置Attack时长
		void SetAttackSec(float sec);

		//设置Release时长
		void SetReleaseSec(float sec);

		//设置比值
		void SetRadio(float radio);

		//设置门限
		void SetThreshold(float threshold);

		//设置拐点的软硬
		void SetKneeWidth(float width);

		//设置增益补偿
		void SetMakeupGain(float gain);

		//设置是否自动增益补偿
		void SetAutoMakeupGain(bool isAuto);

		//计算系数
		void CalculateCoefficients();

		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	public:
		//设置采样频率
		void SetSampleFreqTask(float freq);

		//设置Attack时长
		void SetAttackSecTask(float sec);

		//设置Release时长
		void SetReleaseSecTask(float sec);

		//设置比值
		void SetRadioTask(float radio);

		//设置门限
		void SetThresholdTask(float threshold);

		//设置拐点的软硬
		void SetKneeWidthTask(float width);

		//设置增益补偿
		void SetMakeupGainTask(float gain);

		//设置增益补偿
		void SetAutoMakeupGainTask(bool isAuto);

		//计算系数
		void CalculateCoefficientsTask();

	private:
		static void _SetSampleFreqTask(Task* task);
		static void _SetAttackSecTask(Task* task);
		static void _SetReleaseSecTask(Task* task);
		static void _SetRadioTask(Task* task);
		static void _SetThresholdTask(Task* task);
		static void _SetKneeWidthTask(Task* task);
		static void _SetMakeupGainTask(Task* task);
		static void _SetAutoMakeupGainTask(Task* task);
		static void _CalculateCoefficientsTask(Task* task);


	private:
		dsignal::Compressor* leftCompressor;
		dsignal::Compressor* rightCompressor;
	};

}

#endif
