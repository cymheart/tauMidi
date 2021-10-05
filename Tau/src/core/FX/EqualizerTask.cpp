#include"Equalizer.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Equalizer::SetFreqBandGainTask(int bandIdx, float gainDB)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetFreqBandGainTask;
		task->valuei[0] = bandIdx;
		task->valuef[0] = gainDB;
		synther->PostTask(task);
	}

	void Equalizer::_SetFreqBandGainTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Equalizer& eq = *(Equalizer*)(effectTask->effect);
		eq.SetFreqBandGain(effectTask->valuei[0], effectTask->valuef[0]);
	}

}
