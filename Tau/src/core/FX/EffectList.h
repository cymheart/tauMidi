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
			this->leftChannelSamples = leftChannelSamples;
			this->rightChannelSamples = rightChannelSamples;
			this->channelSampleCount = channelSampleCount;
		}

		void Process();

		//添加效果器
		void AppendEffect(TauEffect* effect);

	private:
		vector<TauEffect*> effects;
		float* leftChannelSamples = nullptr;
		float* rightChannelSamples = nullptr;
		int channelSampleCount = 64;
	};

}

#endif
