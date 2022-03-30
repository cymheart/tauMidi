#ifndef _RenderEvent_h_
#define _RenderEvent_h_

#include"TauTypes.h"
#include"Synther/Synther.h"
#include"TauPool.h"
#include"Midi/MidiTypes.h"

namespace tau
{
	class SyntherEvent : public Task
	{
	public:
		SyntherEvent()
		{
			isSysReleaseTask = false;
			releaseCallBack = Release;
		}

		Synther* synther = nullptr;
		VirInstrument* virInst = nullptr;
		void* ptr = nullptr;
		int key = -1;
		float velocity = 0;
		string midiFilePath;
		int deviceChannelNum = 0;
		int bankSelectMSB = 0;
		int bankSelectLSB = 0;
		int instrumentNum = 0;
		MidiControllerType midiCtrlType = MidiControllerType::CC_None;
		int value = 0;
		int iValue[10] = { 0 };
		string text;
		float exValue[10] = { 0 };
		void* exPtr[10] = { nullptr };
		Semaphore* sem = nullptr;


	public:
		static SyntherEvent* New();
	private:
		static void Release(Task* task);
	};
}

#endif
