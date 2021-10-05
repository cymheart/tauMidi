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
		Chorus(Synther* synther = nullptr);
		virtual ~Chorus();

		void SetSynther(Synther* synther);
		void Init(float sample_rate);
		float GetEffectMix();

		//! Set modulation depth in range 0.0 - 1.0.
		void SetModDepth(float depth);

		//! Set modulation frequency.
		void SetModFrequency(float frequency);

		void SetEffectMix(float mix);

		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	public:
		void SetModDepthTask(float depth);
		void SetModFrequencyTask(float depth);
		void SetEffectMixTask(float depth);

	private:
		static void _SetModDepthTask(Task* task);
		static void _SetModFrequencyTask(Task* task);
		static void _SetEffectMixTask(Task* task);

	private:

		daisysp::Chorus* leftChannelChorus;
		daisysp::Chorus* rightChannelChorus;

		//stk::Chorus* leftChannelChorus;
		//stk::Chorus* rightChannelChorus;
	};
}

#endif
