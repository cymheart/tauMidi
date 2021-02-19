#include"EffectDelay.h"

namespace ventrue
{
	EffectDelay::EffectDelay()
	{
		delayLeftChannel = new DelayL();
		delayRightChannel = new DelayL();

		SetDelay(221);
	}

	EffectDelay::~EffectDelay()
	{
		DEL(delayLeftChannel);
		DEL(delayRightChannel);
	}

	void EffectDelay::SetDelay(float delay)
	{
		delayLeftChannel->setDelay(delay);
		delayRightChannel->setDelay(delay);
	}


	void EffectDelay::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			if (delayChannel == DelayChannel::LeftChannel) {
				leftChannelSamples[i] = delayLeftChannel->tick(leftChannelSamples[i]);
			}
			else if (delayChannel == DelayChannel::RightChannel) {
				rightChannelSamples[i] = delayRightChannel->tick(rightChannelSamples[i]);
			}
			else {
				leftChannelSamples[i] = delayLeftChannel->tick(leftChannelSamples[i]);
				rightChannelSamples[i] = delayRightChannel->tick(rightChannelSamples[i]);
			}
		}
	}
}
