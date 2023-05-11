#ifndef _PcmRecorder_h_
#define _PcmRecorder_h_

#include "scutils/Utils.h"
#include"wav/WavWriter.hpp"
#include"TauEffect.h"
#include "scutils/Mp3Recorder.h"
using namespace scutils;

namespace tauFX
{
	enum class AudioFormat
	{
		PCM = 0,
		WAV,
		MP3,
	};

	/*合成流的实时录制*/
	class DLL_CLASS PcmRecorder : public TauEffect
	{
	public:
		PcmRecorder();
		~PcmRecorder();

		void Clear();

		//清空pcm录制缓存
		void ClearRecordPCM();
		//开始录制pcm
		void StartRecordPCM();
		//停止录制pcm
		void StopRecordPCM();
		//保存录制的pcm到文件
		void SaveRecordPCM(string& path);
		//保存录制的pcm到wav文件
		void SaveRecordPCMToWav(string& path, uint32_t sampleRate, uint32_t numChannels);
		//保存录制pcm的到mp3文件
		void SaveRecordPCMToMp3(string& path, uint32_t sampleRate, uint32_t numChannels);

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
		void EffectProcess(float* synthStream, int numChannels, int channelSampleCount);

	private:

		//pcm录制状态0:不录制, 1:录制中， 2：准备停止录制
		int pcmRecordState = 0;
		Semaphore pcmRecordWaitSem;


		Mp3Recorder mp3Recoder;
		WavWriter* wavWriter;

		vector<float*> pcmBlocks;
		int pos;

		//按双声道每秒钟44100个采样点，30秒时间长度空间
		int pcmBlockSize = 44100 * 2 * 30;
	};

}

#endif