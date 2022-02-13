#ifndef _MidiTrackRecord_h_
#define _MidiTrackRecord_h_

#include"TauTypes.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include<thread>

namespace tau
{
	struct RecordTempo
	{
		//BPM(Beat per Minute)的意思是每分钟的拍子数。
		float BPM = 120;
		//开始时间点
		float sec = 0;
	};

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
		MidiTrack* TakeMidiTrack(vector<RecordTempo>& tempos);

		//获取录制的midi轨道
		//tickForQuarterNote: 轨道的tickForQuarterNote
		MidiTrack* TakeMidiTrack(float tickForQuarterNote, vector<RecordTempo>& tempos);


		//获取录制时间点(单位:s)
		float GetRecordSec();

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

		//录制设置PolyPressure
		void RecordSetPolyPressure(int key, int pressure, int channel);

	private:
		//是否录制Midi
		bool isRecord = false;

		//录制midi所在的midi轨道
		MidiTrack* midiTrack = nullptr;

		//录制midi时一个四分音符所要弹奏的tick数,默认值480
		float defTickForQuarterNote = 480;

		//开始录制的起始时间点
		chrono::high_resolution_clock::time_point startRecordTime;
	};

}

#endif
