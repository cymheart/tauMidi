#ifndef _DsignalTypes_h_
#define _DsignalTypes_h_

namespace dsignal
{


	//Freqz采样区域信息
	struct FreqzSampleRegionInfo
	{
		//低频类型采样总数
		int lowFreqTypeSampleCount = 0;
		//低频采样起始标号
		int lowFreqStartSampleIdx = 0;
		//低频实际采样数量
		int lowFreqSampleCount = 0;
		//低频起始位置频率
		float lowFreqStartHz = 0;
		//低频结束位置频率
		float lowFreqEndHz = 0;
		//低频步进值
		float perLowFreqHz = 0;

		//
		//高频类型采样总数
		int highFreqTypeSampleCount = 0;
		//高频采样起始标号
		int highFreqStartSampleIdx = 0;
		//高频实际采样数量
		int highFreqSampleCount = 0;
		//高频起始位置频率
		float highFreqStartHz = 0;
		//高频结束位置频率
		float highFreqEndHz = 0;
		//高频步进值
		float perHighFreqHz = 0;

		//转换标号到频率点位
		float TransIdxToFreq(int idx)
		{
			float freq;
			if (idx < lowFreqSampleCount)
				freq = idx * perLowFreqHz + lowFreqStartHz;
			else
				freq = (idx - lowFreqSampleCount) * perHighFreqHz + highFreqStartHz;

			return freq;
		}
	};


	using BodeFreqzProcessCallBack = bool (*)(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data);

}

#endif
