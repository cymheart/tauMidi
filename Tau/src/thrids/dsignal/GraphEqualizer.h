#ifndef _GraphEqualizer_h_
#define _GraphEqualizer_h_

#include "iir/Iir1.h"
#include"Filter.h"

namespace dsignal
{
	//频带均衡信息
	struct FreqBandEQInfo
	{
		//中心频率(单位:HZ)
		float centerFreq = 0;

		//带宽(单位:HZ)
		float bandWidth = 0;

		//频带增益(单位:db)
		float gainDB = 0;
	};

	/**
	* 图形均衡器
	*/
	class GraphEqualizer
	{
	public:
		GraphEqualizer();

		void SetSampleRate(float sampleRate)
		{
			this->sampleRate = sampleRate;
		}

		void SetFreqBandGain(int bandIdx, float gainDB);

		vector<dsignal::Filter*> GetFilters();

		double Filtering(double input);

	private:
		float sampleRate = 44100;
		int bandCount = 10;
		FreqBandEQInfo freqBandEQInfo[31];
		Iir::RBJ::BandShelf freqBandBiquad[31];
		Filter bandFilter[31];
		int size = 0;
	};
}

#endif
