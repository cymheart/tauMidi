﻿#ifndef _Track_h_
#define _Track_h_

#include "Synth/TauTypes.h"
#include "InstFragment.h"

namespace tau
{

	// 演奏音轨
	//by cymheart, 2020--2021.
	class Track
	{
	public:
		Track(MidiEditor* midiEditor);
		~Track();

		inline MidiEditor* GetMidiEditor()
		{
			return midiEditor;
		}

		inline Channel* GetChannel()
		{
			return channel;
		}

		void SetChannelNum(int channelNum);
		int GetChannelNum();

		void Clear();

		//新建分径
		void NewBranch();
		int GetBranchCount();

		void AddInstFragment(InstFragment* instFragment, int branchIdx = 0);
		void RemoveInstFragment(InstFragment* instFragment);

	public:

		MidiEditor* midiEditor;

		//是否播放结束
		bool isEnded = false;

		//是否禁止播放
		bool isDisablePlay = false;

		// 通道
		Channel* channel;

		vector<list<InstFragment*>*> instFragments;

		//结束时间点
		float endSec;
	};


}

#endif