#include"MidiMarkerList.h"
#include"Midi/MidiEvent.h"

namespace tau
{
	MidiMarkerList::MidiMarkerList()
	{

	}

	MidiMarkerList::~MidiMarkerList()
	{
		Clear();
	}

	void MidiMarkerList::Clear()
	{
		lastGetTempoIdx = 0;
		tickForQuarterNote = 0;

		for (int i = 0; i < midiMarkers.size(); i++)
			DEL(midiMarkers[i]);
		midiMarkers.clear();

	}


	void MidiMarkerList::Copy(MidiMarkerList& cpyMidiMarkerList)
	{
		vector<MidiMarker*>& cpyMidiMarkers = cpyMidiMarkerList.midiMarkers;

		MidiMarker* midiMarker;
		int j = 0;
		int jend = midiMarkers.size();
		for (int i = 0; i < cpyMidiMarkerList.midiMarkers.size(); i++)
		{
			if (j < jend) {
				midiMarkers[j]->Copy(*(cpyMidiMarkers[i]));
				j++;
			}
			else
			{
				midiMarker = new MidiMarker();
				midiMarker->Copy(*(cpyMidiMarkers[i]));
				midiMarkers.push_back(midiMarker);
				j++;
			}
		}

		for (int i = j; i < jend; i++) {
			DEL(midiMarkers[i]);
		}

		midiMarkers.erase(std::begin(midiMarkers) + j, std::end(midiMarkers));

		//
		lastGetTempoIdx = cpyMidiMarkerList.lastGetTempoIdx;
		tickForQuarterNote = cpyMidiMarkerList.tickForQuarterNote;
		defaultTempo.SetTempoByBPM(120, tickForQuarterNote, 0, 0);

	}

	//获取当前速度
	Tempo* MidiMarkerList::GetTempo(double sec)
	{
		if (sec < 0) { sec = 0; }
		if (sec < midiMarkers[lastGetTempoIdx]->GetStartSec())
			return GetTempo(-1, sec);

		return GetTempo(-1, sec, lastGetTempoIdx);
	}

	//获取当前速度
	Tempo* MidiMarkerList::GetTempo(int tick)
	{
		if (tick < 0) { tick = 0; }
		if (tick < midiMarkers[lastGetTempoIdx]->GetStartTick())
			return GetTempo(tick, -1);

		return GetTempo(tick, -1, lastGetTempoIdx);
	}


	//获取当前速度
	//tick >= 0 时使用tick
	//sec >=0 时使用sec判断
	Tempo* MidiMarkerList::GetTempo(int tick, double sec, int startIdx)
	{
		int prevIdx = -1;
		MidiMarker* midiMarker;
		int i = startIdx;
		for (; i < midiMarkers.size(); i++)
		{
			midiMarker = midiMarkers[i];
			if (!midiMarker->isEnableTempo)
				continue;

			if ((tick >= 0 && tick >= midiMarker->GetStartTick()) ||
				sec >= midiMarker->GetStartSec())
			{
				prevIdx = i;
				continue;
			}

			break;
		}

		lastGetTempoIdx = max(prevIdx, 0);

		if (i == 0 || prevIdx == -1)
			return &defaultTempo;

		return &(midiMarkers[prevIdx]->tempo);
	}

	//计算速度和起始时间
	void MidiMarkerList::ComputeTempoAndStartSec()
	{
		lastGetTempoIdx = 0;

		double startSec;
		MidiMarker* midiMarker;
		Tempo* tempo = nullptr;

		for (int i = 0; i < midiMarkers.size(); i++)
		{
			midiMarker = midiMarkers[i];
			if (tempo == nullptr)
				startSec = 0;
			else
				startSec = tempo->GetTickSec(midiMarker->GetStartTick());

			midiMarker->SetStartSec(startSec);

			if (midiMarker->isEnableTempo) {
				midiMarker->ComputeTempo();
				tempo = &(midiMarker->tempo);
			}
		}
	}

	void MidiMarkerList::AppendMidiMarker(MidiMarker* midiMarker)
	{
		midiMarkers.push_back(midiMarker);
		sort(midiMarkers.begin(), midiMarkers.end(), MidiMarkerTickCompare);
		ComputeTempoAndStartSec();
	}

	//从midiEvents中发现并添加MidiMarker标记
	void MidiMarkerList::AppendFromMidiEvents(LinkedList<MidiEvent*>& midiEvents)
	{
		LinkedListNode<MidiEvent*>* node = midiEvents.GetHeadNode();

		MidiEvent* ev;
		MidiMarker* midiMarker;
		TempoEvent* tempoEvent;
		TimeSignatureEvent* timeSignEvent;
		KeySignatureEvent* keySignEvent;

		for (; node; node = node->next)
		{
			ev = node->elem;

			if (ev->type != MidiEventType::Tempo &&
				ev->type != MidiEventType::TimeSignature &&
				ev->type != MidiEventType::KeySignature &&
				ev->type != MidiEventType::Text)
			{
				continue;
			}

			if (ev->type == MidiEventType::Text)
			{
				TextEvent* textEvent = (TextEvent*)ev;
				if (textEvent->textType != MidiTextType::Marker)
				{
					continue;
				}
			}

			//
			midiMarker = new MidiMarker();
			midiMarker->SetStartTick(ev->startTick);

			switch (ev->type)
			{
			case MidiEventType::Tempo:
				tempoEvent = (TempoEvent*)ev;
				midiMarker->SetTempo(tempoEvent->microTempo, tickForQuarterNote, true);
				break;

			case MidiEventType::TimeSignature:
				timeSignEvent = (TimeSignatureEvent*)ev;
				midiMarker->SetTimeSignature(timeSignEvent->numerator, timeSignEvent->denominator, true);
				break;

			case MidiEventType::KeySignature:
				keySignEvent = (KeySignatureEvent*)ev;
				midiMarker->SetKeySignature(keySignEvent->sf, keySignEvent->mi, true);
				midiMarker->track = keySignEvent->track;
				break;

			case MidiEventType::Text:
			{
				TextEvent* textEvent = (TextEvent*)ev;
				if (textEvent->textType == MidiTextType::Marker)
				{
					midiMarker->SetMarkerText(textEvent->text);
				}
			}
			break;


			default:
				break;
			}

			midiMarkers.push_back(midiMarker);
		}

		sort(midiMarkers.begin(), midiMarkers.end(), MidiMarkerTickCompare);
		ComputeTempoAndStartSec();
	}

	bool MidiMarkerList::MidiMarkerTickCompare(MidiMarker* a, MidiMarker* b)
	{
		return a->GetStartTick() < b->GetStartTick();//升序
	}

}
