#ifndef _RealtimeSynther_h_
#define _RealtimeSynther_h_

#include "Synth/TauTypes.h"
#include "Synther.h"



namespace tau
{
	/*
	   实时合成器
	*/
	class RealtimeSynther : public Synther
	{
	public:
		RealtimeSynther(Tau* tau);
		virtual ~RealtimeSynther();

		virtual SyntherType GetType() { return SyntherType::Realtime; }

	public:

		virtual void Open();
		virtual void Close();

		// 按下按键
		void OnKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS = 0);

		// 释放按键
		void OffKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS = 0);

		// 取消按键任务
		void CancelDownKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS = 0);

		// 取消释放按键任务
		void CancelOffKeyTask(int key, float velocity, VirInstrument* virInst, int delayMS = 0);


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
		VirInstrument* EnableVirInstrumentTask(
			int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);


	private:

		static void _OnKeyTask(Task* ev);
		static void _OffKeyTask(Task* ev);
		static bool _CmpKeyTask(Task* curtTask, Task* cmpTask);
		static void _EnableInstrumentTask(Task* ev);

	protected:



		// 处理实时onkey或者offkey事件
		// 当frameSampleCount过大时，audio()正在发声时，此时Synther渲染线程的渲染速度高于audio线程，
		// synther线程提前停止工作
		// 如果在此期间发送onkey, 紧接着发送offkey操作，由于synther线程还未启动
		// (如果算法中必须等待audio线程发送渲染指令才启动)，这个onkey操作将不会发出声音
		// 修正算法: 调用ProcessRealtimeKeyEvents()处理
		void PostRealtimeKeyOpTask(Task* task, int delay = 0);

		// 渲染每帧音频
		virtual void Render();

		// 处理播放midi文件事件
		virtual void ProcessMidiEvents()
		{

		}

		/// <summary>
		/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在将在虚拟乐器列表中自动创建它
		/// </summary>
		/// <param name="deviceChannel">乐器所在设备通道</param>
		/// <param name="bankSelectMSB">声音库选择0</param>
		/// <param name="bankSelectLSB">声音库选择1</param>
		/// <param name="instrumentNum">乐器编号</param>
		/// <returns></returns>
		VirInstrument* EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 删除虚拟乐器
		void DelVirInstrument(VirInstrument* virInst);

		//根据设备通道号获取设备通道
		//查找不到将返回空值
		Channel* GetDeviceChannel(int deviceChannelNum);

		//按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst);

		// 释放按键
		void OffKey(int key, float velocity, VirInstrument* virInst);

		// 处理实时onkey或者offkey事件
		void ProcessRealtimeKeyEvents();

	protected:

		TaskProcesser* realtimeKeyOpTaskProcesser = nullptr;

		//设备通道列表
		unordered_map<int, Channel*> deviceChannelMap;
		RealtimeKeyEventList* realtimeKeyEventList = nullptr;
		mutex cmdLock;


		friend class Synther;
		friend class Tau;
		friend class Editor;
		friend class MidiEditor;

	};
}

#endif
