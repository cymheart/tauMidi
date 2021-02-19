#include"MidiTrackRecord.h"

namespace ventrue
{
	MidiTrackRecord::MidiTrackRecord()
	{
	}

	MidiTrackRecord::~MidiTrackRecord()
	{
		Clear();
	}

	void MidiTrackRecord::Clear()
	{
		isRecord = false;
		DEL(midiTrack);
	}

	void MidiTrackRecord::SetBPM(float bpm)
	{
		BPM = bpm;
		microTempo = 60000000 / BPM;  //60000000: 1分钟的微秒数
	}

	void MidiTrackRecord::SetTickForQuarterNote(float tick)
	{
		tickForQuarterNote = tick;
	}

	//开始录制Midi,支持中途变速录制
	void MidiTrackRecord::Start()
	{
		if (isRecord)
			return;

		Clear();
		midiTrack = new MidiTrack();

		//
		clock::time_point curTime = clock::now();
		if (isRecord == false)
			startRecordTime = curTime;

		if (msPerTick != 0) {
			int tm = (int)(chrono::duration_cast<res>(curTime - startRecordTime).count() * 0.001f);
			baseTick = (int)(baseTick + tm / msPerTick);
		}

		startRecordTime = curTime;
		msPerTick = microTempo / tickForQuarterNote * 0.001f;
		isRecord = true;

		//
		RecordSetTempo(microTempo);
	}


	//停止录制
	void MidiTrackRecord::Stop()
	{
		isRecord = false;
	}

	// 获取录制的midi轨道
	MidiTrack* MidiTrackRecord::TakeMidiTrack()
	{
		return TakeMidiTrack(-1);
	}

	// 获取录制的midi轨道
	//baseTickForQuarterNote: 改变轨道的tickForQuarterNote
	MidiTrack* MidiTrackRecord::TakeMidiTrack(float baseTickForQuarterNote)
	{
		if (midiTrack == nullptr)
			return nullptr;

		MidiTrack* cpyMidiTrack = new MidiTrack(*midiTrack);

		if (baseTickForQuarterNote > 0)
			cpyMidiTrack->ChangeMidiEventsTickForQuarterNote(baseTickForQuarterNote);

		return cpyMidiTrack;
	}


	//变换当前录制时间点为tick数量
	int MidiTrackRecord::TransCurTimeToRecordMidiTick()
	{
		return TransToRecordMidiTick(clock::now());
	}

	//变换录制时间点(单位:ms)为tick数量
	int MidiTrackRecord::TransToRecordMidiTick(clock::time_point curtTime)
	{
		int tm = (int)(chrono::duration_cast<res>(curtTime - startRecordTime).count() * 0.001f);
		return (int)(baseTick + tm / msPerTick);
	}

	//录制SetTempo
	void MidiTrackRecord::RecordSetTempo(float microTempo)
	{
		if (!isRecord)
			return;

		TempoEvent* tempoEvent = new TempoEvent();
		tempoEvent->startTick = TransCurTimeToRecordMidiTick();
		tempoEvent->microTempo = microTempo;
		midiTrack->AddEvent(tempoEvent);
	}

	//录制OnKey
	void MidiTrackRecord::RecordOnKey(int key, float velocity, int channel)
	{
		if (!isRecord)
			return;

		NoteOnEvent* noteOnEvent = new NoteOnEvent();
		noteOnEvent->note = key;
		noteOnEvent->velocity = (int)velocity;
		noteOnEvent->channel = channel;
		noteOnEvent->startTick = TransCurTimeToRecordMidiTick();
		midiTrack->AddEvent(noteOnEvent);
	}

	//录制OffKey
	void MidiTrackRecord::RecordOffKey(int key, float velocity, int channel)
	{
		if (!isRecord)
			return;

		NoteOffEvent* noteOffEvent = new NoteOffEvent();
		noteOffEvent->note = key;
		noteOffEvent->velocity = (int)velocity;
		noteOffEvent->channel = channel;
		noteOffEvent->startTick = TransCurTimeToRecordMidiTick();

		NoteOnEvent* noteOnEvent = midiTrack->FindNoteOnEvent(key, channel);
		noteOnEvent->endTick = noteOffEvent->startTick;
		noteOnEvent->noteOffEvent = noteOffEvent;
		noteOffEvent->noteOnEvent = noteOnEvent;

		midiTrack->AddEvent(noteOffEvent);
	}

	//录制设置乐器
	void MidiTrackRecord::RecordSetProgramNum(int instNum, int channel)
	{
		if (!isRecord)
			return;

		ProgramChangeEvent* programEvent = new ProgramChangeEvent();
		programEvent->startTick = TransCurTimeToRecordMidiTick();
		programEvent->channel = channel;
		programEvent->value = instNum;
		midiTrack->AddEvent(programEvent);
	}

	//录制设置midi控制器
	void MidiTrackRecord::RecordSetController(MidiControllerType type, int value, int channel)
	{
		if (!isRecord)
			return;

		ControllerEvent* ctrlEvent = new ControllerEvent();
		ctrlEvent->startTick = TransCurTimeToRecordMidiTick();
		ctrlEvent->ctrlType = type;
		ctrlEvent->value = value;
		ctrlEvent->channel = channel;
		midiTrack->AddEvent(ctrlEvent);
	}

	//录制设置pitchBend
	void MidiTrackRecord::RecordSetPitchBend(int value, int channel)
	{
		if (!isRecord)
			return;

		PitchBendEvent* pitchBendEvent = new PitchBendEvent();
		pitchBendEvent->startTick = TransCurTimeToRecordMidiTick();
		pitchBendEvent->value = value;
		pitchBendEvent->channel = channel;
		midiTrack->AddEvent(pitchBendEvent);
	}
}
