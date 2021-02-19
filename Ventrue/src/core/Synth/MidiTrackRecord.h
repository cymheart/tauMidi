#ifndef _MidiTrackRecord_h_
#define _MidiTrackRecord_h_

#include"VentrueTypes.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"

namespace ventrue
{
	/*
	* midi轨道录制
	* by cymheart, 2020--2021.
	*/
	class MidiTrackRecord
	{
	public:
		MidiTrackRecord();
		~MidiTrackRecord();

		void Clear();

		void SetBPM(float bpm);
		void SetTickForQuarterNote(float tickForQuarterNote);

		//开始录制Midi,支持中途变速录制
		void Start();

		//停止录制
		void Stop();

		//是否正在录制
		inline bool IsRecord()
		{
			return isRecord;
		}

		//获取录制的midi轨道
		MidiTrack* TakeMidiTrack();

		//获取录制的midi轨道
		//baseTickForQuarterNote: 改变轨道的tickForQuarterNote
		MidiTrack* TakeMidiTrack(float baseTickForQuarterNote);

		//变换当前录制时间点为tick数量
		int TransCurTimeToRecordMidiTick();

		//变换录制时间点(单位:ms)为tick数量
		int TransToRecordMidiTick(clock::time_point curtTime);


		//录制SetTempo
		void RecordSetTempo(float microTempo);
		//录制OnKey
		void RecordOnKey(int key, float velocity, int channel);
		//录制OffKey
		void RecordOffKey(int key, float velocity, int channel);
		//录制设置乐器
		void RecordSetProgramNum(int instNum, int channel);
		//录制设置midi控制器
		void RecordSetController(MidiControllerType type, int value, int channel);
		//录制设置pitchBend
		void RecordSetPitchBend(int value, int channel);

	private:
		//是否录制Midi
		bool isRecord = false;
		//录制midi所在的midi轨道
		MidiTrack* midiTrack = nullptr;

		/// <summary>
	   /// 在音乐中我们一般用BPM来表述乐曲的速度，BPM(Beat per Minute)的意思是每分钟的拍子数。
	   /// 例如，BPM=100，表示该歌曲的速度是每分钟100拍。注意，对于音乐家来说，BPM中的一拍是指一个四分音符所发音的时间，
	   /// 而不管歌曲的拍号是多少.例如，假设歌曲的拍号是3/8拍(以八分音符为一拍，每小节3拍)，BPM=100，
	   /// 那么，音乐家依然会把歌曲的速度认为是以四分音符(非八分音符)为一拍，每分钟100拍。
	   /// 因此，BPM被称为是“音乐速度(Musical Tempo)”
	   /// </summary>
		float BPM = 120;

		//录制midi时一个四分音符所要弹奏的微秒数,默认值500000
		float microTempo = 500000;

		//录制midi时一个四分音符所要弹奏的tick数,默认值480
		float tickForQuarterNote = 480;
		//录制midi时每tick的毫秒数
		float msPerTick = 0;
		//录制midi时的基tick
		int baseTick = 0;
		//开始录制的起始时间点
		clock::time_point startRecordTime;
	};

}

#endif
