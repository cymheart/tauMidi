#ifndef _SF3Parser_h_
#define _SF3Parser_h_

#include"SoundFontFormat/SF2/SF2Parser.h"
#include"Synth/TauTypes.h"
#include"Synth/SoundFontParser.h"

namespace tau
{
	class SF3Parser :public SF2Parser
	{
	public:
		SF3Parser(SoundFont* sf)
			:SF2Parser(sf)
		{
		}

		//½âÎö
		void Parse(string filePath);

	protected:
		SF2* CreateSF2(string& filePath);
	};

}

#endif