
#include"PcmRecorder.h"
#include <fstream>


namespace tau
{
	PcmRecorder::~PcmRecorder()
	{
		Clear();
	}

	void PcmRecorder::Clear()
	{
		for (int i = 0; i < pcmBlocks.size(); i++)
			DEL(pcmBlocks[i]);

		pcmBlocks.clear();
	}


	void PcmRecorder::Write(float* pcm, int size)
	{
		while (size > 0)
		{
			if (pos >= pcmBlockSize || pcmBlocks.empty())
			{
				pos = 0;
				float* newPcmBlock = new float[pcmBlockSize];
				pcmBlocks.push_back(newPcmBlock);
			}

			float* pcmBlock = pcmBlocks[pcmBlocks.size() - 1];
			int richSize = pcmBlockSize - pos;

			if (size <= richSize) {
				memcpy(pcmBlock + pos, pcm, size * sizeof(float));
				pos += size;
				size = 0;
			}
			else {
				memcpy(pcmBlock + pos, pcm, richSize * sizeof(float));
				size -= richSize;
				pos += richSize;
			}
		}
	}

	void PcmRecorder::SaveToFile(string& filePath)
	{
		ofstream fout(filePath, ios::binary);

		for (int i = 0; i < pcmBlocks.size(); i++)
		{
			float* pcmBlock = pcmBlocks[i];
			int size = pcmBlockSize;
			if (i == pcmBlocks.size() - 1)
				size = pos;
			fout.write((char*)(pcmBlock), sizeof(float) * size);
		}

		fout.close();

	}


	void PcmRecorder::SaveToWavFile(
		string& filePath,
		uint32_t sampleRate, //44100 etc..
		uint32_t numChannels, // 1 || 2
		bool samplesAreInts, //False if samples are 32 or 64-bit floating point values
		uint32_t byteDepth)  // 1, 2, 3, 4 for int samples; 4 or 8 for float samples
	{
		wavWriter.initialize(filePath.c_str(),
			sampleRate,
			numChannels,  // 1 || 2
			samplesAreInts, // false for 32/64-bit float vals
			byteDepth);  // 1, 2, 3, 4 for int samples; 4 or 8 for float samples
		wavWriter.startWriting(); //Writes header

		for (int i = 0; i < pcmBlocks.size(); i++)
		{
			float* pcmBlock = pcmBlocks[i];
			int size = pcmBlockSize;
			if (i == pcmBlocks.size() - 1)
				size = pos;

			wavWriter.writeData((uint8_t*)(pcmBlock), sizeof(float) * size);
		}

		wavWriter.finishWriting();
	}



	void PcmRecorder::SaveToMp3File(
		string& filePath,
		uint32_t sampleRate, //44100 etc..
		uint32_t numChannels // 1 || 2
	)
	{

		ofstream fout(filePath, ios::binary);
		mp3Recoder.Init(sampleRate, numChannels, sampleRate, 128, 0);

		int blockSampleCount = 1024 * 5;
		float* buffer_l = new float[blockSampleCount];
		float* buffer_r = new float[blockSampleCount];
		int mp3BufSize = (int)(7200 + blockSampleCount * 1.25);
		uint8_t* mp3Buf = new uint8_t[mp3BufSize];

		int n = 0;
		int len = 0;
		float* pcmBlock;
		for (int i = 0; i < pcmBlocks.size(); i++)
		{
			len = pcmBlockSize;
			if (i == pcmBlocks.size() - 1)
				len = pos;

			pcmBlock = pcmBlocks[i];
			for (int j = 0; j < len; j += 2)
			{
				buffer_l[n] = pcmBlock[j];
				buffer_r[n] = pcmBlock[j + 1];
				n++;

				if (n == blockSampleCount || j == len - 2) {
					int encodeSize = mp3Recoder.EncodeFloat(buffer_l, buffer_r, n, (uint8_t*)mp3Buf, mp3BufSize);
					fout.write((char*)(mp3Buf), encodeSize);
					n = 0;
				}
			}
		}

		//
		int encodeSize = mp3Recoder.flush((uint8_t*)mp3Buf, mp3BufSize);
		fout.write((char*)(mp3Buf), encodeSize);

		fout.close();
	}

}