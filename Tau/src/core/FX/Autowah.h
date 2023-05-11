#ifndef _EffectAutowah_h_
#define _EffectAutowah_h_

#include <DaisySP/daisysp.h>
#include"TauEffect.h"
#include "task/Task.h"
using namespace task;

namespace tauFX
{
	class DLL_CLASS Autowah : public TauEffect
	{
	public:
		Autowah();
		virtual ~Autowah();

		void Clear();

		/** sets wah
			\param wah : set wah amount, , 0...1.0
		*/
		void SetWah(float wah);

		/** sets mix amount
			\param drywet : set effect dry/wet, 0...100.0
		*/
		void SetDryWet(float drywet);

		/** sets wah level
			\param level : set wah level, 0...1.0
		*/
		void SetLevel(float level);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);

	private:
		daisysp::Autowah* leftAutowah;
		daisysp::Autowah* rightAutowah;
	};
}
#endif
