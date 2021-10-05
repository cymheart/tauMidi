#include"Distortion.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{

	void Distortion::SetTypeTask(DistortionType type)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetTypeTask;
		task->valuei[0] = (int)type;
		synther->PostTask(task);
	}

	void Distortion::_SetTypeTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetType((DistortionType)effectTask->valuei[0]);
	}

	void Distortion::SetDriveTask(float drive)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetDriveTask;
		task->valuef[0] = drive;
		synther->PostTask(task);
	}

	void Distortion::_SetDriveTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetDrive(effectTask->valuef[0]);
	}

	//设置干湿混合度[0,1]
	void Distortion::SetMixTask(float mix)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetMixTask;
		task->valuef[0] = mix;
		synther->PostTask(task);
	}


	void Distortion::_SetMixTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetMix(effectTask->valuef[0]);


	}
	//设置阈值[0,1], 作用于Clip,Overdrive类型
	void Distortion::SetThresholdTask(float threshold)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetThresholdTask;
		task->valuef[0] = threshold;
		synther->PostTask(task);
	}

	void Distortion::_SetThresholdTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetThreshold(effectTask->valuef[0]);
	}

	//设置阈值, 作用于Clip,Overdrive, Fuzz类型
	//正阈值：[0,1]
	//负阈值: [-1,0]
	void Distortion::SetThresholdTask(float positiveThreshold, float negativeThreshold)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetThresholdTask2;
		task->valuef[0] = positiveThreshold;
		task->valuef[1] = negativeThreshold;
		synther->PostTask(task);
	}

	void Distortion::_SetThresholdTask2(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetThreshold(effectTask->valuef[0], effectTask->valuef[1]);
	}

	//设置Soft宽度[0,&], 作用于Overdrive类型
	//默认值0.2
	void Distortion::SetSoftWidthTask(float threshold)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetSoftWidthTask;
		task->valuef[0] = threshold;
		synther->PostTask(task);
	}

	void Distortion::_SetSoftWidthTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetSoftWidth(effectTask->valuef[0]);
	}


	void Distortion::SetRectifTask(float rectif)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetRectifTask;
		task->valuef[0] = rectif;
		synther->PostTask(task);
	}

	void Distortion::_SetRectifTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetRectif(effectTask->valuef[0]);
	}

	//设置滤波器过渡位置[0,1]
	void Distortion::SetFilterAlphaTask(float a)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetFilterAlphaTask;
		task->valuef[0] = a;
		synther->PostTask(task);
	}

	void Distortion::_SetFilterAlphaTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetFilterAlpha(effectTask->valuef[0]);
	}

	//设置频率点
	void Distortion::SetFilterFreqTask(float freqHZ)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetFilterFreqTask;
		task->valuef[0] = freqHZ;
		synther->PostTask(task);
	}

	void Distortion::_SetFilterFreqTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetFilterFreq(effectTask->valuef[0]);
	}

	//设置Q
	void Distortion::SetFilterQTask(float q)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetFilterFreqTask;
		task->valuef[0] = q;
		synther->PostTask(task);
	}

	void Distortion::_SetFilterQTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.SetFilterQ(effectTask->valuef[0]);
	}

	//开启前置滤波器
	void Distortion::EnablePreFilterTask()
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetFilterFreqTask;
		synther->PostTask(task);
	}

	void Distortion::_EnablePreFilterTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.EnablePreFilter();
	}


	//开启后置滤波器
	void Distortion::EnablePostFilterTask()
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _EnablePostFilterTask;
		synther->PostTask(task);
	}

	void Distortion::_EnablePostFilterTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.EnablePostFilter();
	}


	//设置不开启滤波器
	void Distortion::NotEnableFilterTask()
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _NotEnableFilterTask;
		synther->PostTask(task);
	}

	void Distortion::_NotEnableFilterTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Distortion& distortion = *(Distortion*)(effectTask->effect);
		distortion.NotEnableFilter();
	}
}
