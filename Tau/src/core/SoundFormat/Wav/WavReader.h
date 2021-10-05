#ifndef _WavReader_h_
#define _WavReader_h_

#include"scutils/Utils.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace scutils;


namespace tau
{
	class WavReader
	{

	public:
		~WavReader();

		// 获取声道个数        
		int GetChannelNum()
		{
			return num_Channels;
		}

		/// <summary>
		/// 获取采样率
		/// </summary>
		/// <returns></returns>
		int GetSamplesRate()
		{
			return SamplesPerSec;
		}

		/// <summary>
		/// 获取PCM数据
		/// </summary>
		/// <returns></returns>
		short* GetData()
		{
			return leftChannelData;
		}

		size_t GetDataSize()
		{
			if (BitsPerSample != 8)
				return dataSize / 2;
			return dataSize;
		}

		/// <summary>
		/// 获取左声道PCM数据
		/// </summary>
		/// <returns></returns>
		short* GetLeftChannelData()
		{
			return leftChannelData;
		}

		/// <summary>
		/// 获取右声道PCM数据
		/// </summary>
		/// <returns></returns>
		short* GetRightChannelData()
		{
			return rightChannelData;
		}

		/// <summary>
		/// 数字节数组转换为int
		/// </summary>
		/// <param name="bytArray"></param>
		/// <returns></returns>
		int bytArray2Int(uint8_t* bytArray)
		{
			return bytArray[0] | (bytArray[1] << 8) | (bytArray[2] << 16) | (bytArray[3] << 24);
		}

		void ReadWavFile(string filePath);

		void DeleteData();

	private:
		// RIFF WAVE Chunk    
		string Id; //文件标识
		size_t Size = 0;  //文件大小
		string Type; //文件类型

		// Format Chunk
		string formatId;
		int formatSize = 0;      //数值为16或18，18则最后又附加信息
		int formatTag = 0;
		int num_Channels = 0;       //声道数目，1--单声道；2--双声道
		int SamplesPerSec = 0;      //采样率
		int AvgBytesPerSec = 0;     //每秒所需字节数 
		int BlockAlign = 0;         //数据块对齐单位(每个采样需要的字节数) 
		int BitsPerSample = 0;      //每个采样需要的bit数
		string additionalInfo;  //附加信息（可选，通过Size来判断有无）
		/*
		 * 以'fmt'作为标示。一般情况下Size为16，此时最后附加信息没有；
		 * 如果为18则最后多了2个字节的附加信息。
		 * 主要由一些软件制成的wav格式中含有该2个字节的附加信息
		 */

		 //Data Chunk
		string dataId;
		size_t dataSize = 0;
		short* leftChannelData = nullptr;  //左单声道数据
		short* rightChannelData = nullptr;  //右单声道数据


	};

}

#endif
