#include"Dsignal.h"

namespace dsignal
{
	/// <summary>
	/// 计算数字滤波器的频率响应
	/// 
	/// 数字滤波器传递函数:
	///         b[0]+ b[1]z^-1 + b[2]z^-2 + ... + b[n]z^-n 
	/// H(z)=  --------------------------------------------
	///         a[0]+ a[1]z^-1 + a[2]z^-2 + ... + a[n]z^-n   
	/// 
	/// 频率响应为：
	///            b[0]+ b[1]e^-jw + b[2]e^-j2w + ... + b[n]e^-jnw 
	/// H(e^jw)=  --------------------------------------------
	///            a[0]+ a[1]e^-jw + a[2]e^-j2w + ... + a[n]e^-jnw   
	/// 
	/// 其中 e^-jnw = cos(nw) - isin(nw)
	/// 
	///	sign = 0 时，x_out 为频率响应的实部， y_out 为频率响应的虚部
	///	sign = 1 时，x_out 为频率响应的模， y_out 为频率响应的幅角
	///	sign = 2 时，x_out 为以 dB 为单位的频率响应， y_out 为频率响应的幅角
	/// </summary>
	/// <param name="num">数字滤波器的分子多项式系数</param>
	/// <param name="den">数字滤波器的分母多项式系数</param>
	/// <param name="num_order">分子多项式的阶数</param>
	/// <param name="den_order">分母多项式的阶数</param>
	/// <param name="x_out">输出</param>
	/// <param name="y_out">输出</param>
	/// <param name="len">频率响应的取样点数</param>
	/// <param name="sign"></param>
	void Dsignal::Freqz(double num[], double den[], int num_order, int den_order,
		double x_out[], double y_out[], int len, int sign)
	{
		int k;
		double omega;
		complex<double> h;
		double tmp;
		for (k = 0; k < len; k++)
		{
			omega = M_PI * k / ((double)len - 1);
			h = PolyValue(num, num_order, omega) / PolyValue(den, den_order, omega);

			switch (sign)
			{
			case 1:
				y_out[k] = arg(h);
				x_out[k] = abs(h);
				break;
			case 2:
				tmp = norm(h);
				y_out[k] = arg(h);
				x_out[k] = 10 * (tmp <= 0 ? -1000000 : log10(tmp));
				break;
			default:
				x_out[k] = real(h);
				y_out[k] = imag(h);
				break;
			}
		}
	}

	complex<double> Dsignal::PolyValue(double p[], int order, double omega)
	{
		int i;
		complex<double> z(cos(omega), sin(omega)), sum = 0;
		for (i = order; i >= 0; i--)
		{
			sum = sum * z + p[i];
		}
		return sum;
	}

}
