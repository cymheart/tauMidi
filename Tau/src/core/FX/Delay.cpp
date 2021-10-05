#include"Delay.h"

namespace tauFX
{
	Delay::Delay(Synther* synther)
		:TauEffect(synther)
	{
		delayLeftChannel = new stk::DelayL();
		delayRightChannel = new stk::DelayL();

		SetDelay(221);
	}

	Delay::~Delay()
	{
		DEL(delayLeftChannel);
		DEL(delayRightChannel);
	}

	void Delay::SetDelay(float delay)
	{
		delayLeftChannel->setDelay(delay);
		delayRightChannel->setDelay(delay);
	}


	void Delay::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			if (delayChannel == DelayChannel::LeftChannel) {
				leftChannelSamples[i] = (float)delayLeftChannel->tick(leftChannelSamples[i]);
			}
			else if (delayChannel == DelayChannel::RightChannel) {
				rightChannelSamples[i] = (float)delayRightChannel->tick(rightChannelSamples[i]);
			}
			else {
				leftChannelSamples[i] = (float)delayLeftChannel->tick(leftChannelSamples[i]);
				rightChannelSamples[i] = (float)delayRightChannel->tick(rightChannelSamples[i]);
			}
		}
	}
}
