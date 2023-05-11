#ifndef _EffectList_h_
#define _EffectList_h_

#include"scutils/Utils.h"
#include"TauEffect.h"
#include"Equalizer.h"
#include"Reverb.h"
#include"Chorus.h"
#include"Delay.h"

namespace tauFX
{
	class EffectList
	{
	public:
		EffectList();
		~EffectList();

		void Set(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
		{
			synthStream = nullptr;

			//
			this->leftChannelSamples = leftChannelSamples;
			this->rightChannelSamples = rightChannelSamples;
			this->channelSampleCount = channelSampleCount;
		}

		void Set(float* synthStream, int numChannels, int channelSampleCount)
		{
			this->leftChannelSamples = nullptr;
			this->rightChannelSamples = nullptr;

			//
			this->synthStream = synthStream;
			this->numChannels = numChannels;
			this->channelSampleCount = channelSampleCount;
		}

		void Clear();

		void Process();

		//添加效果器
		void AppendEffect(TauEffect* effect);

	private:
		vector<TauEffect*> effects;
		float* synthStream = nullptr;
		float* leftChannelSamples = nullptr;
		float* rightChannelSamples = nullptr;
		int numChannels = 2;
		int channelSampleCount = 64;
	};

}

#endif
