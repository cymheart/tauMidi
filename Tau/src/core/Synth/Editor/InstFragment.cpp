#include"InstFragment.h"

namespace tau
{
	InstFragment::InstFragment()
	{

	}

	InstFragment::~InstFragment()
	{

	}

	void InstFragment::Clear()
	{
		isEnded = false;
		eventOffsetIter = midiEvents.begin();
	}

	void InstFragment::AddMidiEvents(list<MidiEvent*>& midiEvents_)
	{
		list<MidiEvent*>::iterator it = midiEvents_.begin();
		list<MidiEvent*>::iterator end = midiEvents_.end();

		for (; it != end; it++)
		{
			midiEvents.push_back(*it);
		}
	}


}
