#ifndef _EffectDelayCmd_h_
#define _EffectDelayCmd_h_

#include"Effect/EffectDelay.h"
#include"EffectCmd.h"


namespace ventrue
{
	class DLL_CLASS EffectDelayCmd :public EffectCmd
	{
	public:
		EffectDelayCmd(Ventrue* ventrue, VentrueEffect* effect)
			:EffectCmd(ventrue, effect)
		{

		}

		void SetDelayChannel(DelayChannel delayChannel);
		void SetDelay(float delay);

	private:
		static void _SetDelayChannel(Task* task);
		static void _SetDelay(Task* task);
	};
}

#endif
