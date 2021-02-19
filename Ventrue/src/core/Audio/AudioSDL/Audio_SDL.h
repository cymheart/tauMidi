#ifndef _Audio_SDL_h_
#define _Audio_SDL_h_

#ifdef _WIN32

#include "Audio/Audio.h"
namespace ventrue
{
	class Audio_SDL :public Audio
	{

	public:
		~Audio_SDL();
		virtual void Open();

		virtual int SetSampleCount(int sampleCount)
		{
			if (sampleCount <= 256)
				sampleCount = 256;
			this->sampleCount = sampleCount;
			return this->sampleCount;
		}
	};
}

#endif

#endif
