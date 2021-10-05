#ifndef _CascadeBiquad_h_
#define _CascadeBiquad_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include "Biquad.h"
using namespace scutils;

namespace dsignal
{

	/**
	* 级联双二阶Biquad滤波器
	*/
	class CascadeBiquad : public Biquad
	{
	public:
		~CascadeBiquad();
		virtual void CalculateCoefficients();
		virtual void Filtering(float* inputs, uint32_t size);
		virtual double Filtering(double input);

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

	public:
		//叠加计算几次，这会使得每倍程增益变大
		int level = 1;

	private:
		vector<Biquad*> biquads;
	};
}

#endif
