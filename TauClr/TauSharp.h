#pragma once

#include<vcclr.h>
#include"SoundFontSharp.h"
#include"Synth/Tau.h"
#include"Types.h"
#include "InstrumentSharp.h"

using namespace System;
using namespace tau;
using namespace std;

namespace TauClr
{

	public ref class TauSharp
	{

	public:
		TauSharp();
		~TauSharp();

		InstrumentSharp^ EnableInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		void OnKey(int key, float velocity, InstrumentSharp^ inst, int delayMS);
		void OnKey(int key, float velocity, InstrumentSharp^ inst)
		{
			OnKey(key, velocity, inst, 0);
		}


		void OffKey(int key, float velocity, InstrumentSharp^ inst, int delayMS);
		void OffKey(int key, float velocity, InstrumentSharp^ inst)
		{
			OffKey(key, velocity, inst, 0);
		}

		void Open();

		bool IsLoadCompleted();
		void Load(String^ midiFilePath, bool isWaitLoadCompleted);
		void Load(String^ midiFilePath);
		void Play();
		void Stop();
		void Pause();
		void Close();
		void Goto(float sec);


		TauClr::EditorState GetEditorState();

		void SetSoundFont(SoundFontSharp^ sf);

	private:

		Tau* tau;
	};
}