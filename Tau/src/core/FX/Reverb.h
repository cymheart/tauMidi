#ifndef _SyntherReverb_h_
#define _SyntherReverb_h_

#include <stk\FreeVerb.h>
#include"TauEffect.h"

namespace tauFX
{
	class DLL_CLASS Reverb : public TauEffect
	{
	public:
		Reverb();
		virtual ~Reverb();
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

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);
		void Tick(float inputL, float inputR);
		const stk::StkFrames& LastFrame();

	private:
		stk::FreeVerb* freeVerb;
	};

}

#endif
