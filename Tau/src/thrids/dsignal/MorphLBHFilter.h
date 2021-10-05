#ifndef _MorphLBHFilter_h_
#define _MorphLBHFilter_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include <dsignal/Biquad.h>
using namespace scutils;

namespace dsignal
{
	class MorphLBHFilter
	{
	public:
		MorphLBHFilter();

		//设置频率点
		void SetFreq(float freqHZ);

		//设置Q
		void SetQ(float q);

		//设置过渡位置0-1
		void SetAlpha(float a);

		//计算滤波参数
		void Compute();

		//获取滤波器
		vector<dsignal::Filter*> GetFilters();

		double Filtering(double input);

	private:
		bool LowPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo);
		bool BandPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo);
		bool HighPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo);

		static bool _LowPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data);
		static bool _BandPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data);
		static bool _HighPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data);

	private:

		Biquad lowPass;
		Biquad bandPass;
		Biquad highPass;


		float freq_ = 0;
		float q_ = 0.707f;
		float alpha[3] = { 0 };
		float a_ = 0;

	};
}

#endif
