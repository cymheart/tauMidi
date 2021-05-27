#ifndef _MidiPlay_h_
#define _MidiPlay_h_

#include "VentrueTypes.h"
#include "Midi/MidiTypes.h"

namespace ventrue
{

	// MidiPlay状态
	enum class MidiPlayState
	{
		//停止
		STOP,
		//播放
		PLAY,
		//暂停
		SUSPEND
	};

	/// <summary>
	/// Midi文件播放类
	/// by cymheart, 2020--2021.
	/// </summary> 
	class MidiPlay
	{
	public:
		MidiPlay(Ventrue* ventrue);
		~MidiPlay();

		//获取其中的midiFile
		inline MidiFile* GetMidiFile()
		{
			return midiFile;
		}

		//获取结束时间
		inline float GetEndSec()
		{
			return endSec;
		}

		//停止播放
		void Stop();
		//开始播放
		void Play();
		//暂停播放
		void Suspend();
		//移除
		void Remove();

		//设置播放的起始时间点
		void Goto(double gotoSec);
		//设置快进到开头
		void GotoStart();
		//设置快进到结尾
		void GotoEnd();



		//打开轨道通道对应的虚拟乐器
		void OnVirInstrumentByTrackChannel(int trackIdx, int channelIdx);
		//关闭轨道通道对应的虚拟乐器
		void OffVirInstrumentByTrackChannel(int trackIdx, int channelIdx);
		//移除轨道通道对应的虚拟乐器
		void RemoveVirInstrumentByTrackChannel(int trackIdx, int channelIdx);
		// 解析MidiFile
		void ParseMidiFile(string midiFilePath, TrackChannelMergeMode mode);
		//轨道运行
		void TrackRun(double sec);
		void DisableTrack(int trackIdx);
		void DisableAllTrack();
		void EnableTrack(int trackIdx);
		void EnableAllTrack();
		void DisableTrackChannel(int trackIdx, int channel);
		void EnableTrackChannel(int trackIdx, int channel);
		void DisableTrackAllChannels(int trackIdx);
		void EnableTrackAllChannels(int trackIdx);



		//设置打击乐号
		void SetPercussionProgramNum(int num);

	private:
		void Clear();
		void TrackPlayCore(double sec);
		//处理轨道事件
		void ProcessTrackEvent(MidiEvent* midEv, int trackIdx, double sec);

	private:

		MidiPlayState state = MidiPlayState::STOP;

		Ventrue* ventrue = nullptr;
		MidiFile* midiFile = nullptr;
		MidiTrackList* midiTrackList = nullptr;


		// 音轨演奏信息
		TrackList trackList;

		//辅助midi事件列表
		MidiEventList assistMidiEvList;

		//辅助轨道
		Track* assistTrack;

		//结束时间点
		float endSec = 0;

		//快进到指定时间点
		double gotoSec = 0;

		//是否快进到结尾
		bool isGotoEnd = false;

		//播放开始时间点
		double startTime = 0;

		//是否首次播放
		bool isOpen = false;

		//是否快进
		bool isDirectGoto = false;

		//是否计算事件时间
		bool isComputeEventTime = false;

		//打击乐号
		int percussionProgramNum = 0;

	};
}

#endif
