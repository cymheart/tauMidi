#include"Bode.h"

namespace dsignal
{
	Bode::Bode()
	{
		x_out = new double[10000];
		y_out = new double[10000];
		x_out_tmp = new double[10000];
		y_out_tmp = new double[10000];
	}

	Bode::~Bode()
	{
		DEL(x_out);
		DEL(y_out);
		DEL(x_out_tmp);
		DEL(y_out_tmp);

		DEL(plotFreqAxisPos);
		DEL(plotGainAxisPos);
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

	//根据给定的低频高频步进值，分别获取低频和高频类型的采样总数，
	//然后两次调用Freqz()，得到低频采样数量的xout,和高频采样数量的xout,前后合在一起
	//之所以不一次使用Freqz()全频取所有采样值，是因为对频率等分分割分配时，受对数坐标影响
	//低频会占用绘制区域的很大一块显示空间，但采样值由于平均分配，低频采样值将不足，而不足以形成连贯显示，
	//所以使用了两次Freqz(),第一次是对低频的分配采样，分配间隔比较小，在低频将获取足够的采样值，
	//高频，由于在对数坐标空间中会被压缩，将只需要很少的采样值，因此分配间隔比较大，
	 //两段采样值的衔接，必须在低频和高频分割点位无缝衔接，这就要求对perLowFreqHz，perHighFreqHz严格取值适配
	void Bode::Freqz()
	{
		float halfSampleFreq = sampleFreq / 2;

		if (plotFreqAxisStart < 0) { plotFreqAxisStart = 0; }
		if (plotFreqAxisEnd > halfSampleFreq) { plotFreqAxisEnd = halfSampleFreq; }

		perLowFreqHz = 1;
		int lowFreqTypeSampleCount = halfSampleFreq;   //低频类型在全频率中的总采样数

		perHighFreqHz = 10;
		int highFreqTypeSampleCount = halfSampleFreq / perHighFreqHz;  //高频类型频在全频率中的总采样数

		lowFreqStartHz = 0;
		lowFreqEndHz = 0;
		int lowFreqStartSampleIdx = 0;

		highFreqStartHz = 0;
		highFreqEndHz = 0;
		int highFreqStartSampleIdx = 0;

		//要绘制的范围全部落在低频区域中
		if (plotFreqAxisStart <= lowFreqEndHzSetting && plotFreqAxisEnd <= lowFreqEndHzSetting)
		{
			lowFreqStartHz = plotFreqAxisStart;
			lowFreqEndHz = plotFreqAxisEnd;

		}
		//一部分在低频区域一部分在高频区域
		else if (plotFreqAxisStart <= lowFreqEndHzSetting && plotFreqAxisEnd > lowFreqEndHzSetting)
		{
			lowFreqStartHz = plotFreqAxisStart;
			lowFreqEndHz = lowFreqEndHzSetting;

			highFreqStartHz = lowFreqEndHzSetting;
			highFreqEndHz = plotFreqAxisEnd;
		}
		//全部在高频区域
		else
		{
			highFreqStartHz = plotFreqAxisStart;
			highFreqEndHz = plotFreqAxisEnd;
		}

		//
		lowFreqSampleCount = (lowFreqEndHz - lowFreqStartHz) / perLowFreqHz;
		lowFreqStartSampleIdx = lowFreqStartHz / perLowFreqHz;

		highFreqSampleCount = (highFreqEndHz - highFreqStartHz) / perHighFreqHz;
		highFreqStartSampleIdx = highFreqStartHz / perHighFreqHz;

		sampleCount = lowFreqSampleCount + highFreqSampleCount;
		memset(y_out, 0, sizeof(double) * sampleCount);

		//
		if (offsetGainDB == 0) {
			memset(x_out, 0, sizeof(double) * sampleCount);
		}
		else {
			for (int j = 0; j < sampleCount; j++)
				x_out[j] = -offsetGainDB;
		}

		FreqzSampleRegionInfo regionInfo;
		regionInfo.lowFreqStartHz = lowFreqStartHz;
		regionInfo.lowFreqEndHz = lowFreqEndHz;
		regionInfo.lowFreqSampleCount = lowFreqSampleCount;
		regionInfo.lowFreqTypeSampleCount = lowFreqTypeSampleCount;
		regionInfo.lowFreqStartSampleIdx = lowFreqStartSampleIdx;
		regionInfo.perLowFreqHz = perLowFreqHz;

		regionInfo.highFreqStartHz = highFreqStartHz;
		regionInfo.highFreqEndHz = highFreqEndHz;
		regionInfo.highFreqSampleCount = highFreqSampleCount;
		regionInfo.highFreqTypeSampleCount = highFreqTypeSampleCount;
		regionInfo.highFreqStartSampleIdx = highFreqStartSampleIdx;
		regionInfo.perHighFreqHz = perHighFreqHz;

		//
		for (int i = 0; i < filters.size(); i++)
		{
			if (!filters[i]->IsEnable())
				continue;

			float atten = filters[i]->GetAttenuation();
			bool isOK = false;

			if (filters[i]->IsEnableBodeFreqzProcess())
			{
				isOK = filters[i]->BodeFreqz(x_out_tmp, y_out_tmp, regionInfo);
			}
			else
			{
				if (lowFreqSampleCount > 0)
				{
					isOK = filters[i]->Freqz(
						x_out_tmp, y_out_tmp, lowFreqTypeSampleCount,
						lowFreqStartSampleIdx, lowFreqStartSampleIdx + lowFreqSampleCount - 1, 2);
				}

				if (highFreqSampleCount > 0)
				{
					isOK = filters[i]->Freqz(
						x_out_tmp + lowFreqSampleCount, y_out_tmp + lowFreqSampleCount, highFreqTypeSampleCount,
						highFreqStartSampleIdx, highFreqStartSampleIdx + highFreqSampleCount - 1, 2);
				}
			}


			if (!isOK)
				continue;

			for (int j = 0; j < lowFreqSampleCount + highFreqSampleCount; j++)
			{
				if (!isFilterBetweenMorphPlot)
				{
					x_out[j] += x_out_tmp[j];
				}
				else {
					x_out_tmp[j] += offsetGainDB;
					x_out[j] += x_out_tmp[j] * atten;
				}

				//
				y_out[j] += y_out_tmp[j];
			}
		}
	}

	//频率转换到轴位置
	float Bode::FreqToAxisPos(float freq)
	{
		float pos;
		if (isUseLogAxis)
			pos = (log10(freq) - freqAxisStartLogPos) / freqAxisLogWidth * plotAreaWidth;
		else
			pos = (freq - plotFreqAxisStart) / plotFreqAxisWidth * plotAreaWidth;
		return pos;
	}


	//计算生成bode图所需要的参数
	void Bode::Compute()
	{
		Freqz();

		if (sampleCount != plotPosCount)
		{
			DEL(plotFreqAxisPos);
			DEL(plotGainAxisPos);

			plotPosCount = sampleCount;
			plotFreqAxisPos = new float[plotPosCount];
			plotGainAxisPos = new float[plotPosCount];
		}

		//
		plotFreqAxisWidth = plotFreqAxisEnd - plotFreqAxisStart;

		//对数坐标转换	
		if (isUseLogAxis) {
			freqAxisStartLogPos = log10(plotFreqAxisStart);
			freqAxisEndLogPos = log10(plotFreqAxisEnd);
			freqAxisLogWidth = freqAxisEndLogPos - freqAxisStartLogPos;
		}

		//计算y轴点位置集合(gainAxis), y轴点位置集合(gainAxis)
		float stepDbGain = plotAreaHeigth / plotGainAxisRange;
		float maxGainDB = plotGainAxisRange / 2;

		float f;
		for (int i = 0; i < lowFreqSampleCount; i++)
		{
			f = i * perLowFreqHz + lowFreqStartHz;
			if (f < 1) { f = 1; }

			plotFreqAxisPos[i] = FreqToAxisPos(f);
			plotGainAxisPos[i] = Clampf((maxGainDB - x_out[i]) * stepDbGain, -40, plotAreaHeigth + 40);
		}


		for (int i = lowFreqSampleCount; i < lowFreqSampleCount + highFreqSampleCount; i++)
		{
			f = (i - lowFreqSampleCount) * perHighFreqHz + highFreqStartHz;
			if (f < 1) { f = 1; }

			plotFreqAxisPos[i] = FreqToAxisPos(f);
			plotGainAxisPos[i] = Clampf((maxGainDB - x_out[i]) * stepDbGain, -40, plotAreaHeigth + 40);
		}


		//计算bode图绘制频率轴标尺频率点位
		plotRulerFreqsPos.clear();
		float pos;
		for (int i = 0; i < rulerFreqs.size(); i++)
		{
			if (rulerFreqs[i] < 0 || rulerFreqs[i] > maxFreq)
				continue;

			f = rulerFreqs[i]; if (f < 1) f = 1;
			pos = FreqToAxisPos(f);
			plotRulerFreqsPos.push_back(pos);
		}


		//计算bode图绘制增益轴标尺dB点位
		plotRulerGainDBsPos.clear();
		for (int i = 0; i < rulerGainDBs.size(); i++)
		{
			if (rulerGainDBs[i] < -maxGainDB || rulerGainDBs[i] > maxGainDB)
				continue;

			pos = (maxGainDB - rulerGainDBs[i]) * stepDbGain;
			plotRulerGainDBsPos.push_back(pos);
		}
	}
}
