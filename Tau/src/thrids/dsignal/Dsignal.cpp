#include"Dsignal.h"

namespace dsignal
{
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
	/// <param name="x_out">输出</param>
	/// <param name="y_out">输出</param>
	/// <param name="len">频率响应的取样点数</param>
	/// <param name="start">取样点开始位置</param>
	/// <param name="end">取样点结束位置</param>
	/// <param name="sign"></param>
	void Dsignal::Freqz(double num[], double den[], int num_order, int den_order,
		double x_out[], double y_out[], int len, int start, int end, int sign)
	{
		int k;
		double omega;
		complex<double> h;
		double tmp;
		for (k = start; k <= end; k++)
		{
			omega = M_PI * k / ((double)len - 1);
			h = PolyValue(num, num_order, omega) / PolyValue(den, den_order, omega);

			switch (sign)
			{
			case 1:
				y_out[k - start] = arg(h);
				x_out[k - start] = abs(h);
				break;
			case 2:
				tmp = norm(h);
				y_out[k - start] = arg(h);
				x_out[k - start] = 10 * (tmp <= 0 ? -1000000 : log10(tmp));
				break;
			default:
				x_out[k - start] = real(h);
				y_out[k - start] = imag(h);
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

	/*
	* limitDB:截止分贝
	* softOffsetDB：软化曲线截止DB
	方程:
	 * 软化滤波器曲线左下降线
	 *        |      a*x + b                                  x<(t - w / 2)                (1)
	 *        |
	 *        |          a(x - w*0.5 - (n - b)/a)^2
	 *  y =   | a*x+b + ----------------------------          (t-w/2) <= x <= (t+w/2)      (2)
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
	 * 设置二阶贝塞尔曲线已知点为: P0((n-b)/a+w/2, n+wa/2)
	 *                             P1((n-b)/a, n)
	 *                             P2((n-b)/a-w/2, n)
	 * 分别带入点的x,y到 P = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2 ,得到:
	 * x = (1-t)^2*x0 + 2*t*(1-t)*x1 + t^2*x2      (4)
	 * y = (1-t)^2*y0 + 2*t*(1-t)*y1 + t^2*y2      (5)
	 * 化解得到:
	 * x = (x0-2x1+x2)*t^2 + 2(x1-x0)*t + x0  (6)
	 * y = (y0-2y1+y2)*t^2 + 2(y1-y0)*t + y0   (7)
	 * 代入已知点数值
	 * 可知: x0-2x1+x2 = (n-b)/a + w/2 - 2(n-b)/a + (n-b)/a - w/2 = 0
	 *       x1-x0 = (n-b)/a - (n-b)/a - w/2 = -w/2
	 *       x0 = (n-b)/a + w/2
	 *
	 *       y0-2y1+y2 = n + wa/2 - 2n + n = wa/2
	 *       y1-y0 =n - n - wa/2 = -wa/2
	 *       y0 = n+wa/2
	 *由上代入到6,7式，得到
	 *          x = -wt + (n-b)/a + w/2             (8)
	 *          y = wa/2*t^2 - wa*t + n+wa/2       (9)
	 *  8式带入到9式
	 *  即可求出2式
	 */
	void Dsignal::SoftFilterLeftCurve(int startIdx, float limitDB, float softOffsetDB, double x_out[], FreqzSampleRegionInfo& regionInfo)
	{
		int len = regionInfo.lowFreqSampleCount + regionInfo.highFreqSampleCount;
		float leftFreq = 0, centerFreq = 0, rightFreq = 0;
		float rightDB = 0;
		int rightIdx = 0;
		int centerIdx = 0;
		int i = startIdx;

		for (; i < len; i++)
		{
			if (x_out[i] < limitDB)
				continue;
			limitDB = x_out[i];
			centerFreq = regionInfo.TransIdxToFreq(i);
			centerIdx = i;
			break;
		}

		if (i == len)
			return;


		for (; i < len; i++)
		{
			if (x_out[i] < limitDB + softOffsetDB)
				continue;

			rightFreq = regionInfo.TransIdxToFreq(i);
			rightDB = x_out[i];
			rightIdx = i;
			break;
		}

		if (i == len)
			return;

		float right2Freq = regionInfo.TransIdxToFreq(i + 1);
		float right2DB = x_out[i + 1];

		float a = (rightDB - right2DB) / (rightFreq - right2Freq);
		float n = limitDB;
		float freq = 0;
		float b = rightDB - a * rightFreq;
		centerFreq = (n - b) / a;
		leftFreq = 2 * centerFreq - rightFreq;
		float w = rightFreq - leftFreq;
		float s = -w * 0.5 - (n - b) / a;

		for (i = rightIdx - 1; i >= 0; i--)
		{
			freq = regionInfo.TransIdxToFreq(i);
			x_out[i] = a * 0.5f / w * pow(freq + s, 2) + a * freq + b;
			limitDB = x_out[i];
			if (freq <= leftFreq)
				break;
		}

		for (; i >= 0; i--)
			x_out[i] = limitDB;


	}


	/*
	* limitDB:截止分贝
	* softOffsetDB：软化曲线截止DB
	方程:
	 * 软化滤波器曲线右下降线
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
	void Dsignal::SoftFilterRightCurve(int startIdx, float limitDB, float softOffsetDB, double x_out[], FreqzSampleRegionInfo& regionInfo)
	{
		int len = regionInfo.lowFreqSampleCount + regionInfo.highFreqSampleCount;
		float leftFreq = 0, centerFreq = 0, rightFreq = 0;
		float leftDB = 0;
		int leftIdx = 0;
		int i = startIdx;

		for (; i >= 0; i--)
		{
			if (x_out[i] < limitDB)
				continue;

			centerFreq = regionInfo.TransIdxToFreq(i);
			break;
		}

		if (i < 0)
			return;

		for (; i >= 0; i--)
		{
			if (x_out[i] < limitDB + softOffsetDB)
				continue;

			leftFreq = regionInfo.TransIdxToFreq(i);
			leftDB = x_out[i];
			leftIdx = i;
			break;
		}

		if (i < 0)
			return;

		float left2Freq = regionInfo.TransIdxToFreq(i - 1);
		float left2DB = x_out[i - 1];

		//
		float a = (leftDB - left2DB) / (leftFreq - left2Freq);
		float n = limitDB;
		float freq = 0;
		float b = leftDB - a * leftFreq;
		centerFreq = (n - b) / a;
		rightFreq = 2 * centerFreq - leftFreq;
		float w = rightFreq - leftFreq;
		float s = w * 0.5f - (n - b) / a;

		for (i = leftIdx + 1; i < len; i++)
		{
			freq = regionInfo.TransIdxToFreq(i);
			x_out[i] = -a * 0.5f / w * pow(freq + s, 2) + a * freq + b;
			limitDB = x_out[i];
			if (freq >= rightFreq)
				break;
		}

		for (; i < len; i++)
			x_out[i] = limitDB;
	}

}
