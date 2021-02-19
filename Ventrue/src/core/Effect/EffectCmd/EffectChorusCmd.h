#ifndef _EffectChorusCmd_h_
#define _EffectChorusCmd_h_

#include"Effect/EffectChorus.h"
#include"EffectCmd.h"


namespace ventrue
{
	class DLL_CLASS EffectChorusCmd :public EffectCmd
	{
	public:
		EffectChorusCmd(Ventrue* ventrue, VentrueEffect* effect)
			:EffectCmd(ventrue, effect)
		{

		}

		//! Set modulation depth in range 0.0 - 1.0.
		void SetModDepth(float depth);

		//! Set modulation frequency.
		void SetModFrequency(float frequency);

		void SetEffectMix(float mix);

	private:
		static void _SetModDepth(Task* task);
		static void _SetModFrequency(Task* task);
		static void _SetEffectMix(Task* task);
	};
}

#endif
