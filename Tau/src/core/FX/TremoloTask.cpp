#include"Tremolo.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Tremolo::SetFreqTask(float freq)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetFreqTask;
		task->valuef[0] = freq;
		synther->PostTask(task);
	}

	void Tremolo::_SetFreqTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Tremolo& tremolo = *(Tremolo*)(effectTask->effect);
		tremolo.SetFreq(effectTask->valuef[0]);
	}


	void Tremolo::SetWaveformTask(int waveform)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetWaveformTask;
		task->valuei[0] = waveform;
		synther->PostTask(task);
	}

	void Tremolo::_SetWaveformTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Tremolo& tremolo = *(Tremolo*)(effectTask->effect);
		tremolo.SetWaveform(effectTask->valuei[0]);
	}


	void Tremolo::SetDepthTask(float depth)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDepthTask;
		task->valuef[0] = depth;
		synther->PostTask(task);
	}

	void Tremolo::_SetDepthTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Tremolo& tremolo = *(Tremolo*)(effectTask->effect);
		tremolo.SetDepth(effectTask->valuef[0]);
	}
}
