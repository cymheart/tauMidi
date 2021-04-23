#include"EffectChorus.h"

namespace ventrue
{
	EffectChorus::EffectChorus()
	{
		leftChannelChorus = new Chorus(2000);
		rightChannelChorus = new Chorus(2000);
		outFrames = new StkFrames();

		outFrames->resize(1, 2, 0);
	}

	EffectChorus::~EffectChorus()
	{
		DEL(leftChannelChorus);
		DEL(rightChannelChorus);
		DEL(outFrames);
	}

	//! Set modulation depth in range 0.0 - 1.0.
	void EffectChorus::SetModDepth(float depth)
	{
		leftChannelChorus->setModDepth(depth);
		rightChannelChorus->setModDepth(depth);
	}

	void EffectChorus::SetModFrequency(float frequency)
	{
		leftChannelChorus->setModFrequency(frequency);
		rightChannelChorus->setModFrequency(frequency);
	}

	void EffectChorus::SetEffectMix(float mix)
	{
		leftChannelChorus->setEffectMix(mix);
		rightChannelChorus->setEffectMix(mix);
	}

	float EffectChorus::GetEffectMix()
	{
		return leftChannelChorus->getEffectMix();
	}

	void EffectChorus::Tick(float inputL, float inputR)
	{
		leftChannelChorus->tick(inputL);
		rightChannelChorus->tick(inputR);
	}

	const StkFrames& EffectChorus::LastFrame()
	{
		const StkFrames& leftFrames = leftChannelChorus->lastFrame();
		const StkFrames& rightFrames = rightChannelChorus->lastFrame();

		(*outFrames)[0] = leftFrames[0];
		(*outFrames)[1] = rightFrames[0];
		return *outFrames;
	}

	void EffectChorus::Clear()
	{
		leftChannelChorus->clear();
		rightChannelChorus->clear();
	}


	void EffectChorus::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		const StkFrames* frames;
		for (int i = 0; i < channelSampleCount; i++)
		{
			Tick(leftChannelSamples[i], rightChannelSamples[i]);
			frames = &LastFrame();
			leftChannelSamples[i] = (float)(*frames)[0];
			rightChannelSamples[i] = (float)(*frames)[1];
		}
	}


}
