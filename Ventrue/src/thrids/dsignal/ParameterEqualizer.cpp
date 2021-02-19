#include"ParameterEqualizer.h"

namespace dsignal
{
	ParameterEqualizer::ParameterEqualizer()
	{
		Init();
	}

	void ParameterEqualizer::Init()
	{
		//高通
		biquad[0].biquadFilterType = BiquadFilterType::HighPass;
		biquad[0].qtype = QType::Q;
		biquad[0].f0 = 25;
		biquad[0].fs = sampleRate;
		biquad[0].level = 1;
		biquad[0].CalculateCoefficients();

		//低架
		biquad[1].biquadFilterType = BiquadFilterType::LowShelf;
		biquad[1].qtype = QType::Q;
		biquad[1].Q = 0.707;
		biquad[1].f0 = 25;
		biquad[1].gainDB = 6;
		biquad[1].fs = sampleRate;
		biquad[1].level = 1;
		biquad[1].CalculateCoefficients();


		//尖峰
		for (int i = 2; i <= 6; i++)
		{
			biquad[i].biquadFilterType = BiquadFilterType::PeakingEQ;
			biquad[i].qtype = QType::Q;
			biquad[i].Q = 0.707;
			biquad[i].f0 = 25;
			biquad[i].gainDB = 6;
			biquad[i].fs = sampleRate;
			biquad[i].level = 1;
			biquad[i].CalculateCoefficients();
		}


		//高架
		biquad[7].biquadFilterType = BiquadFilterType::HighShelf;
		biquad[7].qtype = QType::Q;
		biquad[7].Q = 0.707;
		biquad[7].f0 = 25;
		biquad[7].gainDB = 6;
		biquad[7].fs = sampleRate;
		biquad[7].level = 1;
		biquad[7].CalculateCoefficients();


		//低通
		biquad[8].biquadFilterType = BiquadFilterType::LowPass;
		biquad[8].qtype = QType::Q;
		biquad[8].f0 = 25;
		biquad[8].fs = sampleRate;
		biquad[8].level = 1;
		biquad[8].CalculateCoefficients();

	}

	void ParameterEqualizer::Enable(int filterIdx, bool isEnable)
	{
		if (biquad[filterIdx].IsEnable() == isEnable)
			return;

		biquad[filterIdx].SetEnable(isEnable);
	}


}
