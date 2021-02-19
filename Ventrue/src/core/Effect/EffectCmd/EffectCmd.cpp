#include"EffectCmd.h"

namespace ventrue
{
	void EffectCmd::SetEnable(bool isEnable)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetEnable;
		task->valuebl[0] = isEnable;
		ventrue->PostTask(task);
	}

	void EffectCmd::_SetEnable(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		VentrueEffect& effect = *(effectTask->effect);
		effect.SetEnable(effectTask->valuebl[0]);
	}
}
