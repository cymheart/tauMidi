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
	*
	*
	* Compressor 中常用的参数包括如下几种。
	* Threshold:定义了开始压缩的音量。任何超过阈值的信号都将被压缩。例如当 Threshold = -10db，当信号音量超过 -10db 时，它将被压缩。
	* Ratio: 控制超过 Threshold 的信号的压缩比率。例如当 Threshold = -10，input = -5，此时信号超过 Threshold 有 5db，它将被压缩，
	*        那么压缩多少呢？这就由 Ratio 控制，当 Ratio = 5 时，信号增量从原来的 5db 被抑制为 1db，当 Ratio = 2 时，则抑制为 2.5db，以此类推。
	* Attack Time: Attack Time 和 Release Time 在一定程度上控制 Compressor ”灵敏度“。 Attack Time 定义了一旦信号超过 Threshold，Compressor 将增益降低到期望水平所需要的时间。
	* Release Time: 定义了一旦信号低于 Threshold，将增益恢复至正常水平需要的时间。
	*               Attack Time 和 Release Time 是 Compressor 中最不容易理解的两个参数，我们会在后面的 ”Level Detection“ 章节对这个两个参数有更深的理解，在这里我们简单地将它们理解为灵敏度即可。
	*               Attack Time 值越小，信号超过 Threshold 就会越快地被压缩，Release Time 越小，信号恢复的也越快。
	* 你可能会有疑问，信号超过 Threshold，我们直接进行压缩即可，为啥需要 Attack Time 和 Release Time 呢？非也非也，如果如此简单粗暴的压缩方式会引入杂音，
	* 我们为了避免杂音，首先需要得到平滑的音量曲线，而”平滑“的特性就模糊了时间的精确度，因此引入 Attack Time 和 Release Time 满足人们对时间的某种控制。
	* Make-up Gain: Compressor 降低信号的增益，因此可以施加一个额外的增益使得输入信号与输出信号的响度水平相当。
	* Knee Width: 它控制了压缩曲线的特性，曲线是尖锐的拐角，还是想膝盖一样有弧度的曲线。
	*
	* 1:1不进行压缩。无论阈值电平如何，输入和输出电平都保持不变。
  　* 1.5:1适用于微妙的压缩。这个比例是温和的，透明的声音。它将保留自然的波峰和波谷。
　　* 2:1适用于轻度压缩。这个比例可以平稳地控制动态，而不会对音色和力度造成明显的变化。
　　* 3:1适用于适度压缩。这个比例设置略显激进。它适用于温和的瞬态控制，同时保留自然的动态。
　　* 4:1适用于中等压缩。这个比例对瞬态的控制更严格。音色、冲击力和响度会有微妙的变化。
　　* 10:1适用于重度压缩。这个比例是激进的。它将大幅减少动态范围，如果用力，会导致信号失去冲击力、清晰度和存在感。
　　* 20:1到Infinity:1是限制性的。在Infinity:1时，压缩器基本上会阻止信号穿过阈值。

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

		//设置拐点的软硬(单位:dB)
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
		//定义了开始压缩的音量。任何超过阈值的信号都将被压缩。
		//例如当 Threshold = -10db，当信号音量超过 -10db 时，它将被压缩。
		float threshold = -20;

		//拐点宽度(单位：dB)
		//它控制了压缩曲线的特性，曲线是尖锐的拐角，还是想膝盖一样有弧度的曲线。
		float kneeWidth = 3;

		//压缩比
		//控制超过 Threshold 的信号的压缩比率。例如当 Threshold = -10，input = -5，此时信号超过 Threshold 有 5db，它将被压缩，
		//那么压缩多少呢？这就由 Ratio 控制，当 Ratio = 5 时，信号增量从原来的 5db 被抑制为 1db，当 Ratio = 2 时，则抑制为 2.5db，以此类推
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
