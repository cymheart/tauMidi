#include"Compressor.h"
#include"EffectTask.h"
#include"Synth/Synther/Synther.h"

namespace tauFX
{
	//设置采样频率
	void Compressor::SetSampleFreqTask(float freq)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetSampleFreqTask;
		task->valuef[0] = freq;
		synther->PostTask(task);
	}

	//设置采样频率
	void Compressor::_SetSampleFreqTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetSampleFreq(effectTask->valuef[0]);
	}

	//设置Attack时长
	void Compressor::SetAttackSecTask(float sec)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetAttackSecTask;
		task->valuef[0] = sec;
		synther->PostTask(task);
	}

	//设置Attack时长
	void Compressor::_SetAttackSecTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetAttackSec(effectTask->valuef[0]);
	}


	//设置Release时长
	void Compressor::SetReleaseSecTask(float sec)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetReleaseSecTask;
		task->valuef[0] = sec;
		synther->PostTask(task);
	}

	//设置Release时长
	void Compressor::_SetReleaseSecTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetReleaseSec(effectTask->valuef[0]);
	}

	//设置比值
	void Compressor::SetRadioTask(float radio)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetRadioTask;
		task->valuef[0] = radio;
		synther->PostTask(task);
	}

	//设置比值
	void Compressor::_SetRadioTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetRadio(effectTask->valuef[0]);
	}

	//设置门限
	void Compressor::SetThresholdTask(float threshold)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetThresholdTask;
		task->valuef[0] = threshold;
		synther->PostTask(task);
	}

	//设置门限
	void Compressor::_SetThresholdTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetThreshold(effectTask->valuef[0]);
	}

	//设置拐点的软硬
	void Compressor::SetKneeWidthTask(float width)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetKneeWidthTask;
		task->valuef[0] = width;
		synther->PostTask(task);
	}

	//设置拐点的软硬
	void Compressor::_SetKneeWidthTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetKneeWidth(effectTask->valuef[0]);
	}


	//设置增益补偿
	void Compressor::SetMakeupGainTask(float gain)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetMakeupGainTask;
		task->valuef[0] = gain;
		synther->PostTask(task);
	}

	//设置增益补偿
	void Compressor::_SetMakeupGainTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetMakeupGain(effectTask->valuef[0]);
	}

	//设置增益补偿
	void Compressor::SetAutoMakeupGainTask(bool isAuto)
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _SetAutoMakeupGainTask;
		task->valuei[0] = isAuto;
		synther->PostTask(task);
	}

	//设置是否自动增益补偿
	void Compressor::_SetAutoMakeupGainTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.SetAutoMakeupGain((bool)effectTask->valuei[0]);
	}


	//计算系数
	void Compressor::CalculateCoefficientsTask()
	{
		if (synther == nullptr)
			return;

		EffectTask* task = new EffectTask(this);
		task->processCallBack = _CalculateCoefficientsTask;
		synther->PostTask(task);
	}

	//设置是否自动增益补偿
	void Compressor::_CalculateCoefficientsTask(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		Compressor& compressor = *(Compressor*)(effectTask->effect);
		compressor.CalculateCoefficients();
	}

}
