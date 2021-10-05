#include"MidiTrackRecord.h"

namespace tau
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

	//开始录制Midi,支持中途变速录制
	void MidiTrackRecord::Start()
	{
		if (isRecord)
			return;

		Clear();
		midiTrack = new MidiTrack();
		startRecordTime = clock::now();
		isRecord = true;
	}


	//停止录制
	void MidiTrackRecord::Stop()
	{
		isRecord = false;
	}

	// 获取录制的midi轨道
	MidiTrack* MidiTrackRecord::TakeMidiTrack(vector<RecordTempo>& tempos)
	{
		return TakeMidiTrack(defTickForQuarterNote, tempos);
	}

	// 获取录制的midi轨道
	//baseTickForQuarterNote: 轨道的tickForQuarterNote
	MidiTrack* MidiTrackRecord::TakeMidiTrack(float tickForQuarterNote, vector<RecordTempo>& tempos)
	{
		if (midiTrack == nullptr)
			return nullptr;

		MidiTrack* cpyMidiTrack = new MidiTrack(*midiTrack);
		cpyMidiTrack->CreateMidiEvents(*midiTrack, tickForQuarterNote, tempos);

		return cpyMidiTrack;
	}


	//获取录制时间点(单位:s)
	float MidiTrackRecord::GetRecordSec()
	{
		return (chrono::duration_cast<res>(clock::now() - startRecordTime).count() * 0.001f);
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
		noteOnEvent->startSec = GetRecordSec();
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
		noteOffEvent->startSec = GetRecordSec();

		NoteOnEvent* noteOnEvent = midiTrack->FindNoteOnEvent(key, channel);
		noteOnEvent->endSec = noteOffEvent->startSec;
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
		programEvent->startSec = GetRecordSec();
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
		ctrlEvent->startSec = GetRecordSec();
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
		pitchBendEvent->startSec = GetRecordSec();
		pitchBendEvent->value = value;
		pitchBendEvent->channel = channel;
		midiTrack->AddEvent(pitchBendEvent);
	}

	//录制设置PolyPressure
	void MidiTrackRecord::RecordSetPolyPressure(int key, int pressure, int channel)
	{
		if (!isRecord)
			return;

		KeyPressureEvent* keyPressureEvent = new KeyPressureEvent();
		keyPressureEvent->startSec = GetRecordSec();
		keyPressureEvent->note = key;
		keyPressureEvent->value = pressure;
		keyPressureEvent->channel = channel;
		midiTrack->AddEvent(keyPressureEvent);
	}
}
