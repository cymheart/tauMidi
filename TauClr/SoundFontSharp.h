#pragma once


#include<vcclr.h>
#include<Synth/SoundFont.h>
#include"Types.h"

using namespace System;
using namespace tau;

namespace TauClr
{

	public ref class SoundFontSharp
	{

	public:
		SoundFontSharp();
		~SoundFontSharp();

		void Parse(String^ formatName, String^ path);

		SoundFont* GetSoundFont()
		{
			return sf;
		}

	private:

		SoundFont* sf;

	};
}