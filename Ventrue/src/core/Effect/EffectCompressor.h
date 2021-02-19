#ifndef _EffectCompressor_h_
#define _EffectCompressor_h_

#include "dsignal/Compressor.h"
#include"VentrueEffect.h"
using namespace dsignal;

namespace ventrue
{
	class DLL_CLASS EffectCompressor : public VentrueEffect
	{
	public:
		EffectCompressor();
		~EffectCompressor();

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

	private:
		Compressor* leftCompressor;
		Compressor* rightCompressor;
	};

}

#endif
