#ifndef _EffectCmd_h_
#define _EffectCmd_h_

#include <Synth\Ventrue.h>
#include"EffectTask.h"

namespace ventrue
{
	class DLL_CLASS EffectCmd
	{
	public:
		EffectCmd(Ventrue* ventrue, VentrueEffect* effect)
		{
			this->ventrue = ventrue;
			this->effect = effect;
		}

		void SetEnable(bool isEnable);

	private:
		static void _SetEnable(Task* task);

	protected:
		Ventrue* ventrue = nullptr;
		VentrueEffect* effect = nullptr;
	};
}

#endif
