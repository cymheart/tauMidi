#ifndef _MidiEditor_h_
#define _MidiEditor_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"
#include "EditorTypes.h"
#include"MidiMarkerList.h"
#include"task/Task.h"

using namespace task;

namespace tau
{

	/// <summary>
	/// Midi编辑类
	/// by cymheart, 2020--2021.
	/// </summary> 
	class MidiEditor
	{
	public:
		MidiEditor(Synther* synther);
		~MidiEditor();

		inline EditorState GetState()
		{
			return state;
		}

		//获取结束时间
		inline double GetEndSec()
		{
			return endSec;
		}

		//设置结束时间
		inline void SetEndSec(double sec)
		{
			endSec = sec;
		}

		//获取结束tick
		inline int GetEndTick()
		{
			return endTick;
		}


		//获取播放时间
		inline double GetPlaySec()
		{
			return curtPlaySec;
		}

		//获取播放速率(相对于正常播放速率1.0的倍率)
		inline float GetSpeed()
		{
			return speed;
		}

		//获取轨道数量
		inline int GetTrackCount()
		{
			return trackCount;
		}

		//设置弹奏方式
		inline void SetPlayType(MidiEventPlayType _playType)
		{
			playType = _playType;
		}

		//获取播放模式
		inline EditorPlayMode GetPlayMode()
		{
			return playMode;
		}

		//进入播放模式
		inline void EnterPlayMode(EditorPlayMode _playMode)
		{
			playMode = _playMode;
		}

		//离开播放模式
		inline void LeavePlayMode()
		{
			playMode = EditorPlayMode::Common;
		}

		//停止播放
		void Stop();
		//开始播放
		void Play();
		//暂停播放
		void Pause();
		//移除
		void Remove();

		//移动到指定时间点
		void Runto(double sec);
		//设置播放的起始时间点
		void Goto(double sec);
		//设置快进到开头
		void GotoStart();
		//设置快进到结尾
		void GotoEnd();

		//设置播放速率(相对于正常播放速率1.0的倍率)
		void SetSpeed(float speed_);

		//新建轨道，空轨道
		void NewTrack();

		//删除轨道
		void DeleteTrack(Track* track);

		//设置标记
		void SetMarkerList(MidiMarkerList* mrklist);

		//移除乐器片段
		void RemoveInstFragment(InstFragment* instFragment);

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec);

		//计算指定轨道所有事件的实际时间点
		void ComputeTrackEventsTime(Track* track);

		//计算结束时间点
		void ComputeEndSec();

		//计算结束tick点
		void ComputeEndTick();

		//当前时间curtPlaySec往前处理一个sec的时间长度的所有midi事件
		void Process(double sec, bool isStepOp = false);

		void DisableTrack(Track* track);
		void DisableTrack(int trackIdx);
		void DisableAllTrack();
		void EnableTrack(Track* track);
		void EnableTrack(int trackIdx);
		void EnableAllTrack();
		void DisableChannel(int channelIdx);
		void EnableChannel(int channelIdx);

		//设置轨道事件演奏方式
		void SetTrackPlayType(Track* track, MidiEventPlayType playType);

		//设置轨道乐器
		void SetVirInstrument(Track* track, int bankSelectMSB, int bankSelectLSB, int instrumentNum);
		//设置轨道乐器
		void SetVirInstrument(int trackIdx, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置打击乐器
		void SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);


	private:

		//设置简单模式下, 白色按键的数量
		void SetSimpleModePlayWhiteKeyCount(int count)
		{
			simpleModePlayWhiteKeyCount = count;
		}

		//生成简单模式音符轨道
		void CreateSimpleModeTrack();

		LinkedListNode<MidiEvent*>* FindFitNoteOffEventNode(LinkedListNode<MidiEvent*>* startNode);

		void CopyChildNoteOnEv(
			NoteOnEvent* newNoteEv, NoteOffEvent* newNoteOffEv,
			NoteOnEvent* ev, int note);

		static bool NoteCmp(MidiEvent* a, MidiEvent* b);

		//当前时间curtPlaySec往前处理一个sec的时间长度的所有midi事件
		void ProcessCore(double sec, bool isDirectGoto = false);

		void ProcessTrack(Track* track, bool isDirectGoto);

		//处理轨道事件
		void ProcessEvent(MidiEvent* midEv, Track* track, bool isDirectGoto);

		void ProcessSimpleModeTrack(Track* track, bool isDirectGoto);


		//是否需要等待按键信号
		bool IsNeedWaitKeySignal(MidiEvent* midEv, Track* track);

		//是否为手指弹奏的音符
		bool IsPointerPlayNote(MidiEvent* ev, Track* track);

		//设置当前播放时间
		void SetCurtPlaySec(double sec);

		//设置状态
		void SetState(EditorState s);

	private:

		Tau* tau = nullptr;
		Editor* editor = nullptr;
		Synther* midiSynther = nullptr;
		MidiMarkerList* midiMarkerList;

		//简单模式轨道音符
		LinkedList<MidiEvent*> simpleModeTrackNotes;
		Track* simpleModeTrack = nullptr;
		//简单模式轨道音符偏移
		LinkedListNode<MidiEvent*>* simpleModeTrackNotesOffset = nullptr;
		//简单模式下, 白色按键的数量
		int simpleModePlayWhiteKeyCount = 7;
		//被合并音符的最大时长
		float mergeSimpleSrcNoteLimitSec = 0.25f;
		//合并到目标音符的最大时长
		float mergeSimpleDestNoteLimitSec = 0.4f;

		//按0~128key分类的noteOnEvents
		vector<NoteOnEvent*> classifyNoteOnEvents[128];

		//
		EditorState state = EditorState::STOP;

		// 轨道
		vector<Track*> tracks;
		int trackCount = 0;

		//结束时间点
		double endSec = 0;

		//结束tick点
		double endTick = 0;

		//播放速率(相对于正常播放速率1.0的倍率)
		float speed = 1;

		//播放模式
		EditorPlayMode playMode = EditorPlayMode::Common;

		//演奏类型
		MidiEventPlayType playType = MidiEventPlayType::DoubleHand;

		//当前播放时间
		double curtPlaySec = 0;

		friend class Editor;
		friend class Track;
	};
}

#endif
