#include"Biquad.h"
namespace dsignal
{
	void Biquad::Clear()
	{
		Filter::Clear();
		f0 = 0;
		Q = 1 / sqrt(2);
		fs = 44100;
		gainDB = 0;
		f0 = 0;
		BW = 0;
		rbjFilterType = RBJFilterType::LowPass;
		qtype = QType::Q;
		a0 = 1; a1 = 0; a2 = 0;
		b0 = 1; b1 = 0; b2 = 0;
		x0 = 0; x1 = 0; x2 = 0;
		y1 = 0; y2 = 0;
	}

	// 计算系数      
	// 滤波器的最直接实现方式叫 “Direct Form I”  
	// y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]- (a1/a0)*y[n - 1] - (a2/a0)*y[n - 2]
	void Biquad::CalculateCoefficients()
	{
		double A = 0;
		double w0 = 2.0f * M_PI * f0 / fs;
		double cos_w0 = FastCos(w0);
		double sin_w0 = FastSin(w0);

		double alpha = 0;

		if (rbjFilterType == RBJFilterType::PeakingEQ ||
			rbjFilterType == RBJFilterType::LowShelf ||
			rbjFilterType == RBJFilterType::HighShelf)
		{
			A = powf(10.0f, (gainDB / 40.0f));
		}

		switch (qtype)
		{
		case QType::Q:
			alpha = sin_w0 / (2 * Q);
			break;

		case QType::BandWidth:
			alpha = sin_w0 * sinh(log(2) / 2 * BW * w0 / sin_w0);
			break;

		case QType::ShelfSlope:
			alpha = sin_w0 / 2 * sqrt((A + 1 / A) * (1 / S - 1) + 2);
			break;
		}

		switch (rbjFilterType)
		{
		case RBJFilterType::LowPass:
			b0 = (1 - cos_w0) / 2;
			b1 = 1 - cos_w0;
			b2 = b0;
			a0 = 1 + alpha;
			a1 = -2 * cos_w0;
			a2 = 1 - alpha;
			break;

		case RBJFilterType::HighPass:
			b0 = (1 + cos_w0) / 2;
			b1 = -(1 + cos_w0);
			b2 = b0;
			a0 = 1 + alpha;
			a1 = -2 * cos_w0;
			a2 = 1 - alpha;
			break;

		case RBJFilterType::BandPass:
		{
			float q = Q;
			if (Q == 0 && qtype == QType::BandWidth)
				q = 1;

			b0 = q * alpha;
			b1 = 0;
			b2 = -q * alpha;
			a0 = 1 + alpha;
			a1 = -2 * cos_w0;
			a2 = 1 - alpha;
		}
		break;

		case RBJFilterType::AllPass:
			b0 = 1 - alpha;
			b1 = -2 * cos_w0;
			b2 = 1 + alpha;
			a0 = 1 + alpha;
			a1 = -2 * cos_w0;
			a2 = 1 - alpha;
			break;

		case RBJFilterType::Notch:
			b0 = 1;
			b1 = -2 * cos_w0;
			b2 = 1;
			a0 = 1 + alpha;
			a1 = -2 * cos_w0;
			a2 = 1 - alpha;
			break;

		case RBJFilterType::LowShelf:
		{
			double value = 2 * sqrt(A) * alpha;
			double bVal = (A + 1) - (A - 1) * cos_w0;

			b0 = A * (bVal + value);
			b1 = 2 * A * ((A - 1) - (A + 1) * cos_w0);
			b2 = A * (bVal - value);
			a0 = (A + 1) + (A - 1) * cos_w0 + value;
			a1 = -2 * ((A - 1) + (A + 1) * cos_w0);
			a2 = (A + 1) + (A - 1) * cos_w0 - value;
		}
		break;

		case RBJFilterType::HighShelf:
		{
			double value = 2 * sqrt(A) * alpha;
			double bVal = (A + 1) + (A - 1) * cos_w0;

			b0 = A * (bVal + value);
			b1 = -2 * A * ((A - 1) + (A + 1) * cos_w0);
			b2 = A * (bVal - value);
			a0 = (A + 1) - (A - 1) * cos_w0 + value;
			a1 = 2 * ((A - 1) - (A + 1) * cos_w0);
			a2 = (A + 1) - (A - 1) * cos_w0 - value;
		}
		break;

		case RBJFilterType::PeakingEQ:
		{
			b0 = 1 + alpha * A;
			b1 = -2 * cos_w0;
			b2 = 1 - alpha * A;
			a0 = 1 + alpha / A;
			a1 = -2 * cos_w0;
			a2 = 1 - alpha / A;

		}
		break;

		}

		double inv_a0 = 1 / a0;
		b0 *= inv_a0;
		b1 *= inv_a0;
		b2 *= inv_a0;
		a1 *= inv_a0;
		a2 *= inv_a0;
		a0 = 1;

		//
		double num[] = { b0, b1, b2 };
		double den[] = { a0, a1, a2 };
		SetNumCoefficient(num, 3);
		SetDenCoefficient(den, 3);
	}

	//直接设置分子分母系数
	void Biquad::SetNumAndDenCoefficient(
		double b0, double b1, double b2,
		double a0, double a1, double a2)
	{
		this->b0 = b0;
		this->b1 = b1;
		this->b2 = b2;
		this->a0 = a0;
		this->a1 = a1;
		this->a2 = a2;

		double num[] = { b0, b1, b2 };
		double den[] = { a0, a1, a2 };
		SetNumCoefficient(num, 3);
		SetDenCoefficient(den, 3);
	}

	void Biquad::Filtering(float* inputs, uint32_t size)
	{
		if (!IsEnable())
			return;

		for (uint32_t i = 0; i < size; i++)
		{
			inputs[i] = (float)Filtering(inputs[i]);
		}
	}

	double Biquad::Filtering(double input)
	{
		if (!IsEnable())
			return input;

		x0 = input;
		double output = x0 * b0 + x1 * b1 + x2 * b2 - y1 * a1 - y2 * a2;
		x2 = x1;
		x1 = x0;
		y2 = y1;
		y1 = output;
		return output;
	}

}
