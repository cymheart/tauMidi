#ifndef _Audio_pa_h_
#define _Audio_pa_h_

#ifdef _WIN32

#include "Audio/Audio.h"
#include"portaudio/portaudio.h"
#include"portaudio/pa_win_ds.h"

namespace tau
{
	class Audio_pa :public Audio
	{

	public:
		virtual ~Audio_pa();
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
		static int patest1Callback(const void* inputBuffer, void* outputBuffer,
			unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void* userData);

	private:
		PaStream* stream = nullptr;
	};
}

#endif

#endif
