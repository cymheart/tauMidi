#ifndef _InstFragment_h_
#define _InstFragment_h_

#include "Synth/TauTypes.h"


namespace tau
{
	// 乐器片段
	//by cymheart, 2021.
	class InstFragment
	{
	public:
		InstFragment();
		~InstFragment();

		void Clear();

		void SetStartTick(uint32_t tick)
		{
			startTick = tick;
		}

		void SetEndTick(uint32_t tick)
		{
			endTick = tick;
		}

		int GetStartTick()
		{
			return (int)startTick;
		}

		int GetEndTick()
		{
			return (int)endTick;
		}

		double GetStartSec()
		{
			return startSec;
		}

		double GetEndSec()
		{
			return endSec;
		}

		Track* GetTrack()
		{
			return track;
		}

		int GetBranchIdx()
		{
			return branchIdx;
		}

		void SetTrack(Track* track_, int branchIdx_ = 0)
		{
			track = track_;
			branchIdx = branchIdx_;
		}

		void AddMidiEvents(LinkedList<MidiEvent*>& midiEvents_);


		LinkedList<MidiEvent*>& GetMidiEvents()
		{
			return midiEvents;
		}

		void SetName(string& _name);

	private:
		string name;
		uint32_t startTick = 0;
		uint32_t endTick = 0;
		double startSec = 0;
		double endSec = 0;

		Track* track = nullptr;

		int branchIdx = 0;

		//对应音轨事件队列的当前处理位置
		LinkedListNode<MidiEvent*>* eventOffsetNode;
		LinkedList<MidiEvent*> midiEvents;


		friend class Track;
		friend class MidiEditor;
		friend class MeiExporter;
	};
}

#endif
