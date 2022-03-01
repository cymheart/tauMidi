#include"MidiTrack.h"
#include <Synth/MidiTrackRecord.h>

namespace tau
{
	MidiTrack::MidiTrack()
	= default;

	MidiTrack::~MidiTrack()
	{
		LinkedListNode<MidiEvent*>* node = midiEventList.GetHeadNode();
		for (; node; node = node->next)
			DEL(node->elem);
		midiEventList.Release();

		//
		node = midiGolbalEventList.GetHeadNode();
		for (; node; node = node->next)
			DEL(node->elem);
		midiGolbalEventList.Release();


		//
		defaultProgramChangeEvent = nullptr;

		//
		noteOnEventMap.clear();
	}

	//清空midiTrack列表,但并不真正删除列表中的事件
	void MidiTrack::Clear()
	{
		midiEventList.Release();
		midiGolbalEventList.Release();
		noteOnEventMap.clear();

		for (auto & n : midiEventListAtChannel)
			n.Release();
	}

	//根据给定时间点获取tick的数量
#define GetTickCount(sec) (int)(baseTick + (uint32_t)(((sec) - baseTickSec) * 1000 / msPerTick));

	void MidiTrack::CreateTempoEvents(float tickForQuarterNote, vector<RecordTempo>& tempos)
	{
		int tempoIdx = GetNextTempoIndex(tempos, -1);
		int baseTick = 0;
		float baseTickSec = 0;
		// 一个四分音符的微秒数
		float microTempo = 60000000.0f / tempos[tempoIdx].BPM; //60000000: 1分钟的微秒数
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
			microTempo = 60000000.0f / tempos[tempoIdx].BPM;   //60000000: 1分钟的微秒数
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
		float microTempo = 60000000.0f / tempos[tempoIdx].BPM; //60000000: 1分钟的微秒数
		//录制midi时每tick的毫秒数	
		float msPerTick = microTempo / tickForQuarterNote * 0.001f;
		tempoIdx = GetNextTempoIndex(tempos, tempoIdx);

		//
		LinkedList<MidiEvent*>& cpyMidiEventList = ((MidiTrack&)orgMidiTrack).midiEventList;
		MidiEvent* midiEvent = nullptr;
		MidiEvent* cpyMidiEvent;

		//
		LinkedListNode<MidiEvent*>* node = cpyMidiEventList.GetHeadNode();
		for (; node; node = node->next)
		{
			cpyMidiEvent = node->elem;

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

			if (midiEvent == nullptr)
				continue;

			midiEvent->startTick = GetTickCount(midiEvent->startSec);

			if (tempoIdx >= 0 && midiEvent->startSec >= tempos[tempoIdx].sec)
			{
				microTempo = 60000000.0f / tempos[tempoIdx].BPM;   //60000000: 1分钟的微秒数
				msPerTick = microTempo / tickForQuarterNote * 0.001f;
				baseTickSec = midiEvent->startSec;
				baseTick = midiEvent->startTick;
			}

			AddEvent(midiEvent);
		}
	}

