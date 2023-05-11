#include"Tremolo.h"

namespace tauFX
{
	Tremolo::Tremolo()
	{
		leftTremolo = new daisysp::Tremolo();
		rightTremolo = new daisysp::Tremolo();

		leftTremolo->Init(44100);
		rightTremolo->Init(44100);
	}

	Tremolo::~Tremolo()
	{
		DEL(leftTremolo);
		DEL(rightTremolo);
	}

	/** Sets the tremolo rate.
	\param freq Tremolo freq in Hz.
	*/
	void Tremolo::SetFreq(float freq)
	{
		leftTremolo->SetFreq(freq);
		rightTremolo->SetFreq(freq);
	}

	/** Shape of the modulating lfo
		\param waveform Oscillator waveform. Use Oscillator::WAVE_SIN for example.
	*/
	void Tremolo::SetWaveform(int waveform)
	{
		leftTremolo->SetWaveform(waveform);
		rightTremolo->SetWaveform(waveform);
	}

	/** How much to modulate your volume.
		\param depth Works 0-1.
	*/
	void Tremolo::SetDepth(float depth)
	{
		leftTremolo->SetDepth(depth);
		rightTremolo->SetDepth(depth);
	}

	void Tremolo::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = leftTremolo->Process(leftChannelSamples[i]);
			rightChannelSamples[i] = leftTremolo->Process(rightChannelSamples[i]);
		}
	}

	void Tremolo::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		if (numChannels == 2) {
			for (int i = 0; i < channelSampleCount * numChannels; i += 2)
			{
				synthStream[i] = leftTremolo->Process(synthStream[i]);
				synthStream[i + 1] = leftTremolo->Process(synthStream[i + 1]);
			}
		}
		else {
			for (int i = 0; i < channelSampleCount; i++)
			{
				synthStream[i] = leftTremolo->Process(synthStream[i]);
			}
		}
	}
}
