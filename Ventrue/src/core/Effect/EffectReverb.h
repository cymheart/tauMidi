#ifndef _VentrueReverb_h_
#define _VentrueReverb_h_

#include <stk\FreeVerb.h>
#include"VentrueEffect.h"
using namespace stk;

namespace ventrue
{
	class DLL_CLASS EffectReverb : public VentrueEffect
	{
	public:
		EffectReverb();
		~EffectReverb();

		void Tick(float inputL, float inputR);
		const StkFrames& LastFrame();
		void Clear();
		float GetEffectMix();

		//! Set the room size (comb filter feedback gain) parameter [0,1].
		void SetRoomSize(float value);

		//! Set the width (left-right mixing) parameter [0,1].
		void SetWidth(float value);

		//! Set the damping parameter [0=low damping, 1=higher damping].
		void SetDamping(float value);

		//! Set the effect mix [0 = mostly dry, 1 = mostly wet].
		void SetEffectMix(float mix);

		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		FreeVerb* freeVerb;
	};

}

#endif
