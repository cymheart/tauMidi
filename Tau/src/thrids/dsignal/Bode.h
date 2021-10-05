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
		Bode();
		~Bode();


		//增加数字滤波器到要计算的bode图中
		void AddFilters(vector<dsignal::Filter*>& filterVec);

		//增加一个数字滤波器到要计算的bode图中
		void AddFilter(Filter* filter);

		//设置是否使用对数坐标轴
		void SetUseLogAxis(bool isUse)
		{
			isUseLogAxis = isUse;
		}

		//判断是否使用了对数坐标轴
		bool IsUseLogAxis()
		{
			return isUseLogAxis;
		}

		//设置滤波器之间是否互相变形绘制
		void SetFilterBetwwenMorphPlot(bool isMorphPlot)
		{
			isFilterBetweenMorphPlot = isMorphPlot;
		}

		//设置采样频率
		void SetSampleFreq(float sfreq)
		{
			sampleFreq = sfreq;
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

		//设置增益的偏移DB
		void SetGainOffsetDB(float offsetGainDB)
		{
			this->offsetGainDB = offsetGainDB;
		}

		//设置频率轴标尺频率点
		void SetRulerFreqs(float rulerFreqs[], int len);

		//设置增益轴标尺dB点
		void SetRulerGainDBs(float ruleGainDB[], int len);

		//计算生成bode图所需要的参数
		void Compute();

		//获取bode图绘制区域的宽度
		float GetPlotAreaWidth()
		{
			return plotAreaWidth;
		}

		//获取bode图绘制区域的高度
		float GetPlotAreaHeigth()
		{
			return plotAreaHeigth;
		}

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

	private:
		//根据给定的低频高频步进值，分别获取低频和高频类型的采样总数，
		//然后两次调用Freqz()，得到低频采样数量的xout,和高频采样数量的xout,前后合在一起
		//之所以不一次使用Freqz()全频取所有采样值，是因为对频率等分分割分配时，受对数坐标影响
		//低频会占用绘制区域的很大一块显示空间，但采样值由于平均分配，低频采样值将不足，而不足以形成连贯显示，所有
		//使用了两次Freqz(),第一次是对低频的分配采样，分配间隔比较小，在低频将获取足够的采样值，
		//高频，由于在对数坐标空间中会被压缩，将只需要很少的采样值，因此分配间隔比较大，
		//两段采样值的衔接，必须在低频和高频分割点位无缝衔接，这就要求对perLowFreqHz，perHighFreqHz严格取值适配
		void Freqz();

		//频率转换到轴位置
		float FreqToAxisPos(float freq);

	private:
		//数字滤波器
		vector<Filter*> filters;

		//是否使用对数坐标
		bool isUseLogAxis = true;

		//是否滤波器之间互相变形绘制
		bool isFilterBetweenMorphPlot = false;

		//采样频率
		float sampleFreq = 44100;

		//最大频率
		float maxFreq = 44100 / 2;

		//单位频率
		float unitFreq = maxFreq / 512;

		//采样个数设置
		int sampleCountSetting = 512;
		//采样个数
		int sampleCount = 0;

		//bode图绘制区域的宽度
		float plotAreaWidth = 200;

		//bode图绘制区域的高度
		float plotAreaHeigth = 200;

		//bode图绘制区域增益轴DB的范围
		//例如:+15dB ~ -15dB 范围为30dB
		float plotGainAxisRange = 20;

		//增益偏移
		float offsetGainDB = 0;

		//bode图绘制区域频率轴的起始位置(单位:HZ)
		float plotFreqAxisStart = 0;

		//bode图绘制区域频率轴的结束位置(单位:HZ)
		float plotFreqAxisEnd = 44100 / 2;

		float plotFreqAxisWidth = 0;

		//bode图绘制区域频率轴的起始对数位置(单位:对数)
		float freqAxisStartLogPos = 0;

		//bode图绘制区域频率轴的结束对数位置(单位:对数)
		float freqAxisEndLogPos = 0;

		//对数宽度
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
		double* x_out_tmp = nullptr;
		double* y_out_tmp = nullptr;


		float lowFreqEndHzSetting = 1000;

		float lowFreqStartHz = 0;     //低频起始位置频率
		float lowFreqEndHz = 0;      //低频结束位置频率
		int lowFreqSampleCount = 0;  //低频采样数量
		float perLowFreqHz = 0;   //低频步进值

		float highFreqStartHz = 0;  //高频起始位置频率
		float highFreqEndHz = 0;     //高频结束位置频率
		int highFreqSampleCount = 0;  //高频采样数量
		float perHighFreqHz = 0;  //高频步进值



	};
}

#endif
