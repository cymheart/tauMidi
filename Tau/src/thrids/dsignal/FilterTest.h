#ifndef _FilterTest_h_
#define _FilterTest_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include <dsignal/Biquad.h>
using namespace scutils;

namespace dsignal
{
	class FilterTest
	{
	public:
		FilterTest();

		//设置最大偏移增益
		void SetMaxOffsetGainDB(float maxOffsetGain)
		{
			maxOffsetGain_ = abs(maxOffsetGain);
			maxOffsetGainlin = pow(10.0, -maxOffsetGain_ / 20);
		}

		//设置最大偏移增益
		float GetMaxOffsetGainDB()
		{
			return maxOffsetGain_;
		}

		//设置频率点
		void SetFreq(float freqHZ);

		//设置Q
		void SetQ(float q);

		//设置Qs
		void SetQs(int idx, float q);

		//设置增益位置0-1
		void SetGainPos(float pos);

		//计算滤波参数
		void Compute();

		//获取滤波器
		vector<dsignal::Filter*> GetFilters();

		double Filtering(double input);

	private:
		Biquad lowPass[3];
		float qs_[3] = { 0.707f };


		Biquad lowShelf;
		Biquad lowShelf2;
		Biquad peak;
		Biquad highShelf;

		float maxOffsetGainlin;
		float maxOffsetGain_ = 10;//dB
		float freq_ = 0;
		float q_ = 0.707f;
		float gain[3] = { 0 };
		float pos_ = 0;

	};
}

#endif
