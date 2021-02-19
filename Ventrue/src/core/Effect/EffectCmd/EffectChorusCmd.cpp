#include"EffectChorusCmd.h"

namespace ventrue
{
	void EffectChorusCmd::SetModDepth(float depth)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetModDepth;
		task->valuef[0] = depth;
		ventrue->PostTask(task);
	}

	void EffectChorusCmd::_SetModDepth(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectChorus& chorus = *(EffectChorus*)(effectTask->effect);
		chorus.SetModDepth(effectTask->valuef[0]);
	}

	void EffectChorusCmd::SetModFrequency(float depth)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetModFrequency;
		task->valuef[0] = depth;
		ventrue->PostTask(task);
	}

	void EffectChorusCmd::_SetModFrequency(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectChorus& chorus = *(EffectChorus*)(effectTask->effect);
		chorus.SetModFrequency(effectTask->valuef[0]);
	}

	void EffectChorusCmd::SetEffectMix(float depth)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetEffectMix;
		task->valuef[0] = depth;
		ventrue->PostTask(task);
	}

	void EffectChorusCmd::_SetEffectMix(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectChorus& chorus = *(EffectChorus*)(effectTask->effect);
		chorus.SetEffectMix(effectTask->valuef[0]);
	}
}
