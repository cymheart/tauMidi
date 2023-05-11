#ifndef _EffectChorus_h_
#define _EffectChorus_h_

#include"TauEffect.h"
#include <DaisySP/daisysp.h>
#include <stk/Chorus.h>

namespace tauFX
{
	class DLL_CLASS Chorus : public TauEffect
	{
	public:
		Chorus();
		virtual ~Chorus();

		void Clear();
		void Init(float sample_rate);
		float GetEffectMix();

		//! Set modulation depth in range 0.0 - 1.0.
		void SetModDepth(float depth);

		//! Set modulation frequency.
		void SetModFrequency(float frequency);

		void SetEffectMix(float mix);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);

	private:

		daisysp::Chorus* leftChannelChorus;
		daisysp::Chorus* rightChannelChorus;

		//stk::Chorus* leftChannelChorus;
		//stk::Chorus* rightChannelChorus;
	};
}

#endif
