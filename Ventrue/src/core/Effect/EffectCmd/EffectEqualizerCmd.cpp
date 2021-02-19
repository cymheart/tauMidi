#include"EffectEqualizerCmd.h"

namespace ventrue
{
	void EffectEqualizerCmd::SetFreqBandGain(int bandIdx, float gainDB)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetFreqBandGain;
		task->valuei[0] = bandIdx;
		task->valuef[0] = gainDB;
		ventrue->PostTask(task);
	}

	void EffectEqualizerCmd::_SetFreqBandGain(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Equalizer& eq = *(Equalizer*)(effectTask->effect);
		eq.SetFreqBandGain(effectTask->valuei[0], effectTask->valuef[0]);
	}

}
