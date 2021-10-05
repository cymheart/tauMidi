#include"MidiMarkerList.h"
#include"Midi/MidiEvent.h"

namespace tau
{
	MidiMarkerList::MidiMarkerList()
	{

	}

	MidiMarkerList::~MidiMarkerList()
	{
		for (int i = 0; i < midiMarkers.size(); i++)
			DEL(midiMarkers[i]);
		midiMarkers.clear();
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
			return nullptr;

		return &(midiMarkers[prevIdx]->tempo);
	}

	//计算速度
	void MidiMarkerList::ComputeTempo()
	{
		lastGetTempoIdx = 0;

		//
		int prevIdx = -1;
		double startSec;
		MidiMarker* midiMarker;
		for (int i = 0; i < midiMarkers.size(); i++)
		{
			midiMarker = midiMarkers[i];
			if (!midiMarker->isEnableTempo)
				continue;

			if (prevIdx < 0)
				startSec = 0;
			else
				startSec = midiMarkers[prevIdx]->tempo.GetTickSec(midiMarker->GetStartTick());

			midiMarker->SetStartSec(startSec);
			midiMarker->ComputeTempo();
			prevIdx = i;
		}
	}

	void MidiMarkerList::AppendMidiMarker(MidiMarker* midiMarker)
	{
		midiMarkers.push_back(midiMarker);
		sort(midiMarkers.begin(), midiMarkers.end(), MidiMarkerTickCompare);
		ComputeTempo();
	}

	void MidiMarkerList::AppendFormMidiEvents(list<MidiEvent*>& midiEvents)
	{
		list<MidiEvent*>::iterator it = midiEvents.begin();
		list<MidiEvent*>::iterator end = midiEvents.end();

		MidiEvent* ev;
		MidiMarker* midiMarker;
		TempoEvent* tempoEvent;
		TimeSignatureEvent* timeSignEvent;
		KeySignatureEvent* keySignEvent;

		for (; it != end; it++)
		{
			ev = *it;

			if (ev->type != MidiEventType::Tempo &&
				ev->type != MidiEventType::TimeSignature &&
				ev->type != MidiEventType::KeySignature)
			{
				continue;
			}

			//
			midiMarker = new MidiMarker();
			midiMarker->SetStartTick(ev->startTick);

			if (ev->type == MidiEventType::Tempo)
			{
				tempoEvent = (TempoEvent*)ev;
				midiMarker->SetTempo(tempoEvent->microTempo, tickForQuarterNote, true);
			}
			else if (ev->type == MidiEventType::TimeSignature)
			{
				timeSignEvent = (TimeSignatureEvent*)ev;
				midiMarker->SetTimeSignature(timeSignEvent->denominator, timeSignEvent->numerator, true);
			}
			else if (ev->type == MidiEventType::KeySignature)
			{
				keySignEvent = (KeySignatureEvent*)ev;
				midiMarker->SetKeySignature(keySignEvent->sf, keySignEvent->mi, true);
			}

			midiMarkers.push_back(midiMarker);
		}

		sort(midiMarkers.begin(), midiMarkers.end(), MidiMarkerTickCompare);
		ComputeTempo();
	}

	bool MidiMarkerList::MidiMarkerTickCompare(MidiMarker* a, MidiMarker* b)
	{
		return a->GetStartTick() < b->GetStartTick();//升序
	}
}
