#ifndef _Audio_Rt_h_
#define _Audio_Rt_h_

#ifdef _WIN32

#include "Audio/Audio.h"
#include"rtaudio/RtAudio.h"

namespace tau
{
	class Audio_Rt :public Audio
	{

	public:
		~Audio_Rt();
		virtual void Close();
		virtual void Open();

		virtual int SetSampleCount(int sampleCount)
		{
			if (sampleCount <= 256)
				sampleCount = 256;
			this->sampleCount = sampleCount;
			return this->sampleCount;
		}

	private:

		static int output(void* outputBuffer, void* /*inputBuffer*/, unsigned int nBufferFrames,
			double /*streamTime*/, RtAudioStreamStatus /*status*/, void* data);

	private:
		RtAudio* dac;
	};
}

#endif

#endif
