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
		Delay(Synther* synther = nullptr);
		virtual ~Delay();

		void SetDelayChannel(DelayChannel delayChannel)
		{
			this->delayChannel = delayChannel;
		}

		void SetDelay(float delay);

	public:
		void SetDelayChannelTask(DelayChannel delayChannel);
		void SetDelayTask(float delay);

	private:
		static void _SetDelayChannelTask(Task* task);
		static void _SetDelayTask(Task* task);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		stk::DelayL* delayLeftChannel;
		stk::DelayL* delayRightChannel;
		DelayChannel delayChannel = DelayChannel::RightChannel;
	};
}
#endif
