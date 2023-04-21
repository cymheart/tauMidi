#ifndef _Compressor_h_
#define _Compressor_h_

#include"scutils/MathUtils.h"
using namespace scutils;

namespace dsignal
{

	/**
	* 音频压缩器
	* Dynamic Range Control
	* 算法来自: https://ww2.mathworks.cn/help/audio/ug/dynamic-range-control.html?.mathworks.com
	* 1.将原始信号从线性值转换为dB值:x->xdB，所有的DRC系统使用的增益信号都是用dB值进行处理。xdB = 20log10(x)
	* 2.增益计算，通过将dB信号传递到静态特征方程(DRC静态曲线)，获取差值gc=xsc-xdB;
	* 3.增益平滑:gc->gs；使用attack time、release time以及hold time作为系数对信号进行平滑。
	*   也就是对于台阶信号，增益信号并不是台阶的，使用指数或者对数函数进行平滑过渡。
	*   Attack time和release time对应增益信号从最终值的10%到90%所花费的时间。Hold time是在增益被施加之前的延时周期。
	*   为什么会有hold time？因为即使使用很短的attack time，limiter仍然会出现超调量，这可以在增益平滑中使用一个很短的时延来解决，例如300微秒。
	*   增益平滑使用的是一阶递归滤波器：gs(n) = (1 − k) · gs(n − 1) + k · gc(n),k=AttackTime,ReleaseTime;
	* 4.添加make-up gain(对于压缩器和限幅器):gs->gm
	* 5.dB值到线性值的转换:gm->glin
	* 6.在原始音频信号中应用计算出来的增益信号:y=glin * x
	*
	* 增益计算
	*增益计算单元进行增益信号的初步预估，增益计算单元的核心模块是静态特征，每一种类型的DRC有不同的静态特征，包括了可调的属性参数：
	*a)阈值threshold--所有的静态特性都有一个阈值，在阈值的一边，输入信号没有被修改，在阈值的另一边，输入信号被压缩、扩展以及限幅；
	*b)压缩比率Ratio--扩展器和压缩器的静态特征模块在计算增益时依赖用户提供的输入/输出的压缩比；
	*c)kneewidth-- 扩展器、压缩器以及限幅器的静态特征模块可以调整拐点的宽度；拐点宽度包含在阈值中，拐点宽度越大，阈值附近的过渡越平滑。
	   如果拐点没有提供过度，则是硬拐点；如果拐点附近有大于0的拐点宽度值，则是软拐点。
	*在计算信号增益要进行电平检测，电平检测使用递归一阶滤波器获取原始信号的电平；
	*有基于Peak和基于RMS两种方法；Peak是生成信号电平的峰值包络，RMS是计算前N个样本的RMS值作为当前样本的电平。
	*电平检测的作用是对于一个在原点波动的信号，DRC关心的是信号的峰值或者RMS值，而不关心信号的振荡情况。
	*电平检测一阶递归滤波器，使用Attack time或者release time作为滤波器的系数进行控制。
	*
	* 特征方程:
	*           | xdB                                   xdB<(T-W/2)                (1)
	*           |
	*           |       (1/R-1)(xdB -T + W/2)^2
	* xsc(xdB) =| xdB + ------------------------        (T-W/2)<=xdB<=(T+W/2)      (2)
	*           |                 2W
	*           |
	*           | T+(xdB -T)/R                           xdB>(T+W/2)               (3)
	*
	* 特征方程2式的推导，可以从二阶贝塞尔曲线的参数方程入手推导，
	* 设置二阶贝塞尔曲线已知点为: P0(T-W/2, T-W/2)
	*                             P1(T, T)
	*                             P2(T+W/2, T+W/2/R)
	* 分别带入点的x,y到 P = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2 ,得到:
	* x = (1-t)^2*x0 + 2*t*(1-t)*x1 + t^2*x2      (4)
	* y = (1-t)^2*y0 + 2*t*(1-t)*y1 + t^2*y2      (5)
	* 化解得到:
	* x = (x0-2x1+x2)*t^2 + 2(x1-x0)*t + x0  (6)
	* y = (y0-2y1+y2)*t^2 + 2(y1-y0)*t + y0   (7)
	* 代入已知点数值
	* 可知: x0-2x1+x2 = T-W/2 -2T + T+W/2 = 0
	*       x1-x0 = T - (T-W/2) = W/2
	*       x0 = T-W/2
	*
	*       y0-2y1+y2 = T-W/2 - 2T+T+W/2/R = W(1-R)/(2R)
	*       y1-y0 = T - (T-W/2) = W/2
	*       y0 = T-W/2
	*由上代入到6,7式，得到
	*          x = Wt + T-W/2                      (8)
	*          y = W(1-R)/(2R)*t^2 + Wt + T-W/2       (9)
	*  8式带入到9式
	*  即可求出2式
	*
	* 平滑处理公式:
	*
	*            | attackCoffe*gs[n-1] + (1-attackCoffe)gc[n]  if gc[n]<=gs[n-1]
	*    gs[n] = |
	*            | releaseCoffe*gs[n-1] + (1-releaseCoffe)gc[n]  if gc[n]>gs[n-1]
	*
	*  其中:   attackCoffe = exp(-logf(9) / (attackSecLen * sampleFreq));
	*          releaseCoffe = exp(-logf(9) / (releaseSecLen * sampleFreq));
	*/
	class Compressor
	{
	public:
		Compressor();

		void Clear()
		{
			gs = 0;
		}

		//设置采样频率
		void SetSampleFreq(float freq)
		{
			sampleFreq = freq;
			invSampleFreq = 1 / sampleFreq;
			CalculateCoefficients();
		}

		//设置Attack时长
		void SetAttackSec(float sec)
		{
			attackSecLen = sec;
			CalculateCoefficients();
		}

		//设置Release时长
		void SetReleaseSec(float sec)
		{
			releaseSecLen = sec;
			CalculateCoefficients();
		}

		//设置比值
		void SetRadio(float radio)
		{
			this->radio = radio;
			CalculateCoefficients();
		}

		//设置门限
		void SetThreshold(float threshold)
		{
			if (threshold > 0)
				threshold = 0;

			this->threshold = threshold;
			CalculateCoefficients();

		}

		//设置拐点的软硬
		void SetKneeWidth(float width)
		{
			kneeWidth = width;
		}

		//设置增益补偿
		void SetMakeupGain(float gain)
		{
			makeupGain = gain;
		}

		//设置增益补偿
		void SetAutoMakeupGain(bool isAuto)
		{
			isAutoMakeupGain = isAuto;
			CalculateCoefficients();
		}

		//计算系数
		void CalculateCoefficients();


		float Process(float value);

	private:
		//采样频率
		float sampleFreq = 44100;
		//采样频率倒数
		float invSampleFreq = 1 / 44100.0f;

		//启动时长
		float attackSecLen = 0.01f;

		//释放时长
		float releaseSecLen = 0.1f;

		//门限
		float threshold = -20;

		//拐点宽度
		float kneeWidth = 3;

		//压缩比
		float radio = 2;

		float gs = 0;

		float attackCoffe = 0;
		float releaseCoffe = 0;

		//是否自动增益补偿
		bool isAutoMakeupGain = false;
		//输出增益补偿
		float makeupGain = 0;

	};
}
#endif
