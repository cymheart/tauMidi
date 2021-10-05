#include"MorphLBHFilter.h"
#include <dsignal/DsignalTypes.h>

namespace dsignal
{
	MorphLBHFilter::MorphLBHFilter()
	{
		lowPass.rbjFilterType = RBJFilterType::LowPass;
		lowPass.qtype = QType::Q;
		lowPass.SetBodeFreqzProcess(_LowPassBodeFreqz, this);

		bandPass.rbjFilterType = RBJFilterType::BandPass;
		bandPass.qtype = QType::Q;
		bandPass.SetBodeFreqzProcess(_BandPassBodeFreqz, this);

		highPass.rbjFilterType = RBJFilterType::HighPass;
		highPass.qtype = QType::Q;
		highPass.SetBodeFreqzProcess(_HighPassBodeFreqz, this);

		SetFreq(1000);
		SetQ(0.707);
		SetAlpha(0);
		Compute();
	}


	//设置频率点
	void MorphLBHFilter::SetFreq(float freqHZ)
	{
		freq_ = freqHZ;
	}

	//设置Q
	void MorphLBHFilter::SetQ(float q)
	{
		q_ = q;
	}

	//设置过渡位置0-1
	void MorphLBHFilter::SetAlpha(float a)
	{
		if (a <= 0.5)
		{
			alpha[1] = a / 0.5;
			alpha[0] = 1 - alpha[1];
			alpha[2] = 0;
		}
		else
		{
			alpha[2] = (a - 0.5) / 0.5;
			alpha[1] = 1 - alpha[2];
			alpha[0] = 0;
		}
		a_ = a;
	}

	void MorphLBHFilter::Compute()
	{
		lowPass.Q = q_;
		lowPass.f0 = freq_;
		lowPass.SetAttenuation(alpha[0]);

		bandPass.Q = q_;
		bandPass.BW = 1;
		bandPass.f0 = freq_;
		bandPass.SetAttenuation(alpha[1]);

		highPass.Q = q_;
		highPass.f0 = freq_;
		highPass.SetAttenuation(alpha[2]);

		lowPass.CalculateCoefficients();
		bandPass.CalculateCoefficients();
		highPass.CalculateCoefficients();

	}



	//获取滤波器
	vector<dsignal::Filter*> MorphLBHFilter::GetFilters()
	{
		vector<dsignal::Filter*> filters;
		filters.push_back(&lowPass);
		filters.push_back(&bandPass);
		filters.push_back(&highPass);


		return filters;
	}

	double MorphLBHFilter::Filtering(double input)
	{
		double out = 0;

		out += lowPass.Filtering(input) * alpha[0];
		out += bandPass.Filtering(input) * alpha[1];
		out += highPass.Filtering(input) * alpha[2];

		return out;
	}

	bool MorphLBHFilter::LowPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo)
	{
		bool isOK = lowPass.FreqzBySampleRegionInfo(x_out, y_out, regionInfo, 2);
		if (!isOK)
			return false;

		int len = regionInfo.lowFreqSampleCount + regionInfo.highFreqSampleCount;
		Dsignal::SoftFilterRightCurve(len - 1, -15, 3, x_out, regionInfo);
		return true;
	}


	bool MorphLBHFilter::BandPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo)
	{
		bool isOK = bandPass.FreqzBySampleRegionInfo(x_out, y_out, regionInfo, 2);
		if (!isOK)
			return false;


		int len = regionInfo.lowFreqSampleCount + regionInfo.highFreqSampleCount;
		Dsignal::SoftFilterLeftCurve(0, -15, 3, x_out, regionInfo);
		Dsignal::SoftFilterRightCurve(len - 1, -15, 3, x_out, regionInfo);
		return true;
	}

	bool MorphLBHFilter::HighPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo)
	{
		bool isOK = highPass.FreqzBySampleRegionInfo(x_out, y_out, regionInfo, 2);
		if (!isOK)
			return false;

		Dsignal::SoftFilterLeftCurve(0, -15, 3, x_out, regionInfo);
		return true;
	}


	bool MorphLBHFilter::_LowPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data)
	{
		MorphLBHFilter* filter = (MorphLBHFilter*)data;
		return filter->LowPassBodeFreqz(x_out, y_out, regionInfo);
	}

	bool MorphLBHFilter::_BandPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data)
	{
		MorphLBHFilter* filter = (MorphLBHFilter*)data;
		return filter->BandPassBodeFreqz(x_out, y_out, regionInfo);
	}

	bool MorphLBHFilter::_HighPassBodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, void* data)
	{
		MorphLBHFilter* filter = (MorphLBHFilter*)data;
		return filter->HighPassBodeFreqz(x_out, y_out, regionInfo);
	}

}
