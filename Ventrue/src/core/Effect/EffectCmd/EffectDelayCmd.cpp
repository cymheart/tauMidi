#include"EffectDelayCmd.h"

namespace ventrue
{
	void EffectDelayCmd::SetDelayChannel(DelayChannel delayChannel)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetDelayChannel;
		task->valuei[0] = delayChannel;
		ventrue->PostTask(task);
	}

	void EffectDelayCmd::_SetDelayChannel(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectDelay& delay = *(EffectDelay*)(effectTask->effect);
		delay.SetDelayChannel((DelayChannel)effectTask->valuei[0]);
	}

	void EffectDelayCmd::SetDelay(float delay)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetDelay;
		task->valuei[0] = delay;
		ventrue->PostTask(task);
	}

	void EffectDelayCmd::_SetDelay(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectDelay& delay = *(EffectDelay*)(effectTask->effect);
		delay.SetDelay((DelayChannel)effectTask->valuei[0]);
	}

}