	void MidiTrack::SetMidiEventsChannel(int channel)
	{
		LinkedListNode<MidiEvent*>* node = midiEventList.GetHeadNode();
		for (; node; node = node->next)
		{
			MidiEvent* midiEvent = node->elem;
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
	void MidiTrack::AppendMidiEvents(LinkedList<MidiEvent*>& midiEvents)
	{
		midiEventList.Merge(midiEvents);
	}


	//寻找默认乐器改变事件
	void MidiTrack::FindDefaultProgramChangeEvent()
	{
		LinkedListNode<MidiEvent*>* node = midiEventList.GetHeadNode();
		for (; node; node = node->next)
		{
			MidiEvent* midiEvent = node->elem;
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
	LinkedListNode<MidiEvent*>* MidiTrack::AddEvent(MidiEvent* ev)
	{
		if (ev->channel >= 0)
		{
			LinkedListNode<MidiEvent*>* node = midiEventListAtChannel[ev->channel].AddLast(ev);

			//对noteon事件增加一个map索引，方便noteoff快速找到其对应noteon
			if (ev->type == MidiEventType::NoteOn)
			{
				NoteOnEvent* noteOnEvent = (NoteOnEvent*)ev;
				noteOnEventMap[noteOnEvent->note << 12 | noteOnEvent->channel].push_back(node);
			}

			return node;
		}
		else
		{
			if (ev->type == MidiEventType::Tempo ||
				ev->type == MidiEventType::TimeSignature ||
				ev->type == MidiEventType::KeySignature)
			{
				midiGolbalEventList.AddLast(ev);
			}
			else
			{
				midiEventList.AddLast(ev);
			}
		}

		return nullptr;
	}

	/// <summary>
	/// 移除一个事件
	/// </summary>
	/// <param name="ev"></param>
	void MidiTrack::RemoveEvent(LinkedListNode<MidiEvent*>* midiEventNode)
	{
		if (midiEventNode == nullptr || midiEventNode->elem == nullptr)
			return;

		int channel = midiEventNode->elem->channel;
		midiEventListAtChannel[channel].Remove(midiEventNode);
		delete midiEventNode->elem;
		delete midiEventNode;
	}

	/// <summary>
	/// 是否需要按键事件
	/// </summary>
	bool MidiTrack::IsNeedNoteOnEvents(int channel, uint32_t startTick, int keepCount)
	{
		if (keepCount < 0)
			return true;

		int count = 0;
		LinkedListNode<MidiEvent*>* startNode = nullptr;
		LinkedListNode<MidiEvent*>* node = midiEventListAtChannel[channel].GetLastNode();
		for (; node; node = node->prev)
		{
			if (node->elem->type != MidiEventType::NoteOn)
				continue;

			if (node->elem->startTick == startTick)
			{
				startNode = node;
				count++;
			}
			else
				break;
		}

		if (count > keepCount)
			return false;

		return true;
	}


	/// <summary>
	/// 移除相同StartTick的midi事件
	/// </summary>
	/// <param name="ev"></param>
	void MidiTrack::RemoveSameStartTickEvents(LinkedListNode<MidiEvent*>* midiEventNode, int keepCount)
	{
		if (midiEventNode == nullptr || midiEventNode->elem == nullptr)
			return;

		int count = 0;
		LinkedListNode<MidiEvent*>* startNode = nullptr;
		LinkedListNode<MidiEvent*>* node = midiEventNode;
		for (; node; node = node->prev)
		{
			if (node->elem->type != MidiEventType::NoteOn)
				continue;

			if (node->elem->startTick == midiEventNode->elem->startTick)
			{
				startNode = node;
				count++;
			}
			else
				break;
		}

		if (count > keepCount)
		{
			return;


			/*	midiEventListAtChannel[midiEventNode->elem->channel].Remove(midiEventNode);
				delete midiEventNode->elem;
				delete midiEventNode;*/


				/*	LinkedListNode<MidiEvent*>* next = nullptr;
					int n = 0;
					for (node = startNode; node; node = next)
					{
						next = node->next;

						n++;
						if (n > keepCount)
						{
							midiEventListAtChannel[node->elem->channel].Remove(node);
							delete node->elem;
							delete node;
						}

					}*/
		}
	}


	/// <summary>
	/// 寻找匹配的NoteOnEvent
	/// </summary>
	/// <param name="note"></param>
	/// <param name="channel"></param>
	/// <returns></returns>
	LinkedListNode<MidiEvent*>* MidiTrack::FindNoteOnEvent(int note, int channel)
	{
		LinkedListNode<MidiEvent*>* evNode;
		auto it = noteOnEventMap.find(note << 12 | channel);
		if (it != noteOnEventMap.end()) {
			evNode = it->second.back();
			it->second.pop_back();
			if (it->second.empty())
				noteOnEventMap.erase(note << 12 | channel);
			return evNode;
		}
		return nullptr;
	}

}
