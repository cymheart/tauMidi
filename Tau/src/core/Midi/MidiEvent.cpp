#include"MidiEvent.h"
namespace tau
{
	string KeySignatureEvent::majorName[] = {
		"C","G","D","A","E","B","#F","#C","F","bB","bE","bA","bD","bG","bC"
	};

	string KeySignatureEvent::majorFullName[] = {
		"C","G","D","A","E","B","F sharp","C sharp","F","B flat","E flat","A flat","D flat","G flat","C flat"
	};

	string KeySignatureEvent::minorName[] = {
		"A","E","B","#F","#C","#G","#D","#A","D","G","C","F","bB","bE","bA"
	};

	string KeySignatureEvent::minorFullName[] = {
		"A","E","B","F sharp","C sharp","G sharp","D sharp","A sharp","D","G","C","F","B flat","E flat","A flat"
	};

	NoteOnEvent::~NoteOnEvent()
	{
		childNoteOnEventCount = 0;
		if (childNoteOnEvents != nullptr)
			delete[] childNoteOnEvents;
	}

	void NoteOnEvent::AppendChildNoteOnEvents(NoteOnEvent** noteOnEvs, int count)
	{
		int oldCount = childNoteOnEventCount;
		childNoteOnEventCount += count;
		NoteOnEvent** newChildNoteOnEvents = new NoteOnEvent * [childNoteOnEventCount];

		int idx = 0;
		for (int i = 0; i < oldCount; i++)
			newChildNoteOnEvents[idx++] = childNoteOnEvents[i];

		for (int i = 0; i < count; i++)
			newChildNoteOnEvents[idx++] = noteOnEvs[i];

		DEL(childNoteOnEvents);
		childNoteOnEvents = newChildNoteOnEvents;
	}

	void NoteOnEvent::AppendChildNoteOnEvents(vector<NoteOnEvent*> notes)
	{
		int oldCount = childNoteOnEventCount;
		childNoteOnEventCount += notes.size();
		NoteOnEvent** newChildNoteOnEvents = new NoteOnEvent * [childNoteOnEventCount];

		int idx = 0;
		for (int i = 0; i < oldCount; i++)
			newChildNoteOnEvents[idx++] = childNoteOnEvents[i];

		for (int i = 0; i < notes.size(); i++)
			newChildNoteOnEvents[idx++] = notes[i];

		DEL(childNoteOnEvents);
		childNoteOnEvents = newChildNoteOnEvents;
	}


	SysexEvent::~SysexEvent()
	{
		free(data);
	}

	void SysexEvent::CreateData(byte* d, size_t len)
	{
		if (len <= 0)
			return;

		try
		{
			data = (byte*)malloc(len);
			if (data != nullptr)
				memcpy(data, d, len);
			this->size = len;
		}
		catch (exception)
		{

		}
	}

	UnknownEvent::~UnknownEvent()
	{
		free(data);
	}

	void UnknownEvent::CreateData(byte* d, size_t len)
	{
		if (len <= 0)
			return;

		try
		{
			data = (byte*)malloc(len);
			if (data != nullptr)
				memcpy(data, d, len);
			this->size = len;
		}
		catch (exception)
		{

		}
	}
}
