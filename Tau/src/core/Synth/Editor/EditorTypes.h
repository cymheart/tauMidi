#ifndef _EditorTypes_h_
#define _EditorTypes_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"

namespace tau
{
	using EditorProcessCB = void (*)(Editor* editor);

	class MidiMarkerList;

	//播放模式
	enum class EditorPlayMode
	{
		//通常播放模式
		Common,

		//等待播放模式
		Wait,

		//步进播放模式
		//步进播放模式会启用Runto()函数控制播放时间，非步进模式是由midi固定时间内部控制
		Step,
	};

	// Editor状态
	enum class EditorState
	{
		//停止
		STOP,
		//播放
		PLAY,
		//暂停
		PAUSE,
		//结束暂停
		ENDPAUSE
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
