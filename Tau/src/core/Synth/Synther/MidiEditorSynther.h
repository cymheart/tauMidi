#ifndef _MidiEditorSynther_h_
#define _MidiEditorSynther_h_

#include "Synth/TauTypes.h"
#include "RealtimeSynther.h"



namespace tau
{
	/*
	   midi编辑合成器
	*/
	class MidiEditorSynther : public RealtimeSynther
	{
	public:
		MidiEditorSynther(Tau* tau);
		virtual ~MidiEditorSynther();

		virtual SyntherType GetType() { return SyntherType::MidiEditor; }

	public:

		MidiEditor* GetMidiEditor()
		{
			return midiEditor;
		}

		//建立MidiEditor
		MidiEditor* CreateMidiEditor();

		void EnterWaitPlayModeTask(Semaphore* waitSem);
		void LeaveWaitPlayModeTask(Semaphore* waitSem);

		//播放
		void PlayTask(Semaphore* waitSem);

		//停止
		void StopTask(Semaphore* waitSem);

		//暂停
		void PauseTask(Semaphore* waitSem);

		//移除
		void RemoveTask(Semaphore* waitSem);

		void RuntoTask(Semaphore* waitSem, double sec);

		//指定播放的起始时间点
		void GotoTask(Semaphore* waitSem, double sec);

		//获取状态
		EditorState GetStateTask();

		//获取结束时间(单位:秒)
		float GetEndSecTask();

		//设置结束时间(单位:秒)
		void SetEndSecTask(Semaphore* waitSem, double sec);

		// 设定播放速度
		void SetSpeedTask(Semaphore* waitSem, float speed);

		// 禁止播放指定编号的轨道
		void DisableTrackTask(Semaphore* waitSem, Track* track);
		// 禁止播放所有轨道
		void DisableAllTrackTask(Semaphore* waitSem);

		// 启用播放指定的轨道
		void EnableTrackTask(Semaphore* waitSem, Track* track);
		// 启用播放所有轨道
		void EnableAllTrackTask(Semaphore* waitSem);

		// 禁止播放指定编号通道
		void DisableChannelTask(Semaphore* waitSem, int channelIdx);

		// 启用播放指定编号通道
		void EnableChannelTask(Semaphore* waitSem, int channelIdx);

		// 设置对应轨道的乐器
		void SetVirInstrumentTask(
			Semaphore* waitSem, Track* track,
			int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置打击乐号
		void SetBeatVirInstrumentTask(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		void SetMarkerListTask(Semaphore* waitSem, MidiMarkerList* mrklist);
		void NewTrackTask(Semaphore* waitSem);
		void DeleteTrackTask(Semaphore* waitSem, Track* track);
		void MoveInstFragmentTask(Semaphore* waitSem, InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec);
		void RemoveInstFragmentTask(Semaphore* waitSem, InstFragment* instFragment);
		void ComputeTrackEventsTimeTask(Semaphore* waitSem, Track* track);
		void ComputeEndSecTask(Semaphore* waitSem);

	private:
		//
		static void _EnterWaitPlayModeTask(Task* ev);
		static void _LeaveWaitPlayModeTask(Task* ev);

		static void _PlayTask(Task* ev);
		static void _StopTask(Task* ev);
		static void _PauseTask(Task* ev);
		static void _RemoveTask(Task* ev);
		static void _RuntoTask(Task* ev);
		static void _GotoTask(Task* ev);
		static void _GetStateTask(Task* ev);
		static void _GetEndSecTask(Task* ev);
		static void _SetEndSecTask(Task* ev);
		static void _SetSpeedTask(Task* ev);
		static void _DisableTrackTask(Task* ev);
		static void _EnableTrackTask(Task* ev);
		static void _DisableChannelTask(Task* ev);
		static void _EnableChannelTask(Task* ev);
		static void _SetVirInstrumentTask(Task* ev);
		static void _SetBeatVirInstrumentTask(Task* ev);
		static void _SetMarkerListTask(Task* ev);
		static void _NewTrackTask(Task* ev);
		static void _DeleteTrackTask(Task* ev);
		static void _MoveInstFragmentTask(Task* ev);
		static void _RemoveInstFragmentTask(Task* ev);
		static void _ComputeTrackEventsTimeTask(Task* ev);
		static void _ComputeEndSecTask(Task* ev);


	protected:

		// 处理播放midi文件事件
		virtual void ProcessMidiEvents();

		void EnterWaitPlayMode();
		void LeaveWaitPlayMode();

		//播放midi
		void Play();

		//停止播放midiEditor
		void Stop();

		//暂停播放midiEditor
		void Pause();

		//移除midiEditor
		void Remove();

		void Runto(double sec);

		//midi快进到指定位置
		void Goto(double sec);

		//获取midi播放时间
		float GetPlaySec();

		//获取midi状态
		EditorState GetState();

		//获取midi结束时间(单位:秒)
		float GetEndSec();

		//设置midi结束时间(单位:秒)
		void SetEndSec(double sec);

		//设置midi播放速度
		void SetSpeed(float speed);

		// 禁止播放指定的轨道
		void DisableTrack(Track* track);

		// 启用播放指定的轨道
		void EnableTrack(Track* track);

		// 禁止播放指定编号Midi文件通道
		void DisableChannel(int channelIdx);

		// 启用播放指定编号Midi文件通道
		void EnableChannel(int channelIdx);


		// 设置midi对应轨道通道的乐器
		void SetVirInstrument(Track* track,
			int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//为midi文件设置打击乐号
		void SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置标记
		void SetMarkerList(MidiMarkerList* mrklist);

		//新建轨道
		void NewTrack();

		//删除轨道
		void DeleteTrack(Track* track);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec);

		//移除乐器片段
		void RemoveInstFragment(InstFragment* instFragment);

		//计算指定轨道所有事件的实际时间点
		void ComputeTrackEventsTime(Track* track);

		//计算结束时间点
		void ComputeEndSec();

	protected:

		MidiEditor* midiEditor = nullptr;

		friend class Synther;
		friend class Tau;
		friend class Editor;
		friend class MidiEditor;

	};
}

#endif
