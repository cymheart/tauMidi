#ifndef _Audio_SDL_h_
#define _Audio_SDL_h_

#ifdef _WIN32

#include "Audio/Audio.h"
#include"SDL2/include/SDL.h"

namespace tau
{
	class Audio_SDL :public Audio
	{

	public:
		~Audio_SDL();
		virtual void Close();
		virtual void Open();

		virtual int SetSampleCount(int sampleCount)
		{
			if (sampleCount <= 64)
				sampleCount = 64;
			this->sampleCount = sampleCount;
			return this->sampleCount;
		}

	private:
		int deviceId = 0;
		static bool isInited;

	};
}

#endif
#endif
