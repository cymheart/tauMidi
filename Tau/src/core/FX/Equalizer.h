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
		Equalizer(Synther* synther = nullptr);
		virtual ~Equalizer();

		void SetSynther(Synther* synther);
		void SetFreqBandGain(int bandIdx, float gainDB);
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		vector<dsignal::Filter*> GetFilters();

	public:
		void SetFreqBandGainTask(int bandIdx, float gainDB);

	private:
		static void _SetFreqBandGainTask(Task* task);

	private:
		dsignal::GraphEqualizer* eq;
	};
}

#endif
