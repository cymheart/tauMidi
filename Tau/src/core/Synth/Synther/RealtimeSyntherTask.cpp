#include"RealtimeSynther.h"
#include"Synth/VirInstrument.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"

namespace tau
{

	// 按下按键
	void RealtimeSynther::OnKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _OnKeyTask;
		ev->key = key;
		ev->velocity = velocity;
		ev->virInst = virInst;
		PostRealtimeKeyOpTask(ev, delayMS);
	}

	void RealtimeSynther::_OnKeyTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		RealtimeSynther& realtimeSynther = (RealtimeSynther&)*(se->synther);
		realtimeSynther.OnKey(se->key, se->velocity, se->virInst);
	}

	// 释放按键
	void RealtimeSynther::OffKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _OffKeyTask;
		ev->key = key;
		ev->velocity = velocity;
		ev->virInst = virInst;
		PostRealtimeKeyOpTask(ev, delayMS);
	}

	void RealtimeSynther::_OffKeyTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		RealtimeSynther& realtimeSynther = (RealtimeSynther&)*(se->synther);
		realtimeSynther.OffKey(se->key, se->velocity, se->virInst);
	}

	// 取消按键任务
	void RealtimeSynther::CancelDownKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->msg = TaskMsg::TMSG_TASK_REMOVE;
		ev->synther = this;
		ev->cmpCallBack = _CmpKeyTask;
		ev->key = key;
		ev->velocity = velocity;
		ev->virInst = virInst;
		PostRealtimeKeyOpTask(ev, delayMS);
	}

	// 取消释放按键任务
	void RealtimeSynther::CancelOffKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->msg = TaskMsg::TMSG_TASK_REMOVE;
		ev->synther = this;
		ev->cmpCallBack = _CmpKeyTask;
		ev->key = key;
		ev->velocity = velocity;
		ev->virInst = virInst;
		PostRealtimeKeyOpTask(ev, delayMS);
	}

	bool RealtimeSynther::_CmpKeyTask(Task* curtTask, Task* cmpTask)
	{
		SyntherEvent* curtSE = (SyntherEvent*)curtTask;
		SyntherEvent* cmpSE = (SyntherEvent*)cmpTask;

		if (curtSE->key == cmpSE->key &&
			curtSE->virInst == cmpSE->virInst)
			return true;

		return false;
	}

	/// <summary>
	/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在将在虚拟乐器列表中自动创建它
	/// 注意如果deviceChannelNum已近被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
	/// 而不会同时在一个通道上创建超过1个的虚拟乐器
	/// </summary>
	/// <param name="deviceChannel">乐器所在的设备通道</param>
	/// <param name="bankSelectMSB">声音库选择0</param>
	/// <param name="bankSelectLSB">声音库选择1</param>
	 /// <param name="instrumentNum">乐器编号</param>
	/// <returns></returns>
	VirInstrument* RealtimeSynther::EnableVirInstrumentTask(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		VirInstrument* inst = 0;
		Semaphore waitGetInstrumentSem;

		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _EnableInstrumentTask;
		ev->deviceChannelNum = deviceChannelNum;
		ev->bankSelectLSB = bankSelectLSB;
		ev->bankSelectMSB = bankSelectMSB;
		ev->instrumentNum = instrumentNum;
		ev->ptr = (void*)&inst;
		ev->sem = &waitGetInstrumentSem;
		PostTask(ev);

		//
		waitGetInstrumentSem.wait();
		return inst;
	}

	void RealtimeSynther::_EnableInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		RealtimeSynther& realtimeSynther = (RealtimeSynther&)*(se->synther);
		VirInstrument* vinst =
			realtimeSynther.EnableVirInstrument(
				se->deviceChannelNum,
				se->bankSelectMSB,
				se->bankSelectLSB,
				se->instrumentNum);

		VirInstrument** threadVInst = (VirInstrument**)se->ptr;
		*threadVInst = vinst;
		se->sem->set();
	}

}
