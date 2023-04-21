#include"Autowah.h"

namespace tauFX
{
	Autowah::Autowah(Synther* synther)
		:TauEffect(synther)
	{
		leftAutowah = new daisysp::Autowah();
		rightAutowah = new daisysp::Autowah();

		leftAutowah->Init(44100);
		rightAutowah->Init(44100);
	}

	Autowah::~Autowah()
	{
		DEL(leftAutowah);
		DEL(rightAutowah);
	}

	void Autowah::Clear()
	{
		leftAutowah->Init(44100);
		rightAutowah->Init(44100);
	}

	/** sets wah
			\param wah : set wah amount, , 0...1.0
		*/
	void Autowah::SetWah(float wah)
	{
		leftAutowah->SetWah(wah);
		rightAutowah->SetWah(wah);
	}

	/** sets mix amount
		\param drywet : set effect dry/wet, 0...100.0
	*/
	void Autowah::SetDryWet(float drywet)
	{
		leftAutowah->SetDryWet(drywet);
		rightAutowah->SetDryWet(drywet);
	}

	/** sets wah level
		\param level : set wah level, 0...1.0
	*/
	void Autowah::SetLevel(float level)
	{
		leftAutowah->SetLevel(level);
		rightAutowah->SetLevel(level);
	}


	void Autowah::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		for (int i = 0; i < channelSampleCount; i++)
		{
			leftChannelSamples[i] = leftAutowah->Process(leftChannelSamples[i]);
			rightChannelSamples[i] = rightAutowah->Process(rightChannelSamples[i]);
		}
	}
}
