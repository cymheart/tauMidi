﻿#ifndef _EditorTypes_h_
#define _EditorTypes_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"

namespace tau
{
	class MidiMarkerList;

	// Editor状态
	enum class EditorState
	{
		//停止
		STOP,
		//播放
		PLAY,
		//暂停
		PAUSE
	};

	struct InstFragmentToTrackInfo
	{
		InstFragment* instFragment;
		Track* track;
		int branchIdx;
		float sec;
	};

}

#endif