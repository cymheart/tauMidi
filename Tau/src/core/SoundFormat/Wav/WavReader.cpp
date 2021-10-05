#include "WavReader.h"
#include "scutils\ByteStream.h"

namespace tau
{
	WavReader::~WavReader()
	{
		DeleteData();
	}

	void WavReader::DeleteData()
	{
		DEL_ARRAY(leftChannelData);
		DEL_ARRAY(rightChannelData);
	}

	// 读取波形文件并显示
	void WavReader::ReadWavFile(string filePath)
	{
		if (filePath == "")
			return;

		DEL_ARRAY(leftChannelData);
		DEL_ARRAY(rightChannelData);

		std::ifstream t;
		size_t length;
		char* buffer;

		try
		{
			t.open(filePath, ios::in | ios::binary);
			t.seekg(0, std::ios::end);
			length = t.tellg();
			t.seekg(0, std::ios::beg);
			buffer = new char[length];
			t.read(buffer, length);
		}
		catch (exception)
		{
			t.close();
			cout << filePath << "文件打开出错!" << endl;
			return;
		}


		ByteStream br(buffer, length);
		delete[] buffer;
		byte id[4];
		byte size[4];
		byte type[4];

		byte formatid[4];
		byte formatsize[4];
		byte formattag[2];
		byte numchannels[2];
		byte samplespersec[4];
		byte avgbytespersec[4];
		byte blockalign[2];
		byte bitspersample[2];
		byte additionalinfo[2];    //可选

		byte datasize[4];

		//RIFF WAVE Chunk
		br.read(id, 0, 4);
		br.read(size, 0, 4);
		br.read(type, 0, 4);

		Id.assign((const char*)id, 4);
		Size = bytArray2Int(size);//十六进制转为十进制
		Type.assign((const char*)type, 4);


		//Format Chunk
		br.read(formatid, 0, 4);
		br.read(formatsize, 0, 4);
		br.read(formattag, 0, 2);
		br.read(numchannels, 0, 2);
		br.read(samplespersec, 0, 4);
		br.read(avgbytespersec, 0, 4);
		br.read(blockalign, 0, 2);
		br.read(bitspersample, 0, 2);

		string formatStr((const char*)formatsize, 2);
		if (formatStr == "18")
		{
			br.read(additionalinfo, 0, 2);
			additionalInfo.assign((const char*)additionalinfo, 2);  //附加信息
		}

		formatId.assign((const char*)formatid, 4);
		formatSize = bytArray2Int(formatsize);

		byte fourByte[4] = { formattag[0], formattag[1] };
		formatTag = bytArray2Int(fourByte);

		fourByte[0] = numchannels[0]; fourByte[1] = numchannels[1];
		num_Channels = bytArray2Int(fourByte);                    //声道数目，1--单声道；2--双声道
		SamplesPerSec = bytArray2Int(samplespersec);            //采样率
		AvgBytesPerSec = bytArray2Int(avgbytespersec);          //每秒所需字节数   

		fourByte[0] = blockalign[0]; fourByte[1] = blockalign[1];
		BlockAlign = bytArray2Int(fourByte);                        //数据块对齐单位(每个采样需要的字节数)

		fourByte[0] = bitspersample[0]; fourByte[1] = bitspersample[1];
		BitsPerSample = bytArray2Int(fourByte);        // 每个采样需要的bit数     

		// Data Chunk  
		byte d_flag;

		while (true)
		{
			br.read(&d_flag, 0, 1);
			if (d_flag == 'd')
				break;
		}

		byte dt_id[4];
		dt_id[0] = d_flag;
		br.read(dt_id, 1, 3);
		dataId.assign((const char*)dt_id, 4);
		br.read(datasize, 0, 4);
		dataSize = bytArray2Int(datasize);


		if (BitsPerSample == 8)
		{
			leftChannelData = new short[dataSize];

			if (num_Channels == 2)
			{
				rightChannelData = new short[dataSize];

				for (int i = 0; i < dataSize; i += 2)
				{
					leftChannelData[i] = br.read<byte>();
					rightChannelData[i] = br.read<byte>();
				}
			}
			else
			{
				for (int i = 0; i < dataSize; i++)
				{
					leftChannelData[i] = br.read<byte>();
				}
			}
		}
		else if (BitsPerSample == 16)
		{
			leftChannelData = new short[dataSize / 2];

			if (num_Channels == 2)
			{
				rightChannelData = new short[dataSize / 2];

				for (int i = 0; i < dataSize / 2; i += 2)
				{
					leftChannelData[i] = br.read<short>();
					rightChannelData[i] = br.read<short>();
				}
			}
			else
			{
				for (int i = 0; i < dataSize / 2; i++)
				{
					leftChannelData[i] = br.read<short>();
				}
			}
		}
	}
}
