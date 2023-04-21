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

		void OnKey(int key, float velocity, InstrumentSharp^ inst);


		void OffKey(int key, float velocity, InstrumentSharp^ inst);

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
		//获取当前播放时间点
		double GetPlaySec();
		//获取结束时间点
		double GetEndSec();
		// 设定播放速度
		void SetSpeed(float speed);

		void SetSoundFont(SoundFontSharp^ sf);

	private:

		Tau* tau;
	};
}