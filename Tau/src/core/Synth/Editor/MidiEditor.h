#ifndef _MidiEditor_h_
#define _MidiEditor_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"
#include "EditorTypes.h"
#include"MidiMarkerList.h"

namespace tau
{



	/// <summary>
	/// Midi编辑类
	/// by cymheart, 2020--2021.
	/// </summary> 
	class MidiEditor
	{
	public:
		MidiEditor(MidiEditorSynther* midiSynther);
		~MidiEditor();

		MidiEditorSynther* GetSynther()
		{
			return midiSynther;
		}

		//获取midi文件路径
		inline string& GetMidiFilePath()
		{
			return filePath;
		}

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

		//获取goto时间
		inline float GetGotoSec()
		{
			return (float)gotoSec;
		}

		//获取播放时间
		inline float GetPlaySec()
		{
			return curtPlaySec;
		}

		//获取播放速率(相对于正常播放速率1.0的倍率)
		inline float GetSpeed()
		{
			return speed;
		}

		//获取轨道数量
		int GetTrackCount()
		{
			return trackCount;
		}


		//停止播放
		void Stop();
		//开始播放
		void Play();
		//暂停播放
		void Pause();
		//移除
		void Remove();

		//进入到步进播放模式
		void EnterStepPlayMode();
		//离开步进播放模式
		void LeaveStepPlayMode();

		//移动到指定时间点
		void Moveto(double sec);
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


		void StepRun(double sec);
		//运行
		void Run(double sec, bool isStepOp = false);
		void DisableTrack(Track* track);
		void DisableAllTrack();
		void EnableTrack(Track* track);
		void EnableAllTrack();
		void DisableChannel(int channelIdx);
		void EnableChannel(int channelIdx);

		//设置轨道乐器
		void SetVirInstrument(Track* track, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置打击乐器
		void SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//计算指定轨道所有事件的实际时间点
		void ComputeTrackEventsTime(Track* track);

		//计算结束时间点
		void ComputeEndSec();

	private:

		void RunCore(double sec);
		//处理轨道事件
		void ProcessEvent(MidiEvent* midEv, int trackIdx);

	private:

		EditorState state = EditorState::STOP;

		Tau* tau;
		MidiEditorSynther* midiSynther;
		MidiMarkerList midiMarkerList;

		string filePath;

		float midiTrackTickForQuarterNote = 0;

		// 轨道
		vector<Track*> trackList;
		int trackCount = 0;

		//轨道缓存
		vector<Track*> tempTracks;

		//结束时间点
		double endSec = 0;

		//快进到指定时间点
		double gotoSec = 0;

		//是否快进到结尾
		bool isGotoEnd = false;

		//播放速率(相对于正常播放速率1.0的倍率)
		float speed = 1;

		//是否首次播放
		bool isOpen = false;

		//是否快进
		bool isDirectGoto = false;

		//是否是步进播放模式
		bool isStepPlayMode = false;

		//当前播放时间
		double curtPlaySec = 0;


		friend class Editor;
	};
}

#endif
