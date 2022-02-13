
#include "SoundFontSharp.h"

namespace TauClr
{
	SoundFontSharp::SoundFontSharp()
	{
		sf = new SoundFont();
	}

	SoundFontSharp::~SoundFontSharp()
	{
		delete sf;
	}

	void SoundFontSharp::Parse(String^ formatName, String^ path)
	{
		string stdFormatName = marshal_as<string>(formatName->ToString());
		string stdPath = marshal_as<string>(path->ToString());
		sf->Parse(stdFormatName, stdPath);
	}

}