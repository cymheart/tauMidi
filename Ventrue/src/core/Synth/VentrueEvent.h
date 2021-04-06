#ifndef _VentrueEvent_h_
#define _VentrueEvent_h_

#include"VentrueTypes.h"
#include"Ventrue.h"

namespace ventrue
{
	enum class VentrueEventType
	{
		Unknown,
		OnKey,
		OffKey,
		Render,
		PlayMidi,
		PlayMidiIdx,
		PlayMidiFile,
		AppendMidiFile,
		DisablePlayMidiTrack,
		PlayMidiGoto,
		SetDeviceChannelMidiControllerValue,
		EnableInstrument,
		Text,
		RecordMidi,
		StopRecordMidi,
		CreateRecordMidiFileObject,
		SaveMidiFileToDisk,
	};

	class VentrueEvent : public Task
	{
	public:
		VentrueEvent()
		{
			isSysReleaseTask = false;
			releaseCallBack = Release;
		}

		Ventrue* ventrue = nullptr;
		VentrueEventType evType = VentrueEventType::OnKey;
		KeySounderID keySounderID = 0;
		VirInstrument* virInst = nullptr;
		void* ptr = nullptr;
		int key = -1;
		float velocity = 0;
		string midiFilePath;
		MidiFile* midiFile = nullptr;
		int midiFileIdx = -1;
		int midiTrackIdx = -1;
		int deviceChannelNum = 0;
		int bankSelectMSB = 0;
		int bankSelectLSB = 0;
		int instrumentNum = 0;
		MidiControllerType midiCtrlType = MidiControllerType::CC_None;
		int value = 0;
		float sec = 0;
		float bpm = 0;
		float tickForQuarterNote = 0;
		string text;
		float exValue[10] = { 0 };
		void* exPtr[10] = { nullptr };
		Semaphore* sem = nullptr;


	public:
		static VentrueEvent* New();
	private:
		static void Release(Task* task);
	};
}

#endif
