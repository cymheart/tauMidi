#include"FilterTest.h"

namespace dsignal
{
	FilterTest::FilterTest()
	{
		for (int i = 0; i < 3; i++)
		{
			lowPass[i].rbjFilterType = RBJFilterType::LowPass;
			lowPass[i].qtype = QType::Q;
		}

		/*lowShelf.rbjFilterType = RBJFilterType::LowShelf;
		lowShelf.qtype = QType::Q;

		peak.rbjFilterType = RBJFilterType::PeakingEQ;
		peak.qtype = QType::Q;

		highShelf.rbjFilterType = RBJFilterType::HighShelf;
		highShelf.qtype = QType::Q;*/

		for (int i = 0; i < 3; i++)
			SetQs(i, 0.707f);


		SetFreq(1000);
		SetQ(0.707f);
		SetGainPos(0);
		SetMaxOffsetGainDB(0);
		Compute();
	}

	//设置Qs
	void FilterTest::SetQs(int idx, float q)
	{
		qs_[idx] = q;
	}


	//设置频率点
	void FilterTest::SetFreq(float freqHZ)
	{
		freq_ = freqHZ;
	}

	//设置Q
	void FilterTest::SetQ(float q)
	{
		q_ = q;
	}

	//设置增益位置0-1
	void FilterTest::SetGainPos(float pos)
	{
		if (pos <= 0.5)
		{
			gain[1] = pos / 0.5 * maxOffsetGain_;
			gain[0] = maxOffsetGain_ - gain[1];
			gain[2] = 0;
		}
		else
		{
			gain[2] = (pos - 0.5) / 0.5 * maxOffsetGain_;
			gain[1] = maxOffsetGain_ - gain[2];
			gain[0] = 0;
		}
		pos_ = pos;
	}

	void FilterTest::Compute()
	{
		for (int i = 0; i < 3; i++)
		{
			lowPass[i].Q = qs_[i];
			lowPass[i].f0 = freq_;
			lowPass[i].CalculateCoefficients();
		}

		/*lowShelf.gainDB = gain[0];
		lowShelf.Q = q_;
		lowShelf.f0 = freq_;

		peak.gainDB = gain[1];
		peak.Q = q_;
		peak.f0 = freq_;

		highShelf.gainDB = gain[2];
		highShelf.Q = q_;
		highShelf.f0 = freq_;


		lowShelf.CalculateCoefficients();
		peak.CalculateCoefficients();
		highShelf.CalculateCoefficients();*/
	}

	//获取滤波器
	vector<dsignal::Filter*> FilterTest::GetFilters()
	{
		vector<dsignal::Filter*> filters;
		/*filters.push_back(&lowShelf);
		filters.push_back(&peak);
		filters.push_back(&highShelf);*/

		for (int i = 0; i < 3; i++)
		{
			filters.push_back(&lowPass[i]);
		}

		return filters;
	}

	double FilterTest::Filtering(double input)
	{
		double out = 0;
		out += lowShelf.Filtering(input);
		out += peak.Filtering(input);
		out += highShelf.Filtering(input);

		return out * maxOffsetGainlin;
	}

}
