#include"Delay.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Delay::SetDelayChannelTask(DelayChannel delayChannel)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDelayChannelTask;
		task->valuei[0] = delayChannel;
		synther->PostTask(task);
	}

	void Delay::_SetDelayChannelTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Delay& delay = *(Delay*)(effectTask->effect);
		delay.SetDelayChannel((DelayChannel)effectTask->valuei[0]);
	}

	void Delay::SetDelayTask(float delay)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDelayTask;
		task->valuef[0] = delay;
		synther->PostTask(task);
	}

	void Delay::_SetDelayTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Delay& delay = *(Delay*)(effectTask->effect);
		delay.SetDelay((float)effectTask->valuef[0]);
	}

}
