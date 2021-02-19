#include"EffectReverb.h"

namespace ventrue
{

	EffectReverb::EffectReverb()
	{
		freeVerb = new FreeVerb();
	}

	EffectReverb::~EffectReverb()
	{
		DEL(freeVerb);
	}

	//! Set the room size (comb filter feedback gain) parameter [0,1].
	void EffectReverb::SetRoomSize(float value)
	{
		freeVerb->setRoomSize(value);
	}

	//! Set the width (left-right mixing) parameter [0,1].
	void EffectReverb::SetWidth(float value)
	{
		freeVerb->setWidth(value);
	}

	//! Set the damping parameter [0=low damping, 1=higher damping].
	void EffectReverb::SetDamping(float value)
	{
		freeVerb->setDamping(value);
	}

	//! Set the effect mix [0 = mostly dry, 1 = mostly wet].
	void EffectReverb::SetEffectMix(float mix)
	{
		freeVerb->setEffectMix(mix);
	}

	float EffectReverb::GetEffectMix()
	{
		return freeVerb->getEffectMix();
	}

	void EffectReverb::Tick(float inputL, float inputR)
	{
		freeVerb->tick(inputL, inputR);
	}

	const StkFrames& EffectReverb::LastFrame()
	{
		return freeVerb->lastFrame();
	}

	void EffectReverb::Clear()
	{
		freeVerb->clear();
	}

	void EffectReverb::EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount)
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
