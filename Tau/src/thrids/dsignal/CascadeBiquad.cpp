#include"CascadeBiquad.h"

namespace dsignal
{
	CascadeBiquad::~CascadeBiquad()
	{
		for (int i = 0; i < biquads.size(); i++)
		{
			DEL(biquads[i]);
		}
	}

	void CascadeBiquad::CalculateCoefficients()
	{
		Biquad::CalculateCoefficients();

		//
		for (int i = 0; i < biquads.size(); i++)
		{
			DEL(biquads[i]);
		}
		biquads.clear();

		//
		for (int i = 0; i < level - 1; i++)
		{
			Biquad* biquad = new Biquad;
			biquad->SetNumAndDenCoefficient(b0, b1, b2, a0, a1, a2);
			biquads.push_back(biquad);
		}
	}

	void CascadeBiquad::Filtering(float* inputs, uint32_t size)
	{
		if (!IsEnable())
			return;

		Biquad::Filtering(inputs, size);

		for (int i = 0; i < biquads.size(); i++)
		{
			biquads[i]->Filtering(inputs, size);
		}
	}

	double CascadeBiquad::Filtering(double input)
	{
		if (!IsEnable())
			return input;

		input = Biquad::Filtering(input);

		for (int i = 0; i < biquads.size(); i++)
		{
			input = biquads[i]->Filtering(input);
		}

		return input;
	}


	/// <summary>
	///	sign = 0 时，x_out 为频率响应的实部， y_out 为频率响应的虚部
	///	sign = 1 时，x_out 为频率响应的模， y_out 为频率响应的幅角
	///	sign = 2 时，x_out 为以 dB 为单位的频率响应， y_out 为频率响应的幅角
	/// </summary>
	/// <param name="x_out"></param>
	/// <param name="y_out"></param>
	/// <param name="len">频率响应的取样点数</param>
	/// <param name="sign"></param>
	bool CascadeBiquad::Freqz(double x_out[], double y_out[], int len, int sign)
	{
		if (!IsEnable())
			return false;

		Biquad::Freqz(x_out, y_out, len, 0, len - 1, sign);
		if (biquads.size() <= 0)
			return true;

		//
		double* x_out_tmp = new double[len];
		double* y_out_tmp = new double[len];

		for (int i = 0; i < biquads.size(); i++)
		{
			biquads[i]->Freqz(x_out_tmp, y_out_tmp, len, 0, len - 1, sign);
			for (int i = 0; i < len; i++)
			{
				x_out[i] += x_out_tmp[i];
				y_out[i] += y_out_tmp[i];
			}
		}

		delete[] x_out_tmp;
		delete[] y_out_tmp;
		return true;

	}

}
