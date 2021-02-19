#ifndef _SoundFontParser_h_
#define _SoundFontParser_h_

#include"VentrueTypes.h"

namespace ventrue
{
	//soundfont解析器
	//by cymheart, 2020--2021.
	class SoundFontParser
	{
	public:
		SoundFontParser(Ventrue* ventrue)
		{
			this->ventrue = ventrue;
		}

		virtual ~SoundFontParser()
		{

		}

		virtual	void Parse(string path) = 0;

	protected:
		Ventrue* ventrue;
	};

}

#endif
