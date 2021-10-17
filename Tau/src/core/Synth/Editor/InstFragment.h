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

		int GetStartTick()
		{
			return (int)startTick;
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

		void AddMidiEvents(list<MidiEvent*>& midiEvents_);

		const list<MidiEvent*>& GetMidiEvents()
		{
			return midiEvents;
		}

	private:

		uint32_t startTick = 0;
		double startSec = 0;
		double endSec = 0;

		Track* track = nullptr;
		int branchIdx = 0;

		//对应音轨事件队列的当前处理位置
		list<MidiEvent*>::iterator eventOffsetIter;

		list<MidiEvent*> midiEvents;

		friend class Track;
		friend class MidiEditor;
	};
}

#endif
