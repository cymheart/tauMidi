#ifdef _WIN32

#include"Audio_SDL.h"

namespace tau
{
	bool Audio_SDL::isInited = false;
	Audio_SDL::~Audio_SDL()
	{
		//SDL_Quit();
		Close();
	}
	void Audio_SDL::Close()
	{
		if (!isOpened)
			return;

		isOpened = false;
		SDL_CloseAudioDevice(deviceId);
	}

	void Audio_SDL::Open()
	{
		if (isOpened)
			return;

		if (!isInited)
		{
			if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
				return;
			}
			isInited = true;
		}

		SDL_AudioSpec wanted_spec;
		wanted_spec.freq = freq;
		wanted_spec.format = AUDIO_F32SYS;
		wanted_spec.channels = channelCount;
		wanted_spec.silence = 0;
		wanted_spec.samples = sampleCount;
		wanted_spec.callback = audioCallback;
		wanted_spec.userdata = data;

		//if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
		//     return;
		//SDL_PauseAudio(0);
		//
		if ((deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, nullptr, 0)) < 2) {
			return;
		}

		SDL_PauseAudioDevice(deviceId, 0);
		isOpened = true;
	}
}

#endif
