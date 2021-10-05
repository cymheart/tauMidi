#ifndef _EffectTremolo_h_
#define _EffectTremolo_h_

#include <DaisySP/daisysp.h>
#include"TauEffect.h"

namespace tauFX
{
	class DLL_CLASS Tremolo : public TauEffect
	{
	public:
		Tremolo(Synther* synther = nullptr);
		virtual ~Tremolo();

		/** Sets the tremolo rate.
	\param freq Tremolo freq in Hz.
 */
		void SetFreq(float freq);

		/** Shape of the modulating lfo
			\param waveform Oscillator waveform. Use Oscillator::WAVE_SIN for example.
		*/
		void SetWaveform(int waveform);

		/** How much to modulate your volume.
			\param depth Works 0-1.
		*/
		void SetDepth(float depth);

	public:
		/** Sets the tremolo rate.
	\param freq Tremolo freq in Hz.
 */
		void SetFreqTask(float freq);

		/** Shape of the modulating lfo
			\param waveform Oscillator waveform. Use Oscillator::WAVE_SIN for example.
		*/
		void SetWaveformTask(int waveform);

		/** How much to modulate your volume.
			\param depth Works 0-1.
		*/
		void SetDepthTask(float depth);

	private:
		static void _SetFreqTask(Task* task);
		static void _SetWaveformTask(Task* task);
		static void _SetDepthTask(Task* task);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		daisysp::Tremolo* leftTremolo;
		daisysp::Tremolo* rightTremolo;
	};
}
#endif
