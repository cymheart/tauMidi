﻿#include"Filter.h"

namespace dsignal
{
	void Filter::Clear()
	{
		isEnable = true;
		num.clear();
		den.clear();
	}

	//设置系数(分子分母对半分)
	void Filter::SetCoefficient(double coeff[], int count)
	{
		num.clear();
		den.clear();

		int n = count / 2;
		for (int i = 0; i < n; i++) {
			num.push_back(coeff[i]);
			den.push_back(coeff[n + i]);
		}
	}

	//设置分子系数
	//分子系数: b[0], b[1], b[2] .... b[n]
	void Filter::SetNumCoefficient(double nums[], int len)
	{
		num.clear();
		for (int i = 0; i < len; i++)
			num.push_back(nums[i]);
	}

	//设置分母系数
	//分母系数: a[0], a[1], a[2] .... a[n]
	void Filter::SetDenCoefficient(double dens[], int len)
	{
		den.clear();
		for (int i = 0; i < len; i++)
			den.push_back(dens[i]);
	}

	/// <summary>
	///	sign = 0 时，x_out 为频率响应的实部， y_out 为频率响应的虚部
	///	sign = 1 时，x_out 为频率响应的模， y_out 为频率响应的幅角
	///	sign = 2 时，x_out 为以 dB 为单位的频率响应， y_out 为频率响应的幅角
	/// </summary>
	/// <param name="x_out"></param>
	/// <param name="y_out"></param>
	/// <param name="len">频率响应的取样点数</param>
	/// <param name="start">取样点开始位置</param>
	/// <param name="end">取样点结束位置</param>
	/// <param name="sign"></param>
	bool Filter::Freqz(double x_out[], double y_out[], int len, int start, int end, int sign)
	{
		if (!IsEnable() || num.size() == 0 || den.size() == 0)
			return false;

		Dsignal::Freqz(&num[0], &den[0], num.size() - 1, den.size() - 1, x_out, y_out, len, start, end, sign);
		return true;
	}


	bool Filter::FreqzBySampleRegionInfo(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, int sign)
	{
		bool isOK = false;

		if (regionInfo.lowFreqSampleCount > 0)
		{
			isOK = Freqz(
				x_out, y_out, regionInfo.lowFreqTypeSampleCount,
				regionInfo.lowFreqStartSampleIdx, regionInfo.lowFreqStartSampleIdx + regionInfo.lowFreqSampleCount - 1, 2);
		}

		if (regionInfo.highFreqSampleCount > 0)
		{
			isOK = Freqz(
				x_out + regionInfo.lowFreqSampleCount, y_out + regionInfo.lowFreqSampleCount, regionInfo.highFreqTypeSampleCount,
				regionInfo.highFreqStartSampleIdx, regionInfo.highFreqStartSampleIdx + regionInfo.highFreqSampleCount - 1, 2);
		}

		return isOK;
	}


	bool Filter::BodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo)
	{
		return bodeFreqz(x_out, y_out, regionInfo, bodeFreqzData);
	}
}
