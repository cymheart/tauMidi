#include"Bode.h"

namespace dsignal
{
	Bode::~Bode()
	{
		if (x_out != nullptr)
			delete[] x_out;

		if (y_out != nullptr)
			delete[] y_out;

		if (plotFreqAxisPos != nullptr)
			delete[] plotFreqAxisPos;

		if (plotGainAxisPos != nullptr)
			delete[] plotGainAxisPos;

	}

	//增加数字滤波器到要计算的bode图中
	void Bode::AddFilters(vector<dsignal::Filter*>& filterVec)
	{
		for (int i = 0; i < filterVec.size(); i++)
			filters.push_back(filterVec[i]);
	}

	//增加一个数字滤波器到要计算的bode图中
	void Bode::AddFilter(Filter* filter)
	{
		filters.push_back(filter);
	}


	//设置频率轴标尺频率点
	void Bode::SetRulerFreqs(float ruleFreq[], int len)
	{
		rulerFreqs.clear();
		for (int i = 0; i < len; i++)
			rulerFreqs.push_back(ruleFreq[i]);
	}

	//设置增益轴标尺dB点
	void Bode::SetRulerGainDBs(float ruleGainDB[], int len)
	{
		rulerGainDBs.clear();
		for (int i = 0; i < len; i++)
			rulerGainDBs.push_back(ruleGainDB[i]);
	}

	/// <param name="x_out">为以 dB 为单位的频率响应</param>
	/// <param name="y_out">频率响应的幅角</param>
	/// <param name="len">频率响应的取样点数</param>
	void Bode::Freqz(double x_out[], double y_out[], int len)
	{
		double* x_out_tmp = new double[len];
		double* y_out_tmp = new double[len];

		memset(x_out, 0, sizeof(double) * len);
		memset(y_out, 0, sizeof(double) * len);

		bool ret;
		for (int i = 0; i < filters.size(); i++)
		{
			ret = filters[i]->Freqz(x_out_tmp, y_out_tmp, len, 2);
			if (ret) {
				for (int i = 0; i < len; i++)
				{
					x_out[i] += x_out_tmp[i];
					y_out[i] += y_out_tmp[i];
				}
			}
		}

		delete[] x_out_tmp;
		delete[] y_out_tmp;
	}

	//计算生成bode图所需要的参数
	void Bode::Compute()
	{
		if (settingFreqzSampleCount != freqzSampleCount)
		{
			if (x_out != nullptr)
				delete[] x_out;

			if (y_out != nullptr)
				delete[] y_out;

			freqzSampleCount = settingFreqzSampleCount;
			x_out = new double[freqzSampleCount];
			y_out = new double[freqzSampleCount];
		}

		//
		Freqz(x_out, y_out, freqzSampleCount);

		//
		maxFreq = sampleFreq / 2;
		unitFreq = maxFreq / freqzSampleCount;

		//计算x轴点位置集合(log(freq))
		if (plotFreqAxisStart < 1) { plotFreqAxisStart = 1; }
		if (plotFreqAxisEnd > maxFreq) { plotFreqAxisEnd = maxFreq; }

		//
		int startFreqzSampleIdx = plotFreqAxisStart / unitFreq;
		int endFreqzSampleIdx = plotFreqAxisEnd / unitFreq;
		if (endFreqzSampleIdx >= freqzSampleCount) { endFreqzSampleIdx = freqzSampleCount - 1; }
		int newPlotPosCount = endFreqzSampleIdx - startFreqzSampleIdx + 1;

		if (newPlotPosCount != plotPosCount)
		{
			if (plotFreqAxisPos != nullptr)
				delete[] plotFreqAxisPos;

			if (plotGainAxisPos != nullptr)
				delete[] plotGainAxisPos;

			plotPosCount = newPlotPosCount;
			plotFreqAxisPos = new float[plotPosCount];
			plotGainAxisPos = new float[plotPosCount];
		}


		//
		freqAxisStartLogPos = log10(plotFreqAxisStart);
		freqAxisEndLogPos = log10(plotFreqAxisEnd);
		freqAxisLogWidth = freqAxisEndLogPos - freqAxisStartLogPos;

		//计算y轴点位置集合(gainAxis), y轴点位置集合(gainAxis)
		float stepDbGain = plotAreaHeigth / plotGainAxisRange;
		float maxGainDB = plotGainAxisRange / 2;

		float f;
		for (int i = startFreqzSampleIdx; i <= endFreqzSampleIdx; i++)
		{
			f = i * unitFreq; if (f < 1) f = 1;
			plotFreqAxisPos[i - startFreqzSampleIdx] = (log10(f) - freqAxisStartLogPos) / freqAxisLogWidth * plotAreaWidth;
			plotGainAxisPos[i - startFreqzSampleIdx] = (maxGainDB - x_out[i]) * stepDbGain;
		}


		//计算bode图绘制频率轴标尺频率点位
		plotRulerFreqsPos.clear();
		for (int i = 0; i < rulerFreqs.size(); i++)
		{
			if (rulerFreqs[i] < 0 || rulerFreqs[i] > maxFreq)
				continue;

			f = rulerFreqs[i]; if (f < 1) f = 1;
			float pos = (log10(f) - freqAxisStartLogPos) / freqAxisLogWidth * plotAreaWidth;
			plotRulerFreqsPos.push_back(pos);
		}


		//计算bode图绘制增益轴标尺dB点位
		plotRulerGainDBsPos.clear();
		for (int i = 0; i < rulerGainDBs.size(); i++)
		{
			if (rulerGainDBs[i] < -maxGainDB || rulerGainDBs[i] > maxGainDB)
				continue;

			float pos = (maxGainDB - rulerGainDBs[i]) * stepDbGain;
			plotRulerGainDBsPos.push_back(pos);
		}
	}

}
