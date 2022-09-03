#ifndef _SyntherReverb_h_
#define _SyntherReverb_h_

#include <stk\FreeVerb.h>
#include"TauEffect.h"

namespace tauFX
{
	class DLL_CLASS Reverb : public TauEffect
	{
	public:
		Reverb(Synther* synther = nullptr);
		virtual ~Reverb();

		void Tick(float inputL, float inputR);
		const stk::StkFrames& LastFrame();
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

	public:
		void SetRoomSizeTask(float value);
		void SetWidthTask(float value);
		void SetDampingTask(float value);
		void SetEffectMixTask(float value);

	private:
		static void _SetRoomSizeTask(Task* task);
		static void _SetWidthTask(Task* task);
		static void _SetDampingTask(Task* task);
		static void _SetEffectMixTask(Task* task);


	private:
		stk::FreeVerb* freeVerb;
	};

}

#endif
