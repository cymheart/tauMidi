#include"Equalizer.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	Equalizer::Equalizer()
	{
		eq = new dsignal::GraphEqualizer();
	}

	Equalizer::~Equalizer()
	{
		DEL(eq);
	}


	vector<dsignal::Filter*> Equalizer::GetFilters()
	{
		return eq->GetFilters();
	}

	void Equalizer::SetFreqBandGain(int bandIdx, float gainDB)
	{
		eq->SetFreqBandGain(bandIdx, gainDB);
	}

	void Equalizer::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = (float)eq->Filtering(leftChannelSamples[i]);
			rightChannelSamples[i] = (float)eq->Filtering(rightChannelSamples[i]);
		}
	}

	void Equalizer::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		if (numChannels == 2) {
			for (int i = 0; i < channelSampleCount * numChannels; i += 2)
			{
				synthStream[i] = (float)eq->Filtering(synthStream[i]);
				synthStream[i + 1] = (float)eq->Filtering(synthStream[i + 1]);
			}
		}
		else {
			for (int i = 0; i < channelSampleCount; i++)
			{
				synthStream[i] = (float)eq->Filtering(synthStream[i]);
			}
		}
	}
}
