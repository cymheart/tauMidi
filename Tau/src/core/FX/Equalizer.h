#ifndef _EffectEqualizer_h_
#define _EffectEqualizer_h_

#include "dsignal/GraphEqualizer.h"
#include"TauEffect.h"

namespace tauFX
{
	/**
	* 均衡器
	*/
	class DLL_CLASS Equalizer : public TauEffect
	{
	public:
		Equalizer();
		virtual ~Equalizer();

		void SetFreqBandGain(int bandIdx, float gainDB);

		vector<dsignal::Filter*> GetFilters();

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);

	private:
		dsignal::GraphEqualizer* eq;
	};
}

#endif
