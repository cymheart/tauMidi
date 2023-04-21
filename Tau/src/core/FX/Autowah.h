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
		Autowah(Synther* synther = nullptr);
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

	public:
		/** sets wah
		\param wah : set wah amount, , 0...1.0
	*/
		void SetWahTask(float wah);

		/** sets mix amount
			\param drywet : set effect dry/wet, 0...100.0
		*/
		void SetDryWetTask(float drywet);

		/** sets wah level
			\param level : set wah level, 0...1.0
		*/
		void SetLevelTask(float level);

	private:
		static void _SetWahTask(Task* task);
		static void _SetDryWetTask(Task* task);
		static void _SetLevelTask(Task* task);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		daisysp::Autowah* leftAutowah;
		daisysp::Autowah* rightAutowah;
	};
}
#endif
