#ifndef _MidiPlay_h_
#define _MidiPlay_h_

#include "VentrueTypes.h"
#include "Midi/MidiTypes.h"

namespace ventrue
{
	/// <summary>
	/// Midi文件播放类
	/// by cymheart, 2020--2021.
	/// </summary> 
	class MidiPlay
	{
	public:
		MidiPlay();
		~MidiPlay();

		void Stop();

		/// <summary>
		/// 设置发音合成器ventrue
		/// </summary>
		/// <param name="ventrue"></param>
		void SetVentrue(Ventrue* ventrue);

		/// <summary>
		/// 设置midi文件
		/// </summary>
		/// <param name="midiFile"></param>
		void SetMidiFile(MidiFile* midiFile);
		void TrackPlay(double sec);
		void DisableTrack(int trackIdx);
		void DisableAllTrack();
		void EnableTrack(int trackIdx);
		void EnableAllTrack();
		void DisableTrackChannel(int trackIdx, int channel);
		void EnableTrackChannel(int trackIdx, int channel);
		void DisableTrackAllChannels(int trackIdx);
		void EnableTrackAllChannels(int trackIdx);

		//设置播放的起始时间点
		void GotoSec(double gotoSec);

	private:
		void Clear();
		void TrackPlayCore(double sec);
		//处理轨道事件
		void ProcessTrackEvent(MidiEvent* midEv, int trackIdx, double sec);

	private:
		int i = 0;
		Ventrue* ventrue = nullptr;
		MidiFile* midiFile = nullptr;
		MidiTrackList* midiTrackList = nullptr;


		// 音轨演奏信息
		TrackList trackList;

		//辅助midi事件列表
		MidiEventList assistMidiEvList;

		//辅助轨道
		Track* assistTrack;

		//快进到指定时间点
		double gotoSec = 0;

		//播放开始时间点
		double startTime = 0;

		//是否首次播放
		bool isOpen = false;

		//是否快进
		bool isDirectGoto = false;

	};
}

#endif
