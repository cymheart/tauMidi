#ifndef _EditorTypes_h_
#define _EditorTypes_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"

namespace tau
{
	using EditorProcessCB = void (*)(Editor* editor);

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

	//instFragMode
	enum class SelectInstFragMode
	{
		SingleSelect,
		MultiSelect,
	};

	struct InstFragmentToTrackInfo
	{
		InstFragment* instFragment;
		Track* track;
		int branchIdx;
		float sec;
	};

	struct SelectInstFragmentInfo
	{
		int trackIdx;
		int branchIdx;
		int instFragmentIdx;
	};

}

#endif
