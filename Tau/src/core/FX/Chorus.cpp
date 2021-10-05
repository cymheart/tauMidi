#include"Chorus.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	Chorus::Chorus(Synther* synther)
		:TauEffect(synther)
	{
		//leftChannelChorus = new stk::Chorus(2000);
		//rightChannelChorus = new stk::Chorus(2000);

		leftChannelChorus = new daisysp::Chorus();
		rightChannelChorus = new daisysp::Chorus();

		float sampleRate = 44100;
		if (synther != nullptr)
			sampleRate = synther->GetSampleProcessRate();

		leftChannelChorus->Init(sampleRate);
		rightChannelChorus->Init(sampleRate);

	}

	Chorus::~Chorus()
	{
		DEL(leftChannelChorus);
		DEL(rightChannelChorus);
	}

	void Chorus::SetSynther(Synther* synther)
	{
		this->synther = synther;
		Init(synther->GetSampleProcessRate());
	}

	void Chorus::Init(float sample_rate)
	{
		leftChannelChorus->Init(sample_rate);
		rightChannelChorus->Init(sample_rate);
	}

	//! Set modulation depth in range 0.0 - 1.0.
	void Chorus::SetModDepth(float depth)
	{
		//leftChannelChorus->setModDepth(depth);
		//rightChannelChorus->setModDepth(depth);

		leftChannelChorus->SetLfoDepth(depth);
		rightChannelChorus->SetLfoDepth(depth);

	}

	void Chorus::SetModFrequency(float frequency)
	{
		//leftChannelChorus->setModFrequency(frequency);
		//rightChannelChorus->setModFrequency(frequency);

		leftChannelChorus->SetLfoFreq(frequency);
		rightChannelChorus->SetLfoFreq(frequency);
	}

	void Chorus::SetEffectMix(float mix)
	{
		leftChannelChorus->SetEffectMix(mix);
		rightChannelChorus->SetEffectMix(mix);
	}

	float Chorus::GetEffectMix()
	{
		return (float)leftChannelChorus->GetEffectMix();
	}

	void Chorus::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			/*leftChannelChorus->tick(leftChannelSamples[i]);
			rightChannelChorus->tick(rightChannelSamples[i]);

			leftChannelSamples[i] = leftChannelChorus->lastOut(0) + rightChannelChorus->lastOut(0);
			rightChannelSamples[i] = leftChannelChorus->lastOut(1) + rightChannelChorus->lastOut(1);*/


			leftChannelChorus->Process(leftChannelSamples[i]);
			rightChannelChorus->Process(rightChannelSamples[i]);

			leftChannelSamples[i] = leftChannelChorus->GetLeft() + rightChannelChorus->GetLeft();
			rightChannelSamples[i] = leftChannelChorus->GetRight() + rightChannelChorus->GetRight();
		}
	}


}
