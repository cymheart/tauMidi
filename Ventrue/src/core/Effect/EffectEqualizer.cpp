#include"EffectEqualizer.h"

namespace ventrue
{
	EffectEqualizer::EffectEqualizer()
	{
		eq = new GraphEqualizer();
	}

	EffectEqualizer::~EffectEqualizer()
	{
		DEL(eq);
	}


	vector<dsignal::Filter*> EffectEqualizer::GetFilters()
	{
		return eq->GetFilters();
	}


	void EffectEqualizer::SetFreqBandGain(int bandIdx, float gainDB)
	{
		eq->SetFreqBandGain(bandIdx, gainDB);
	}

	void EffectEqualizer::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = eq->Filtering(leftChannelSamples[i]);
			rightChannelSamples[i] = eq->Filtering(rightChannelSamples[i]);
		}
	}
}
