#include"TauEffect.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"
namespace tauFX
{

	void TauEffect::SetEnableTask(bool isEnable)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetEnableTask;
		task->valuebl[0] = isEnable;
		synther->PostTask(task);
	}

	void TauEffect::_SetEnableTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		TauEffect& effect = *(effectTask->effect);
		effect.SetEnable(effectTask->valuebl[0]);
	}
}
