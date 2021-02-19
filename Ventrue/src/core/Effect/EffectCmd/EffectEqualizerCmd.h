#ifndef _EffectEqualizerCmd_h_
#define _EffectEqualizerCmd_h_

#include"Effect/Equalizer.h"
#include"EffectCmd.h"

namespace ventrue
{
	class DLL_CLASS EffectEqualizerCmd :public EffectCmd
	{
	public:
		EffectEqualizerCmd(Ventrue* ventrue, VentrueEffect* effect)
			:EffectCmd(ventrue, effect)
		{

		}

		void SetFreqBandGain(int bandIdx, float gainDB);

	private:
		static void _SetFreqBandGain(Task* task);
	};
}

#endif
