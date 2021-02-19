#ifdef _WIN32

#include"Audio_SDL.h"

namespace ventrue
{
	Audio_SDL::~Audio_SDL()
	{
		SDL_Quit();
	}


	void Audio_SDL::Open()
	{
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
			return;
		}

		SDL_AudioSpec wanted_spec;
		wanted_spec.freq = freq;
		wanted_spec.format = AUDIO_F32SYS;
		wanted_spec.channels = channelCount;
		wanted_spec.silence = 0;
		wanted_spec.samples = sampleCount;
		wanted_spec.callback = audioCallback;
		wanted_spec.userdata = data;

		if (SDL_OpenAudio(&wanted_spec, NULL) < 0) {
			return;
		}

		SDL_PauseAudio(0);
	}
}

#endif
