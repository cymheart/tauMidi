#ifndef _Equalizer_h_
#define _Equalizer_h_

#include "dsignal/Biquad.h"
#include"VentrueEffect.h"
using namespace dsignal;

namespace ventrue
{
	//频带均衡信息
	struct FreqBandEQInfo
	{
		//开始频率点(单位:HZ)
		float freqStart = 0;

		//结束频率点(单位:HZ)
		float freqEnd = 0;

		//频带增益(单位:db)
		float gainDB = 0;
	};

	/**
	* 均衡器
	*/
	class DLL_CLASS Equalizer : public VentrueEffect
	{
	public:
		Equalizer();
		~Equalizer();

		void Clear();

		void SetSampleRate(float sampleRate)
		{
			this->sampleRate = sampleRate;
		}

		void SetFreqBandEQInfos(FreqBandEQInfo* freqBandEQInfos, int size);
		void Create();
		void SetFreqBandGain(int bandIdx, float gainDB);

		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

		vector<dsignal::Filter*> GetFilters();

	private:
		double Filtering(double input);


	private:
		float sampleRate = 44100;
		FreqBandEQInfo* freqBandEQInfo = nullptr;
		Biquad freqBandBiquad[30];
		int size = 0;


	};
}

#endif
