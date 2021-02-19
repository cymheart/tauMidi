
#ifndef _Audio_H
#define _Audio_H

#include"scutils/Utils.h"
using namespace scutils;

namespace ventrue
{
	class Audio;
	using AudioCallBack = void (*)(void* udata, uint8_t* stream, int len);

	class Audio {
	public:
		virtual ~Audio() { }

		void SetFreq(int freq)
		{
			this->freq = freq;
		}

		void SetChannelCount(int count)
		{
			if (count <= 1)count = 1;
			else if (count >= 2)count = 2;
			this->channelCount = count;
		}

		virtual int SetSampleCount(int sampleCount)
		{
			this->sampleCount = sampleCount;
			return this->sampleCount;
		}

		inline int GetSampleCount()
		{
			return sampleCount;
		}

		void SetAudioCallback(AudioCallBack audioCallback, void* data)
		{
			this->data = data;
			this->audioCallback = audioCallback;
		}

		virtual void Open() = 0;

	protected:
		AudioCallBack audioCallback = nullptr;
		int sampleCount = 1024;
		int channelCount = 2;
		int freq = 44100;
		void* data = nullptr;

	};
}
#endif //_Audio_H
