/*
* SoundFont2格式说明:
*
*

*/

#ifndef _SF2_h_
#define _SF2_h_

#include"SF2Chunks.h"
#include <iostream>
#include <fstream>

namespace tau
{
	class SF2
	{
	public:
		SF2(string path);
		~SF2();

	public:
		uint32_t size = 0;
		InfoListChunk* infoChunk = nullptr;
		SdtaListChunk* soundChunk = nullptr;
		PdtaListChunk* hydraChunk = nullptr;

	};
}

#endif
