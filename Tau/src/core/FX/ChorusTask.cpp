#include"Chorus.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Chorus::SetModDepthTask(float depth)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetModDepthTask;
		task->valuef[0] = depth;
		synther->PostTask(task);
	}

	void Chorus::_SetModDepthTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Chorus& chorus = *(Chorus*)(effectTask->effect);
		chorus.SetModDepth(effectTask->valuef[0]);
	}

	void Chorus::SetModFrequencyTask(float depth)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetModFrequencyTask;
		task->valuef[0] = depth;
		synther->PostTask(task);
	}

	void Chorus::_SetModFrequencyTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Chorus& chorus = *(Chorus*)(effectTask->effect);
		chorus.SetModFrequency(effectTask->valuef[0]);
	}

	void Chorus::SetEffectMixTask(float depth)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetEffectMixTask;
		task->valuef[0] = depth;
		synther->PostTask(task);
	}

	void Chorus::_SetEffectMixTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Chorus& chorus = *(Chorus*)(effectTask->effect);
		chorus.SetEffectMix(effectTask->valuef[0]);
	}
}
