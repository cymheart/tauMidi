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

		//设置播放时间点
		void Goto(float sec);

		//获取状态
		EditorState GetState();

		// 设定速度
		void SetSpeed(float speed);

		// 禁止播放指定编号的轨道
		void DisableTrack(int trackIdx);

		// 禁止播放所有轨道
		void DisableAllTrack();

		// 启用播放指定编号的轨道
		void EnableTrack(int trackIdx);

		// 启用播放所有轨道
		void EnableAllTrack();

		// 禁止播放指定编号通道
		void DisableChannel(int channelIdx);

		// 启用播放指定编号通道
		void EnableChannel(int channelIdx);

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

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveInstFragments(
			vector<InstFragment*>& instFragments,
			vector<Track*>& dstTracks, vector<int>& dstBranchIdx, vector<float>& secs);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragments(vector<InstFragment*>& instFragments, vector<Track*>& dstTracks, vector<float>& secs);

		//移动乐器片段到指定时间点
		void MoveInstFragments(vector<InstFragment*>& instFragments, vector<float>& secs);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragment(InstFragment* instFragment, Track* dstTrack, float sec);

		//移动乐器片段到指定时间点
		void MoveInstFragment(InstFragment* instFragment, float sec);

	private:
		//删除空轨实时RealtimeSynther
		void DelEmptyTrackRealtimeSynther();
		int _NewTracks(MidiEditorSynther* synther, int count);

		//重新给每个MidiEditor设置相同的最大结束时间
		void SetMidiEditorMaxSec();

	private:

		Tau* tau = nullptr;

		// 音轨
		vector<Track*> tracks;
		MidiMarkerList midiMarkerList;

		//结束时间点
		float endSec = 0;

		//
		list<InstFragmentToTrackInfo> orgList;
		unordered_map<MidiEditorSynther*, vector<InstFragmentToTrackInfo>> dataGroup;
		unordered_map<MidiEditorSynther*, unordered_set<Track*>> modifyTrackMap;
		Semaphore waitSem;
	};
}

#endif
