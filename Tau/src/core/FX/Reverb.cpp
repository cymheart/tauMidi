#include"Reverb.h"

namespace tauFX
{

	Reverb::Reverb()
	{
		freeVerb = new stk::FreeVerb();
	}

	Reverb::~Reverb()
	{
		DEL(freeVerb);
	}

	//! Set the room size (comb filter feedback gain) parameter [0,1].
	void Reverb::SetRoomSize(float value)
	{
		freeVerb->setRoomSize(value);
	}

	//! Set the width (left-right mixing) parameter [0,1].
	void Reverb::SetWidth(float value)
	{
		freeVerb->setWidth(value);
	}

	//! Set the damping parameter [0=low damping, 1=higher damping].
	void Reverb::SetDamping(float value)
	{
		freeVerb->setDamping(value);
	}

	//! Set the effect mix [0 = mostly dry, 1 = mostly wet].
	void Reverb::SetEffectMix(float mix)
	{
		freeVerb->setEffectMix(mix);
	}

	float Reverb::GetEffectMix()
	{
		return (float)freeVerb->getEffectMix();
	}

	void Reverb::Tick(float inputL, float inputR)
	{
		freeVerb->tick(inputL, inputR);
	}

	const stk::StkFrames& Reverb::LastFrame()
	{
		return freeVerb->lastFrame();
	}

	void Reverb::Clear()
	{
		freeVerb->clear();
	}

	void Reverb::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
	{
		const stk::StkFrames* frames;

		for (int i = 0; i < channelSampleCount; i++)
		{
			Tick(leftChannelSamples[i], rightChannelSamples[i]);
			frames = &LastFrame();
			leftChannelSamples[i] = (float)(*frames)[0];
			rightChannelSamples[i] = (float)(*frames)[1];
		}
	}

	void Reverb::EffectProcess(float* synthStream, int numChannels, int channelSampleCount)
	{
		const stk::StkFrames* frames;
		if (numChannels == 2) {
			for (int i = 0; i < channelSampleCount * numChannels; i += 2)
			{
				Tick(synthStream[i], synthStream[i + 1]);
				frames = &LastFrame();
				synthStream[i] = (float)(*frames)[0];
				synthStream[i + 1] = (float)(*frames)[1];
			}
		}
		else {
			for (int i = 0; i < channelSampleCount; i++)
			{
				Tick(synthStream[i], synthStream[i]);
				frames = &LastFrame();
				synthStream[i] = (float)(*frames)[0];
			}
		}
	}
}

