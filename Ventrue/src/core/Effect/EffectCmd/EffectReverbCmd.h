#ifndef _EffectReverbCmd_h_
#define _EffectReverbCmd_h_

#include"Effect/EffectReverb.h"
#include"EffectCmd.h"

namespace ventrue
{
	class DLL_CLASS EffectReverbCmd :public EffectCmd
	{
	public:
		EffectReverbCmd(Ventrue* ventrue, VentrueEffect* effect)
			:EffectCmd(ventrue, effect)
		{

		}

		//! Set the room size (comb filter feedback gain) parameter [0,1].
		void SetRoomSize(float value);

		//! Set the width (left-right mixing) parameter [0,1].
		void SetWidth(float value);

		//! Set the damping parameter [0=low damping, 1=higher damping].
		void SetDamping(float value);

		//! Set the effect mix [0 = mostly dry, 1 = mostly wet].
		void SetEffectMix(float mix);

	private:
		static void _SetRoomSize(Task* task);
		static void _SetWidth(Task* task);
		static void _SetDamping(Task* task);
		static void _SetEffectMix(Task* task);
	};
}

#endif
