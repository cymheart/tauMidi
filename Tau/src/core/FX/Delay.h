#ifndef _EffectDelay_h_
#define _EffectDelay_h_

#include <stk\DelayL.h>
#include"TauEffect.h"

namespace tauFX
{
	enum DelayChannel
	{
		LeftChannel = 0,
		RightChannel,
		AllChannel
	};

	class DLL_CLASS Delay : public TauEffect
	{
	public:
		Delay();
		virtual ~Delay();

		void SetDelayChannel(DelayChannel delayChannel)
		{
			this->delayChannel = delayChannel;
		}

		void SetDelay(float delay);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);

	private:
		stk::DelayL* delayLeftChannel;
		stk::DelayL* delayRightChannel;
		DelayChannel delayChannel = DelayChannel::RightChannel;
	};
}
#endif
