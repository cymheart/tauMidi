#ifndef _PcmRecorder_h_
#define _PcmRecorder_h_

#include "scutils/Utils.h"
#include"wav/WavWriter.hpp"
#include "scutils/Mp3Recorder.h"
using namespace scutils;

namespace tau
{
	enum class AudioFormat
	{
		PCM = 0,
		WAV,
		MP3,
	};

	class PcmRecorder
	{
	public:
		~PcmRecorder();

		void SetFrameSampleCount(int count)
		{
			frameSampleCount = count;
		}

		void Clear();

		void Write(float* pcm, int size);
		void SaveToFile(string& filePath);

		void SaveToWavFile(
			string& filePath,
			uint32_t sampleRate,
			uint32_t numChannels,
			bool samplesAreInts, //False if samples are 32 or 64-bit floating point values
			uint32_t byteDepth);


		void SaveToMp3File(
			string& filePath,
			uint32_t sampleRate,
			uint32_t numChannels);

	private:

		Mp3Recorder mp3Recoder;
		WavWriter wavWriter;

		vector<float*> pcmBlocks;
		int pos;
		int frameSampleCount = 256;

		//按双声道每秒钟44100个采样点，30秒时间长度空间
		int pcmBlockSize = 44100 * 2 * 30;
	};

}

#endif