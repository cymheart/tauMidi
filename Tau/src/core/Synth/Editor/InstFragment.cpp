#include"InstFragment.h"
#include"Midi/MidiEvent.h"

namespace tau
{
	InstFragment::InstFragment()
	{

	}

	InstFragment::~InstFragment()
	{
		LinkedListNode<MidiEvent*>* node = midiEvents.GetHeadNode();
		for (; node; node = node->next)
		{
			DEL(node->elem);
		}

		midiEvents.Release();
	}

	void InstFragment::Clear()
	{
		eventOffsetNode = midiEvents.GetHeadNode();
	}

	void InstFragment::AddMidiEvents(LinkedList<MidiEvent*>& midiEvents_)
	{
		midiEvents.Merge(midiEvents_);
	}


}
