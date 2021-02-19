#include"EffectCompressorCmd.h"

namespace ventrue
{
	//设置采样频率
	void EffectCompressorCmd::SetSampleFreq(float freq)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetSampleFreq;
		task->valuef[0] = freq;
		ventrue->PostTask(task);
	}

	//设置采样频率
	void EffectCompressorCmd::_SetSampleFreq(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetSampleFreq(effectTask->valuef[0]);
	}

	//设置Attack时长
	void EffectCompressorCmd::SetAttackSec(float sec)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetAttackSec;
		task->valuef[0] = sec;
		ventrue->PostTask(task);
	}

	//设置Attack时长
	void EffectCompressorCmd::_SetAttackSec(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetAttackSec(effectTask->valuef[0]);
	}


	//设置Release时长
	void EffectCompressorCmd::SetReleaseSec(float sec)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetReleaseSec;
		task->valuef[0] = sec;
		ventrue->PostTask(task);
	}

	//设置Release时长
	void EffectCompressorCmd::_SetReleaseSec(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetReleaseSec(effectTask->valuef[0]);
	}

	//设置比值
	void EffectCompressorCmd::SetRadio(float radio)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetRadio;
		task->valuef[0] = radio;
		ventrue->PostTask(task);
	}

	//设置比值
	void EffectCompressorCmd::_SetRadio(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetRadio(effectTask->valuef[0]);
	}

	//设置门限
	void EffectCompressorCmd::SetThreshold(float threshold)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetThreshold;
		task->valuef[0] = threshold;
		ventrue->PostTask(task);
	}

	//设置门限
	void EffectCompressorCmd::_SetThreshold(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetThreshold(effectTask->valuef[0]);
	}

	//设置拐点的软硬
	void EffectCompressorCmd::SetKneeWidth(float width)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetKneeWidth;
		task->valuef[0] = width;
		ventrue->PostTask(task);
	}

	//设置拐点的软硬
	void EffectCompressorCmd::_SetKneeWidth(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetKneeWidth(effectTask->valuef[0]);
	}


	//设置增益补偿
	void EffectCompressorCmd::SetMakeupGain(float gain)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetMakeupGain;
		task->valuef[0] = gain;
		ventrue->PostTask(task);
	}

	//设置增益补偿
	void EffectCompressorCmd::_SetMakeupGain(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetMakeupGain(effectTask->valuef[0]);
	}

	//设置增益补偿
	void EffectCompressorCmd::SetAutoMakeupGain(bool isAuto)
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _SetAutoMakeupGain;
		task->valuei[0] = isAuto;
		ventrue->PostTask(task);
	}

	//设置是否自动增益补偿
	void EffectCompressorCmd::_SetAutoMakeupGain(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.SetAutoMakeupGain((bool)effectTask->valuei[0]);
	}


	//计算系数
	void EffectCompressorCmd::CalculateCoefficients()
	{
		EffectTask* task = new EffectTask(effect);
		task->processCallBack = _CalculateCoefficients;
		ventrue->PostTask(task);
	}

	//设置是否自动增益补偿
	void EffectCompressorCmd::_CalculateCoefficients(Task* task)
	{
		EffectTask* effectTask = (EffectTask*)task;
		EffectCompressor& compressor = *(EffectCompressor*)(effectTask->effect);
		compressor.CalculateCoefficients();
	}

}
