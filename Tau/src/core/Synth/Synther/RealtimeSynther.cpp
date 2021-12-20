#include"RealtimeSynther.h"
#include"Synth/Tau.h"
#include"Synth/VirInstrument.h"
#include"Synth/Editor/MidiEditor.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"


namespace tau
{

	RealtimeSynther::RealtimeSynther(Tau* tau)
		:Synther(tau)
	{
		realtimeKeyOpTaskProcesser = new TaskProcesser;
		realtimeKeyEventList = new RealtimeKeyEventList;

	}

	RealtimeSynther::~RealtimeSynther()
	{
		RemoveAllVirInstrument();

		//
		ReqDeleteTask();
		waitDelSem.wait();

		//
		realtimeKeyOpTaskProcesser->Stop();


		DEL(realtimeKeyEventList);
		DEL(realtimeKeyOpTaskProcesser);

	}

	void RealtimeSynther::Open()
	{
		Synther::Open();
		realtimeKeyOpTaskProcesser->Start();
	}

	void RealtimeSynther::Close()
	{
		RemoveAllVirInstrument();
		ReqDeleteTask();
		waitDelSem.wait();

		realtimeKeyOpTaskProcesser->Stop();

		Synther::Close();
		isReqDelete = false;
	}

	//投递实时按键操作任务
	void RealtimeSynther::PostRealtimeKeyOpTask(Task* task, int delay)
	{
		realtimeKeyOpTaskProcesser->PostTask(task, delay);
	}

	/// <summary>
	/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在将在虚拟乐器列表中自动创建它
	/// </summary>
	/// <param name="deviceChannel">乐器所在设备通道</param>
	/// <param name="bankSelectMSB">声音库选择0</param>
	/// <param name="bankSelectLSB">声音库选择1</param>
	/// <param name="instrumentNum">乐器编号</param>
	/// <returns></returns>
	VirInstrument* RealtimeSynther::EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		Channel* channel = GetDeviceChannel(deviceChannelNum);
		if (channel == nullptr)
		{
			channel = new Channel(deviceChannelNum);
			deviceChannelMap[deviceChannelNum] = channel;
		}

		return Synther::EnableVirInstrument(channel, bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	// 删除虚拟乐器
	void RealtimeSynther::DelVirInstrument(VirInstrument* virInst)
	{
		if (virInst == nullptr)
			return;

		RemoveVirInstFromList(virInst);

		//
		if (virInst->IsRealtime())
		{
			//移除对应的设备通道
			Channel* channel = virInst->GetChannel();
			int num = channel->GetChannelNum();

			auto itc = deviceChannelMap.find(num);
			if (itc != deviceChannelMap.end())
			{
				deviceChannelMap.erase(itc);
				DEL(channel);
			}
		}

		//
		DEL(virInst);
	}

	//根据设备通道号获取设备通道
	//查找不到将返回空值
	Channel* RealtimeSynther::GetDeviceChannel(int deviceChannelNum)
	{
		auto it = deviceChannelMap.find(deviceChannelNum);
		if (it != deviceChannelMap.end()) {
			return it->second;
		}

		return nullptr;
	}


	//按下按键
	void RealtimeSynther::OnKey(int key, float velocity, VirInstrument* virInst)
	{
		RealtimeKeyEvent ev;
		ev.isOnKey = true;
		ev.key = key;
		ev.velocity = velocity;
		ev.virInst = virInst;
		ev.timeSec = GetCurrentTimeMsec() * 0.001f;

		cmdLock.lock();

		if (realtimeKeyEventList->empty())
		{
			ev.sampleSec = sec;
		}
		else
		{
			RealtimeKeyEvent& lastev = realtimeKeyEventList->back();
			ev.sampleSec = lastev.sampleSec + ev.timeSec - lastev.timeSec;
		}

		realtimeKeyEventList->push_back(ev);

		cmdLock.unlock();
	}

	// 释放按键
	void RealtimeSynther::OffKey(int key, float velocity, VirInstrument* virInst)
	{
		RealtimeKeyEvent ev;
		ev.isOnKey = false;
		ev.key = key;
		ev.velocity = velocity;
		ev.virInst = virInst;
		ev.timeSec = GetCurrentTimeMsec() * 0.001f;

		cmdLock.lock();

		if (realtimeKeyEventList->empty())
		{
			ev.sampleSec = sec;
		}
		else
		{
			RealtimeKeyEvent& lastev = realtimeKeyEventList->back();
			ev.sampleSec = lastev.sampleSec + ev.timeSec - lastev.timeSec;
		}

		realtimeKeyEventList->push_back(ev);
		cmdLock.unlock();

	}


	// 渲染每帧音频
	void RealtimeSynther::Render()
	{

		if (isReqDelete && isSoundEnd)
		{
			if (isMainSynther)
			{
				computedFrameBufSyntherCount--;
				if (computedFrameBufSyntherCount == 0)
					isFrameRenderCompleted = true;
			}
			else
			{
				RealtimeSynther& mainSynther = *(RealtimeSynther*)(tau->mainEditorSynther);
				mainSynther.computedFrameBufSyntherCount--;
				if (mainSynther.computedFrameBufSyntherCount == 0)
					mainSynther.isFrameRenderCompleted = true;
			}

			waitDelSem.set();
			return;
		}


		//清除通道buffer
		ClearChannelBuffer();

		for (childFramePos = 0; childFramePos < frameSampleCount; childFramePos += tau->childFrameSampleCount)
		{
			curtSampleCount += tau->childFrameSampleCount;
			sec = tau->invSampleProcessRate * curtSampleCount;

			if (!isReqDelete)
			{
				ProcessRealtimeKeyEvents();
				ProcessMidiEvents();
			}

			//渲染虚拟乐器区域发声
			RenderVirInstRegionSound();

			//移除已完成采样的KeySounder
			RemoveProcessEndedKeySounder();

		}

		//混合所有乐器中的样本到synther的声道buffer中
		MixVirInstsSamplesToChannelBuffer();
		CombineSynthersFrameBufs();
	}


	// 处理实时onkey或者offkey事件
	// 当frameSampleCount过大时，audio()正在发声时，此时Synther渲染线程的渲染速度高于audio线程，
	// synther线程提前停止工作
	// 如果在此期间发送onkey, 紧接着发送offkey操作，由于synther线程还未启动
	// (如果算法中必须等待audio线程发送渲染指令才启动)，这个onkey操作将不会发出声音
	// 修正算法: 调用ProcessRealtimeKeyEvents()处理
	void RealtimeSynther::ProcessRealtimeKeyEvents()
	{
		cmdLock.lock();

		if (realtimeKeyEventList->empty()) {
			cmdLock.unlock();
			return;
		}

		auto virInstEnd = virInstSet.end();
		RealtimeKeyEventList::iterator it = realtimeKeyEventList->begin();
		for (; it != realtimeKeyEventList->end(); )
		{
			RealtimeKeyEvent& ev = *it;
			if (ev.sampleSec > sec)
				break;

			if (virInstSet.find(ev.virInst) == virInstEnd)
				continue;

			if (ev.isOnKey)
			{
				ev.virInst->OnKey(ev.key, ev.velocity);
			}
			else
			{
				ev.virInst->OffKey(ev.key, ev.velocity);
			}

			it = realtimeKeyEventList->erase(it);
		}

		cmdLock.unlock();
	}



}
