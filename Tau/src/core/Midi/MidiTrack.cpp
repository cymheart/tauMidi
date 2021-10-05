#include"MidiTrack.h"
#include <Synth/MidiTrackRecord.h>

namespace tau
{
	MidiTrack::MidiTrack()
	{

	}

	MidiTrack::~MidiTrack()
	{
		list<MidiEvent*>::iterator it = midiEventList.begin();
		list<MidiEvent*>::iterator end = midiEventList.end();
		for (; it != end; it++)
			DEL(*it);
		midiEventList.clear();

		//
		it = midiGolbalEventList.begin();
		end = midiGolbalEventList.end();
		for (; it != end; it++)
			DEL(*it);
		midiGolbalEventList.clear();

		//
		defaultProgramChangeEvent = nullptr;

		//
		noteOnEventMap.clear();
	}

	//清空midiTrack列表,但并不真正删除列表中的事件
	void MidiTrack::Clear()
	{
		midiEventList.clear();
		midiGolbalEventList.clear();
		noteOnEventMap.clear();

		for (int n = 0; n < 16; n++)
			midiEventListAtChannel[n].clear();
	}

	//根据给定时间点获取tick的数量
#define GetTickCount(sec) (int)(baseTick + (uint32_t)((sec - baseTickSec) * 1000 / msPerTick));

	void MidiTrack::CreateTempoEvents(float tickForQuarterNote, vector<RecordTempo>& tempos)
	{
		int tempoIdx = GetNextTempoIndex(tempos, -1);
		int baseTick = 0;
		float baseTickSec = 0;
		// 一个四分音符的微秒数
		float microTempo = 60000000 / tempos[tempoIdx].BPM; //60000000: 1分钟的微秒数
		//录制midi时每tick的毫秒数	
		float msPerTick = microTempo / tickForQuarterNote * 0.001f;


		for (int i = 0; i < tempos.size(); i++)
		{
			TempoEvent* tempoEvent = new TempoEvent();
			tempoEvent->startTick = GetTickCount(tempos[tempoIdx].sec);
			tempoEvent->microTempo = microTempo;
			tempoEvent->channel = -1;
			AddEvent(tempoEvent);

			//	
			microTempo = 60000000 / tempos[tempoIdx].BPM;   //60000000: 1分钟的微秒数
			msPerTick = microTempo / tickForQuarterNote * 0.001f;
			baseTickSec = tempos[tempoIdx].sec;
			baseTick = tempoEvent->startTick;
			tempoIdx = GetNextTempoIndex(tempos, tempoIdx);
		}
	}

	void MidiTrack::CreateMidiEvents(const MidiTrack& orgMidiTrack, float tickForQuarterNote, vector<RecordTempo>& tempos)
	{
		int tempoIdx = GetNextTempoIndex(tempos, -1);
		int baseTick = 0;
		float baseTickSec = 0;
		// 一个四分音符的微秒数
		float microTempo = 60000000 / tempos[tempoIdx].BPM; //60000000: 1分钟的微秒数
		//录制midi时每tick的毫秒数	
		float msPerTick = microTempo / tickForQuarterNote * 0.001f;
		tempoIdx = GetNextTempoIndex(tempos, tempoIdx);

		//
		list<MidiEvent*>& cpyMidiEventList = ((MidiTrack&)orgMidiTrack).midiEventList;
		MidiEvent* midiEvent = nullptr;
		MidiEvent* cpyMidiEvent;

		//
		list<MidiEvent*>::iterator it = cpyMidiEventList.begin();
		list<MidiEvent*>::iterator end = cpyMidiEventList.end();
		for (; it != end; it++)
		{
			cpyMidiEvent = *it;

			//
			switch (cpyMidiEvent->type)
			{
			case MidiEventType::NoteOn:
			{
				NoteOnEvent* noteOnEvent = new NoteOnEvent(*(NoteOnEvent*)cpyMidiEvent);
				noteOnEvent->noteOffEvent = nullptr;
				noteOnEvent->endTick = GetTickCount(noteOnEvent->endSec);
				midiEvent = noteOnEvent;
			}
			break;

			case MidiEventType::NoteOff:
			{
				NoteOffEvent* noteOffEvent = new NoteOffEvent(*(NoteOffEvent*)cpyMidiEvent);
				midiEvent = noteOffEvent;
			}
			break;

			case MidiEventType::Tempo:
				midiEvent = new TempoEvent(*(TempoEvent*)cpyMidiEvent);
				break;

			case MidiEventType::TimeSignature:
				midiEvent = new TimeSignatureEvent(*(TimeSignatureEvent*)cpyMidiEvent);
				break;

			case MidiEventType::KeySignature:
				midiEvent = new KeySignatureEvent(*(KeySignatureEvent*)cpyMidiEvent);
				break;

			case MidiEventType::Controller:
				midiEvent = new ControllerEvent(*(ControllerEvent*)cpyMidiEvent);
				break;

			case MidiEventType::ProgramChange:
				midiEvent = new ProgramChangeEvent(*(ProgramChangeEvent*)cpyMidiEvent);
				break;

			case MidiEventType::KeyPressure:
				midiEvent = new KeyPressureEvent(*(KeyPressureEvent*)cpyMidiEvent);
				break;

			case MidiEventType::ChannelPressure:
				midiEvent = new ChannelPressureEvent(*(ChannelPressureEvent*)cpyMidiEvent);
				break;

			case MidiEventType::PitchBend:
				midiEvent = new PitchBendEvent(*(PitchBendEvent*)cpyMidiEvent);
				break;

			case MidiEventType::Text:
				midiEvent = new TextEvent(*(TextEvent*)cpyMidiEvent);
				break;

			case MidiEventType::Sysex:
				midiEvent = new SysexEvent(*(SysexEvent*)cpyMidiEvent);
				break;

			case MidiEventType::Unknown:
				midiEvent = new UnknownEvent(*(UnknownEvent*)cpyMidiEvent);
				break;

			default:
				break;
			}

			midiEvent->startTick = GetTickCount(midiEvent->startSec);

			if (tempoIdx >= 0 && midiEvent->startSec >= tempos[tempoIdx].sec)
			{
				microTempo = 60000000 / tempos[tempoIdx].BPM;   //60000000: 1分钟的微秒数
				msPerTick = microTempo / tickForQuarterNote * 0.001f;
				baseTickSec = midiEvent->startSec;
				baseTick = midiEvent->startTick;
			}

			AddEvent(midiEvent);
		}
	}

