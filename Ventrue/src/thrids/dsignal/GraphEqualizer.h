#ifndef _GraphEqualizer_h_
#define _GraphEqualizer_h_

#include "Biquad.h"

namespace dsignal
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
	* 图示均衡器
	*/
	class GraphEqualizer
	{
	public:
		GraphEqualizer();
		~GraphEqualizer();

		void Clear();

		void SetSampleRate(float sampleRate)
		{
			this->sampleRate = sampleRate;
		}

		void SetFreqBandEQInfos(FreqBandEQInfo* freqBandEQInfos, int size);
		void Create();
		void SetFreqBandGain(int bandIdx, float gainDB);

		vector<dsignal::Filter*> GetFilters();

	private:
		double Filtering(double input);


	private:
		float sampleRate = 44100;
		FreqBandEQInfo* freqBandEQInfo = nullptr;
		Biquad* freqBandBiquad;
		int size = 0;
	};
}

#endif
