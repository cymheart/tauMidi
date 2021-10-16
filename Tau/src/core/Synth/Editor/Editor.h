#ifndef _Editor_h_
#define _Editor_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"
#include"EditorTypes.h"
#include"MidiMarkerList.h"

namespace tau
{

	/// <summary>
	/// 编辑类
	/// by cymheart, 2020--2021.
	/// </summary> 
	class Editor
	{


	public:
		Editor(Tau* tau);
		~Editor();

		inline const vector<Track*>& GetTracks()
		{
			return tracks;
		}

		//获取结束时间
		inline float GetEndSec()
		{
			return endSec;
		}

		//获取播放速率(相对于正常播放速率1.0的倍率)
		inline float GetSpeed()
		{
			return speed;
		}

		//载入
		void Load(string& midiFilePath);

		//移除
		void Remove();

		//播放
		void Play();

		//停止
		void Stop();

		//暂停
		void Pause();

		//进入到步进播放模式
		void EnterStepPlayMode();
		//离开步进播放模式
		void LeaveStepPlayMode();

		//进入到等待播放模式
		void EnterWaitPlayMode();

		//离开等待播放模式
		void LeaveWaitPlayMode();

		//等待(区别于暂停，等待相当于在原始位置播放)
		void Wait();
		//继续，相对于等待命令
		void Continue();

		//按键信号
		void OnKeySignal(int key);
		//松开按键信号
		void OffKeySignal(int key);

		//移动到指定时间点
		void Runto(double sec);

		//设置播放时间点
		void Goto(double sec);

		//获取状态
		EditorState GetState();

		//获取当前播放时间点
		double GetPlaySec();

		// 设定速度
		void SetSpeed(float speed_);

		// 禁止播放指定编号的轨道
		void DisableTrack(int trackIdx);

		// 禁止播放指定编号的轨道
		void DisableTracks(vector<int>& trackIdxs);

		// 禁止播放所有轨道
		void DisableAllTrack();

		// 启用播放指定编号的轨道
		void EnableTrack(int trackIdx);

		// 启用播放指定编号的轨道
		void EnableTracks(vector<int>& trackIdxs);

		// 启用播放所有轨道
		void EnableAllTrack();

		// 禁止播放指定编号通道
		void DisableChannel(int channelIdx);

		// 启用播放指定编号通道
		void EnableChannel(int channelIdx);

		//设置轨道事件演奏方式
		void SetTrackPlayType(int trackIdx, MidiEventPlayType playType);

		// 设置对应轨道的乐器
		void SetVirInstrument(int trackIdx, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置打击乐号
		void SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);


		//增加轨道，来自于MidiTrackList
		void AddMidiTracks(
			vector<MidiTrack*>& midiTracks,
			int start = -1, int end = -1,
			Track* dstFirstTrack = nullptr, int dstFirstBranchIdx = 0, float sec = 0);

		//增加标记，来自于midi事件列表中
		void AddMarkers(list<MidiEvent*>& midiEvents);

		//新建轨道
		void NewTracks(int count);

		//删除轨道
		void DeleteTracks(vector<Track*>& tracks);


		void SetSelectInstFragMode(SelectInstFragMode mode)
		{
			selectInstFragMode = mode;
		}

		void SelectInstFragment(int trackIdx, int branchIdx, int instFragIdx);
		void UnSelectInstFragment(int trackIdx, int branchIdx, int instFragIdx);

		void UnSelectAllInstFragment()
		{
			selectedInstFrags.clear();
		}

		InstFragment* GetInstFragment(int trackIdx, int branchIdx, int instFragIdx);

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveSelectedInstFragment(int dstTrack, int dstBranchIdx, float sec);

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveSelectedInstFragments(vector<int>& dstTracks, vector<int>& dstBranchIdx, vector<float>& secs);


	private:

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveInstFragments(
			vector<InstFragment*>& instFragments,
			vector<Track*>& dstTracks, vector<int>& dstBranchIdxs, vector<float>& secs);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragments(vector<InstFragment*>& instFragments, vector<Track*>& dstTracks, vector<float>& secs);

		//移动乐器片段到指定时间点
		void MoveInstFragments(vector<InstFragment*>& instFragments, vector<float>& secs);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragment(InstFragment* instFragment, Track* dstTrack, float sec);

		//移动乐器片段到指定时间点
		void MoveInstFragment(InstFragment* instFragment, float sec);

		//需要按键信号
		void NeedOnKeySignal(int key);

		//需要松开按键信号
		void NeedOffKeySignal(int key);

		//删除空轨实时RealtimeSynther
		void DelEmptyTrackRealtimeSynther();

		int ResetTrackCountNewTracks(int count);
		int _NewTracks(MidiEditorSynther* synther, int count);

		//重新给每个MidiEditor设置相同的最大结束时间
		void ComputeMidiEditorMaxSec();

		//打印工程信息
		void PrintProjectInfo();

	private:

		Tau* tau = nullptr;

		// 音轨
		vector<Track*> tracks;
		MidiMarkerList midiMarkerList;

		//结束时间点
		float endSec = 0;

		//播放速率(相对于正常播放速率1.0的倍率)
		float speed = 1;

		//计算的每个合成器中最大轨道数量
		int computedPerSyntherLimitTrackCount = 20;

		//是否是步进播放模式
		atomic_bool isStepPlayMode = false;
		//是否为等待播放模式
		bool isWaitPlayMode = false;

		//是否等待
		atomic_bool isWait = false;

		mutex waitOnKeyLock;
		int onkey[128] = { 0 };

		int needOnkey[128] = { 0 };
		int needOnKeyCount = 0;

		int needOffkey[128] = { 0 };
		int needOffKeyCount = 0;


		SelectInstFragMode selectInstFragMode = SelectInstFragMode::SingleSelect;
		vector<SelectInstFragmentInfo> selectedInstFrags;


		//
		list<InstFragmentToTrackInfo> orgList;
		unordered_map<MidiEditorSynther*, vector<InstFragmentToTrackInfo>> dataGroup;
		unordered_map<MidiEditorSynther*, unordered_set<Track*>> modifyTrackMap;
		unordered_set<MidiEditorSynther*> syntherSet;
		Semaphore waitSem;


		friend class MidiEditor;

	};
}

#endif
