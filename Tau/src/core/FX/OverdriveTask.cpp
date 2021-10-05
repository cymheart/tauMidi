#include"Overdrive.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	void Overdrive::SetDriveTask(float drive)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDriveTask;
		task->valuef[0] = drive;
		synther->PostTask(task);
	}

	void Overdrive::_SetDriveTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Overdrive& overdrive = *(Overdrive*)(effectTask->effect);
		overdrive.SetDrive(effectTask->valuef[0]);
	}

}
