#include"Delay.h"

namespace tauFX
{
	Delay::Delay()
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

	void Delay::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		if (numChannels == 2) {
			for (int i = 0; i < channelSampleCount * numChannels; i += 2)
			{
				if (delayChannel == DelayChannel::LeftChannel) {
					synthStream[i] = (float)delayLeftChannel->tick(synthStream[i]);
				}
				else if (delayChannel == DelayChannel::RightChannel) {
					synthStream[i + 1] = (float)delayRightChannel->tick(synthStream[i + 1]);
				}
				else {
					synthStream[i] = (float)delayLeftChannel->tick(synthStream[i]);
					synthStream[i + 1] = (float)delayRightChannel->tick(synthStream[i + 1]);
				}
			}
		}
		else {
			for (int i = 0; i < channelSampleCount; i++)
			{
				synthStream[i] = (float)delayLeftChannel->tick(synthStream[i]);
			}
		}
	}
}
