#ifndef _EffectEqualizer_h_
#define _EffectEqualizer_h_

#include "dsignal/GraphEqualizer.h"
#include"VentrueEffect.h"
using namespace dsignal;

namespace ventrue
{
	/**
	* 均衡器
	*/
	class DLL_CLASS EffectEqualizer : public VentrueEffect
	{
	public:
		EffectEqualizer();
		~EffectEqualizer();

		void SetSampleRate(float sampleRate);
		void SetFreqBandGain(int bandIdx, float gainDB);
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		vector<dsignal::Filter*> GetFilters();

	private:
		GraphEqualizer* eq;
	};
}

#endif
