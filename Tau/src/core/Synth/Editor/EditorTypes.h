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

		//静音模式
		Mute,
	};

	//播放状态
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

	//缓存状态
	enum class CacheState
	{
		CacheStop,
		//停止中
		CacheStoping,

		//缓存并读取中
		CachingAndRead,
		//缓存中但不读取
		CachingNotRead,
		//仅读取
		OnlyRead,

		//缓存中但暂停
		CachingPauseRead,
		//暂停等待读取
		PauseWaitRead,

		CacheReadTail,

		//进入步进播放模式
		EnterStep,

		//离开步进播放模式
		LeaveStep,

		Remove,
		Removing
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

	struct LateNoteInfo
	{
		float sec;
		int note;
		Track* track;
		float vel;
	};

}

#endif
