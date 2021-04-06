#include"MidiTrack.h"
#include"MidiEvent.h"

namespace ventrue
{
	MidiTrack::MidiTrack()
	{
		midiEventList = new MidiEventList;
	}

	MidiTrack::MidiTrack(const MidiTrack& obj)
	{
		midiEventList = new MidiEventList;

		MidiEventList* cpyMidiEventList = obj.midiEventList;
		MidiEvent* midiEvent = nullptr;
		MidiEvent* cpyMidiEvent;
		for (int i = 0; i < cpyMidiEventList->size(); i++)
		{
			cpyMidiEvent = (*cpyMidiEventList)[i];

			switch (cpyMidiEvent->type)
			{
			case MidiEventType::NoteOn:
			{
				NoteOnEvent* noteOnEvent = new NoteOnEvent(*(NoteOnEvent*)cpyMidiEvent);
				noteOnEvent->noteOffEvent = nullptr;
				midiEvent = noteOnEvent;
			}
			break;

			case MidiEventType::NoteOff:
			{
				NoteOffEvent* noteOffEvent = new NoteOffEvent(*(NoteOffEvent*)cpyMidiEvent);
				NoteOnEvent* noteOnEvent = FindNoteOnEvent(noteOffEvent->note, noteOffEvent->channel);
				noteOnEvent->endTick = noteOffEvent->startTick;
				noteOnEvent->noteOffEvent = noteOffEvent;
				noteOffEvent->noteOnEvent = noteOnEvent;
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

			AddEvent(midiEvent);
		}
	}

	MidiTrack::~MidiTrack()
	{
		DEL_OBJS_VECTOR(midiEventList);
	}


	/// <summary>
	/// 增加一个事件
	/// </summary>
	/// <param name="ev"></param>
	void MidiTrack::AddEvent(MidiEvent* ev)
	{
		midiEventList->push_back(ev);

		//对noteon事件增加一个map索引，方便noteoff快速找到其对应noteon
		if (ev->type == MidiEventType::NoteOn)
		{
			NoteOnEvent* noteOnEvent = (NoteOnEvent*)ev;
			noteOnEventMap[noteOnEvent->note << 12 | noteOnEvent->channel].push_back(noteOnEvent);
		}
	}

	void MidiTrack::AddNoteOnEventToMap(int note, int channel, NoteOnEvent* midiEvent)
	{
		noteOnEventMap[note << 12 | channel].push_back(midiEvent);
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

	//变换tick
#define TransTick(orgTick) (dstBaseTick + (orgTick - orgBaseTick) * orgMsPerTick / dstMsPerTick);

//改变轨道事件中一个四分音符所要弹奏的tick数
	void MidiTrack::ChangeMidiEventsTickForQuarterNote(float changedTickForQuarterNote)
	{
		if (changedTickForQuarterNote == tickForQuarterNote)
			return;

		int orgBaseTick = 0;
		int dstBaseTick = 0;
		//未修改前的每tick的毫秒数
		float orgMsPerTick = 0;
		//修改后的每tick的毫秒数
		float dstMsPerTick = 0;

		for (int i = 0; i < midiEventList->size(); i++)
		{
			MidiEvent* midiEvent = (*midiEventList)[i];

			switch (midiEvent->type)
			{
			case MidiEventType::NoteOn:
			{
				NoteOnEvent* noteOnEvent = (NoteOnEvent*)midiEvent;
				noteOnEvent->startTick = TransTick(noteOnEvent->startTick);
			}
			break;

			case MidiEventType::NoteOff:
			{
				NoteOffEvent* noteOffEvent = (NoteOffEvent*)midiEvent;
				noteOffEvent->startTick = TransTick(noteOffEvent->startTick);
				noteOffEvent->noteOnEvent->endTick = noteOffEvent->startTick;
			}
			break;

			case MidiEventType::Tempo:
			{
				TempoEvent* tempoEvent = (TempoEvent*)midiEvent;
				float microTempo = tempoEvent->microTempo;
				orgBaseTick = tempoEvent->startTick;
				tempoEvent->startTick = TransTick(tempoEvent->startTick);
				dstBaseTick = tempoEvent->startTick;
				orgMsPerTick = microTempo / tickForQuarterNote;
				dstMsPerTick = microTempo / changedTickForQuarterNote;
			}
			break;

			default:
				midiEvent->startTick = TransTick(midiEvent->startTick);
				break;
			}
		}

		tickForQuarterNote = changedTickForQuarterNote;
	}


}