	void MidiTrack::SetMidiEventsChannel(int channel)
	{
		list<MidiEvent*>::iterator it = midiEventList.begin();
		list<MidiEvent*>::iterator end = midiEventList.end();
		for (; it != end; it++)
		{
			MidiEvent* midiEvent = *it;
			midiEvent->channel = channel;
		}
	}

	//获取下一个TempoIndex
	int MidiTrack::GetNextTempoIndex(vector<RecordTempo>& tempos, int curtIndex)
	{
		int idx = -1;
		if (curtIndex + 1 < (int)(tempos.size()))
			idx = curtIndex + 1;
		else
			return idx;

		for (int i = idx + 1; i < (int)(tempos.size()); i++)
		{
			if (tempos[i].sec == tempos[idx].sec)
			{
				idx = i;
				continue;
			}
			break;
		}

		return idx;
	}

	//添加midi事件
	void MidiTrack::AppendMidiEvents(vector<MidiEvent*>& midiEvents)
	{
		for (int i = 0; i < midiEvents.size(); i++)
			midiEventList.push_back(midiEvents[i]);
	}

	//寻找默认乐器改变事件
	void MidiTrack::FindDefaultProgramChangeEvent()
	{
		list<MidiEvent*>::iterator it = midiEventList.begin();
		list<MidiEvent*>::iterator end = midiEventList.end();
		for (; it != end; it++)
		{
			MidiEvent* midiEvent = *it;
			if (midiEvent->type == MidiEventType::ProgramChange)
			{
				defaultProgramChangeEvent = (ProgramChangeEvent*)midiEvent;
				break;
			}
			else if (midiEvent->type == MidiEventType::NoteOn)
			{
				break;
			}
		}
	}

	/// <summary>
	/// 增加一个事件
	/// </summary>
	/// <param name="ev"></param>
	void MidiTrack::AddEvent(MidiEvent* ev)
	{
		if (ev->channel >= 0)
		{
			midiEventListAtChannel[ev->channel].push_back(ev);
		}
		else
		{
			if (ev->type == MidiEventType::Tempo ||
				ev->type == MidiEventType::TimeSignature ||
				ev->type == MidiEventType::KeySignature)
			{
				midiGolbalEventList.push_back(ev);
			}
			else
			{
				midiEventList.push_back(ev);
			}
		}

		//对noteon事件增加一个map索引，方便noteoff快速找到其对应noteon
		if (ev->type == MidiEventType::NoteOn)
		{
			NoteOnEvent* noteOnEvent = (NoteOnEvent*)ev;
			noteOnEventMap[noteOnEvent->note << 12 | noteOnEvent->channel].push_back(noteOnEvent);
		}
	}


	/// <summary>
	/// 寻找匹配的NoteOnEvent
	/// </summary>
	/// <param name="note"></param>
	/// <param name="channel"></param>
	/// <returns></returns>
	NoteOnEvent* MidiTrack::FindNoteOnEvent(int note, int channel)
	{
		NoteOnEvent* ev;
		auto it = noteOnEventMap.find(note << 12 | channel);
		if (it != noteOnEventMap.end()) {
			ev = it->second.back();
			it->second.pop_back();
			if (it->second.empty())
				noteOnEventMap.erase(note << 12 | channel);
			return ev;
		}
		return nullptr;
	}

}
