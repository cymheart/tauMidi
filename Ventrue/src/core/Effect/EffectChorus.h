#ifndef _EffectChorus_h_
#define _EffectChorus_h_

#include <stk\Chorus.h>
#include"VentrueEffect.h"
using namespace stk;

namespace ventrue
{
	class DLL_CLASS EffectChorus : public VentrueEffect
	{
	public:
		EffectChorus();
		~EffectChorus();

		void Clear();
		float GetEffectMix();

		//! Set modulation depth in range 0.0 - 1.0.
		void SetModDepth(float depth);

		//! Set modulation frequency.
		void SetModFrequency(float frequency);

		void SetEffectMix(float mix);

		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		void Tick(float inputL, float inputR);
		const StkFrames& LastFrame();

	private:
		Chorus* leftChannelChorus;
		Chorus* rightChannelChorus;
		StkFrames* outFrames;
	};
}

#endif
