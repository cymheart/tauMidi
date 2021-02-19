#ifndef _Filter_h_
#define _Filter_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include "Dsignal.h"
using namespace scutils;

namespace dsignal
{
	/// <summary>
	/// 数字滤波器传递函数:
	///         b[0]+ b[1]z^-1 + b[2]z^-2 + ... + b[n]z^-n   
	/// H(z)=  --------------------------------------------
	///         a[0]+ a[1]z^-1 + a[2]z^-2 + ... + a[n]z^-n   
	/// </summary>
	class Filter
	{
	public:

		virtual void Clear();

		//设置是否启用滤波器
		void SetEnable(bool enable)
		{
			isEnable = enable;
		}

		//判断是否启用滤波器
		bool IsEnable()
		{
			return isEnable;
		}

		//设置分子系数
		//分子系数: b[0], b[1], b[2] .... b[n]
		void SetNumCoefficient(double nums[], int len);

		//设置分母系数
		//分母系数: a[0], a[1], a[2] .... a[n]
		void SetDenCoefficient(double dens[], int len);

		/// <summary>
		///	sign = 0 时，x_out 为频率响应的实部， y_out 为频率响应的虚部
		///	sign = 1 时，x_out 为频率响应的模， y_out 为频率响应的幅角
		///	sign = 2 时，x_out 为以 dB 为单位的频率响应， y_out 为频率响应的幅角
		/// </summary>
		/// <param name="x_out"></param>
		/// <param name="y_out"></param>
		/// <param name="len">频率响应的取样点数</param>
		/// <param name="sign"></param>
		virtual bool Freqz(double x_out[], double y_out[], int len, int sign);



	private:
		//是否开启
		bool isEnable = true;

		//分子系数 b[0], b[1], b[2] .... b[n]
		vector<double> num;

		//分母系数 a[0], a[1], a[2] .... a[n]
		vector<double> den;

		Dsignal dsignalProcess;
	};
}
#endif
