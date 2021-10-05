#ifndef _SoundFontParser_h_
#define _SoundFontParser_h_

#include"TauTypes.h"

namespace tau
{
	//soundfont解析器
	//by cymheart, 2020--2021.
	class SoundFontParser
	{
	public:
		SoundFontParser(SoundFont* sf)
		{
			this->sf = sf;
		}

		virtual ~SoundFontParser()
		{

		}

		virtual	void Parse(string path) = 0;

	protected:
		SoundFont* sf;
	};

}

#endif
