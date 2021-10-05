#include"Reverb.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Reverb::SetRoomSizeTask(float value)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetRoomSizeTask;
		task->valuef[0] = value;
		synther->PostTask(task);
	}

	void Reverb::_SetRoomSizeTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Reverb& reverb = *(Reverb*)(effectTask->effect);
		reverb.SetRoomSize(effectTask->valuef[0]);
	}

	void Reverb::SetWidthTask(float value)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetWidthTask;
		task->valuef[0] = value;
		synther->PostTask(task);
	}

	void Reverb::_SetWidthTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Reverb& reverb = *(Reverb*)(effectTask->effect);
		reverb.SetWidth(effectTask->valuef[0]);
	}

	void Reverb::SetDampingTask(float value)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDampingTask;
		task->valuef[0] = value;
		synther->PostTask(task);
	}

	void Reverb::_SetDampingTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Reverb& reverb = *(Reverb*)(effectTask->effect);
		reverb.SetDamping(effectTask->valuef[0]);
	}


	void Reverb::SetEffectMixTask(float value)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetEffectMixTask;
		task->valuef[0] = value;
		synther->PostTask(task);
	}

	void Reverb::_SetEffectMixTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Reverb& reverb = *(Reverb*)(effectTask->effect);
		reverb.SetEffectMix(effectTask->valuef[0]);
	}

}
