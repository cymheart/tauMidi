#ifndef _Dsignal_h_
#define _Dsignal_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include "DsignalTypes.h"
#include <complex>
using namespace scutils;

namespace dsignal
{


	class Dsignal
	{
	public:

		/// <summary>
		/// 计算数字滤波器的频率响应
		/// 参考:https://blog.csdn.net/liyuanbhu/article/details/38727329
		/// 
		/// 数字滤波器传递函数:
		///         b[0]+ b[1]z^-1 + b[2]z^-2 + ... + b[n]z^-n 
		/// H(z)=  --------------------------------------------
		///         a[0]+ a[1]z^-1 + a[2]z^-2 + ... + a[n]z^-n   
		/// 
		/// 频率响应为：
		///            b[0]+ b[1]e^-jw + b[2]e^-j2w + ... + b[n]e^-jnw 
		/// H(e^jw)=  -------------------------------------------------
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
		static void Freqz(double num[], double den[], int num_order, int den_order,
			double x_out[], double y_out[], int len, int start, int end, int sign);





		/*
		* limitDB:截止分贝
		* softOffsetDB：软化曲线截止DB
		* 软化滤波器曲线右下降线
		*方程:
		*        |      a*x + b                                  x<(t - w / 2)                (1)
		*        |
		*        |          a(x + w*0.5 - (n - b)/a)^2
		*  y =   | a*x+b - ----------------------------          (t-w/2) <= x <= (t+w/2)      (2)
		*        |                2w
		*        |
		*        |   n                                         x>(t + w / 2)               (3)
		*
		*其中:a为直线(1)斜率， w为软连接拐点曲线宽度，n为直线(3)的高度， t为直线方程(1)与直线方程(3)的交点的x值，也是软连接曲宽度的中点
		*取值范围:
		*  a: [1,&)
		*  w:[0,T*2]
		*  n:[0,1]
		* 方程2式的推导，可以从二阶贝塞尔曲线的参数方程入手推导，
		* 设置二阶贝塞尔曲线已知点为: P0((n-b)/a-w/2, n-wa/2)
		*                             P1((n-b)/a, n)
		*                             P2((n-b)/a+w/2, n)
		* 分别带入点的x,y到 P = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2 ,得到:
		* x = (1-t)^2*x0 + 2*t*(1-t)*x1 + t^2*x2      (4)
		* y = (1-t)^2*y0 + 2*t*(1-t)*y1 + t^2*y2      (5)
		* 化解得到:
		* x = (x0-2x1+x2)*t^2 + 2(x1-x0)*t + x0  (6)
		* y = (y0-2y1+y2)*t^2 + 2(y1-y0)*t + y0   (7)
		* 代入已知点数值
		* 可知: x0-2x1+x2 = (n-b)/a - w/2 - 2(n-b)/a + (n-b)/a + w/2 = 0
		*       x1-x0 = (n-b)/a - (n-b)/a + w/2 = w/2
		*       x0 = (n-b)/a - w/2
		*
		*       y0-2y1+y2 = n - wa/2 - 2n + n = -wa/2
		*       y1-y0 =n - n + wa/2 = wa/2
		*       y0 = n-wa/2
		*由上代入到6,7式，得到
		*          x = wt + (n-b)/a - w/2             (8)
		*          y = (-wa/2)*t^2 + wa*t + n-wa/2       (9)
		*  8式带入到9式
		*  即可求出2式
		*/
		static void SoftFilterRightCurve(int startIdx, float limitDB, float softOffsetDB, double x_out[], FreqzSampleRegionInfo& regionInfo);
		static void SoftFilterLeftCurve(int startIdx, float limitDB, float softOffsetDB, double x_out[], FreqzSampleRegionInfo& regionInfo);

	private:
		static complex<double> PolyValue(double p[], int order, double omega);



	};
}

#endif
