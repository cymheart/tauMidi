#ifndef _Bode_h_
#define _Bode_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include "Filter.h"
using namespace scutils;

namespace dsignal
{
	/**
	*bode图参数计算
	* by cymheart, 2020--2021.
	*/
	class Bode
	{
	public:
		~Bode();


		//增加数字滤波器到要计算的bode图中
		void AddFilters(vector<dsignal::Filter*>& filterVec);

		//增加一个数字滤波器到要计算的bode图中
		void AddFilter(Filter* filter);

		//设置采样频率
		void SetSampleFreq(float sfreq)
		{
			sampleFreq = sfreq;
		}

		//设置频率响应采样个数
		//默认值:512个
		void SetFreqzSampleCount(int count)
		{
			settingFreqzSampleCount = count;
		}

		//设置bode图绘制区域的宽度
		void SetPlotAreaWidth(float width)
		{
			plotAreaWidth = width;
		}

		//设置bode图绘制区域的高度
		void SetPlotAreaHeight(float height)
		{
			plotAreaHeigth = height;
		}

		//设置bode图绘制区域频率轴的起始Hz(单位:HZ)
		void SetPlotFreqAxisStart(float startFreqHz)
		{
			plotFreqAxisStart = startFreqHz;
		}

		//设置bode图绘制区域频率轴的结束Hz(单位:HZ)
		void SetPlotFreqAxisEnd(float endFreqHz)
		{
			plotFreqAxisEnd = endFreqHz;
		}

		//设置bode图绘制区域增益DB的范围
		//默认值:+15dB ~ -15dB 范围为30dB
		void SetPlotGainDbRange(float gainRangeDB)
		{
			if (gainRangeDB < 0)
				gainRangeDB = -gainRangeDB;
			plotGainAxisRange = gainRangeDB;
		}

		//设置频率轴标尺频率点
		void SetRulerFreqs(float rulerFreqs[], int len);

		//设置增益轴标尺dB点
		void SetRulerGainDBs(float ruleGainDB[], int len);

		//计算生成bode图所需要的参数
		void Compute();

		//bode图绘制频率轴位置点集合
		float* GetPlotFreqAxisPos()
		{
			return plotFreqAxisPos;
		}

		//获取bode图绘制增益轴位置点集合
		float* GetPlotGainAxisPos()
		{
			return plotGainAxisPos;
		}

		//获取bode图绘制轴位置点数量
		int GetPlotAxisPosCount()
		{
			return plotPosCount;
		}

		//获取bode图绘制频率轴标尺频率点位
		vector<float> GetPlotRulerFreqsPos()
		{
			return plotRulerFreqsPos;
		}

		//获取bode图绘制增益轴标尺dB点位
		vector<float> GetPlotRulerGainDBsPos()
		{
			return plotRulerGainDBsPos;
		}

		/// <param name="x_out">以 dB 为单位的频率响应</param>
		/// <param name="y_out">频率响应的幅角</param>
		/// <param name="len">频率响应的取样点数</param>
		void Freqz(double x_out[], double y_out[], int len);

	private:
		//数字滤波器
		vector<Filter*> filters;

		//采样频率
		float sampleFreq = 44100;

		//最大频率
		float maxFreq = 44100 / 2;

		//单位频率
		float unitFreq = maxFreq / 512;

		//设置频率响应采样个数
		int settingFreqzSampleCount = 512;

		//bode图绘制区域的宽度
		float plotAreaWidth = 200;

		//bode图绘制区域的高度
		float plotAreaHeigth = 200;

		//bode图绘制区域增益轴DB的范围
		//例如:+15dB ~ -15dB 范围为30dB
		float plotGainAxisRange = 30;

		//bode图绘制区域频率轴的起始位置(单位:HZ)
		float plotFreqAxisStart = 0;

		//bode图绘制区域频率轴的结束位置(单位:HZ)
		float plotFreqAxisEnd = 44100 / 2;

		float freqAxisStartLogPos = 0;
		float freqAxisEndLogPos = 0;
		float freqAxisLogWidth = 0;

		//频率轴标尺频率点
		vector<float> rulerFreqs;
		//bode图绘制频率轴标尺频率点位
		vector<float> plotRulerFreqsPos;


		//增益轴标尺dB点
		vector<float> rulerGainDBs;
		//bode图绘制增益轴标尺dB点位
		vector<float> plotRulerGainDBsPos;


		//bode图绘制频率轴位置点集合
		float* plotFreqAxisPos = nullptr;
		//bode图绘制增益轴位置点集合
		float* plotGainAxisPos = nullptr;

		//bode图绘制位置数量
		int plotPosCount = 0;


		//以 dB 为单位的频率响应
		double* x_out = nullptr;

		//频率响应的幅角
		double* y_out = nullptr;

		///频率响应采样个数
		int freqzSampleCount = 512;



	};
}

#endif
