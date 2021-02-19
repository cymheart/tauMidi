#ifndef _EffectDelay_h_
#define _EffectDelay_h_

#include <stk\DelayL.h>
#include"VentrueEffect.h"
using namespace stk;

namespace ventrue
{
	enum DelayChannel
	{
		LeftChannel = 0,
		RightChannel,
		AllChannel
	};

	class DLL_CLASS EffectDelay : public VentrueEffect
	{
	public:
		EffectDelay();
		~EffectDelay();

		void SetDelayChannel(DelayChannel delayChannel)
		{
			this->delayChannel = delayChannel;
		}

		void SetDelay(float delay);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		DelayL* delayLeftChannel;
		DelayL* delayRightChannel;
		DelayChannel delayChannel = DelayChannel::RightChannel;
	};
}
#endif
