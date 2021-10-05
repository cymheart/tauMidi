#include"Autowah.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Autowah::SetWahTask(float wah)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetWahTask;
		task->valuef[0] = wah;
		synther->PostTask(task);
	}

	void Autowah::_SetWahTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Autowah& autowah = *(Autowah*)(effectTask->effect);
		autowah.SetWah(effectTask->valuef[0]);
	}


	void Autowah::SetDryWetTask(float drywet)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDryWetTask;
		task->valuef[0] = drywet;
		synther->PostTask(task);
	}

	void Autowah::_SetDryWetTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Autowah& autowah = *(Autowah*)(effectTask->effect);
		autowah.SetDryWet(effectTask->valuef[0]);
	}


	void Autowah::SetLevelTask(float level)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetLevelTask;
		task->valuef[0] = level;
		synther->PostTask(task);
	}

	void Autowah::_SetLevelTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Autowah& autowah = *(Autowah*)(effectTask->effect);
		autowah.SetLevel(effectTask->valuef[0]);
	}
}
