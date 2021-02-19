#ifndef _EffectList_h_
#define _EffectList_h_

#include"scutils/Utils.h"
#include "VentrueEffect.h"
#include"Equalizer.h"
#include"EffectReverb.h"
#include"EffectChorus.h"
#include"EffectDelay.h"

namespace ventrue
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
		void AppendEffect(VentrueEffect* effect);

	private:
		vector<VentrueEffect*> effects;
		float* leftChannelSamples = nullptr;
		float* rightChannelSamples = nullptr;
		int channelSampleCount = 64;
	};

}

#endif
