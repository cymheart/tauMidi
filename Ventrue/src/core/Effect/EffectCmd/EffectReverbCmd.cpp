#include"EffectReverbCmd.h"

namespace ventrue
{
	void EffectReverbCmd::SetRoomSize(float value)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetRoomSize;
		task->valuef[0] = value;
		ventrue->PostTask(task);
	}

	void EffectReverbCmd::_SetRoomSize(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectReverb& reverb = *(EffectReverb*)(effectTask->effect);
		reverb.SetRoomSize(effectTask->valuef[0]);
	}

	void EffectReverbCmd::SetWidth(float value)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetWidth;
		task->valuef[0] = value;
		ventrue->PostTask(task);
	}

	void EffectReverbCmd::_SetWidth(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectReverb& reverb = *(EffectReverb*)(effectTask->effect);
		reverb.SetWidth(effectTask->valuef[0]);
	}

	void EffectReverbCmd::SetDamping(float value)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetDamping;
		task->valuef[0] = value;
		ventrue->PostTask(task);
	}

	void EffectReverbCmd::_SetDamping(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectReverb& reverb = *(EffectReverb*)(effectTask->effect);
		reverb.SetDamping(effectTask->valuef[0]);
	}


	void EffectReverbCmd::SetEffectMix(float value)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetEffectMix;
		task->valuef[0] = value;
		ventrue->PostTask(task);
	}

	void EffectReverbCmd::_SetEffectMix(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectReverb& reverb = *(EffectReverb*)(effectTask->effect);
		reverb.SetEffectMix(effectTask->valuef[0]);
	}

}
