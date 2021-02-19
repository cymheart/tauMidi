#ifndef _Dsignal_h_
#define _Dsignal_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include <complex>
using namespace scutils;

namespace dsignal
{
	class Dsignal
	{
	public:

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
		/// <param name="x_out"></param>
		/// <param name="y_out"></param>
		/// <param name="len">频率响应的取样点数</param>
		/// <param name="sign"></param>
		void Freqz(double num[], double den[], int num_order, int den_order,
			double x_out[], double y_out[], int len, int sign);


	private:
		complex<double> PolyValue(double p[], int order, double omega);



	};
}

#endif
