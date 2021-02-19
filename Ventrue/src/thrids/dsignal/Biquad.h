#ifndef _Biquad_h_
#define _Biquad_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include"Filter.h"
using namespace scutils;

namespace dsignal
{
	// Biquad滤波器类型
	enum BiquadFilterType
	{
		LowPass,
		HighPass,
		BandPass,
		Notch,
		AllPass,
		PeakingEQ,
		LowShelf,
		HighShelf
	};

	enum QType
	{
		/// <summary>
		/// Q增益倍率因子决定了截止处的共振峰resonance peak(共振峰单位dB)
		/// 共振峰resonance处的频率的增益明显高于其他频率。resonance可在直流增益上以dB为单位测量
		/// 换算: resonance = 20Log(10,Q)
		/// 曲线平坦时，Q = 0.707, 即resonance = 20*Lg(0.707), resonance = -3dB，即为截至频率的增益
		/// </summary>
		Q,

		/// <summary>
		//  bandwidth单位是八度oct(octaves),1个八度相当于频率高了一倍
		//  对于BPF( Band-Pass Filters)和notch,指的是在-3db频率间的（单位:八度），
		//  对于peaking EQ ,指的是在增益中点gainDB/2(增益)位置，频率间的 （单位:八度）
		/// </summary>
		BandWidth,

		/// <summary>
		/// "shelf slope" 仅用于shelfving EQ 滤波器的斜度参数，
		/// 当S=1时，陆架坡度尽可能陡峭，并保持增益随频率单调增加或减少, 此时Q= 0.707.
		/// 当s> 1时，shelf会有一个尖峰，而破环单调性
		/// 对于固定f0/Fs和dBgain的所有其他值，搁板斜率（单位：dB/倍频程）保持与S成比例。
		/// </summary>
		ShelfSlope
	};

	// 双二阶Biquad滤波器
	// 参考:音频EQ系数的生成(Cookbook formulae for audio EQ biquad filter coefficients)
	class Biquad : public Filter
	{

	public:
		virtual void Clear();
		// 计算系数
		// 滤波器的最直接实现方式叫 “Direct Form I”
		// y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]- (a1/a0)*y[n - 1] - (a2/a0)*y[n - 2]
		virtual void CalculateCoefficients();
		//直接设置分子分母系数
		void SetNumAndDenCoefficient(double b0, double b1, double b2, double a0, double a1, double a2);
		virtual void Filtering(float* inputs, uint32_t size);
		virtual double Filtering(double input);

	public:

		BiquadFilterType biquadFilterType = BiquadFilterType::LowPass;
		QType qtype = QType::Q;

		// 采样频率
		double fs = 44100;

		// 频率增益,单位DB,仅用于peaking 和 shelving 类型的滤波器
		double gainDB = 0;

		// 通带的中心频率，或者低通滤波器的通带右边频点及高通滤波器的通带左边频点截至频率
		double f0 = 0;

		// Q增益倍率因子决定了截止处的共振峰resonance peak(共振峰单位dB)
		// 共振峰resonanceDB处的频率的增益明显高于其他频率。resonanceDB可在直流增益上以dB为单位测量
		// 换算: resonanceDB = 10*Ln(Q)
		// 曲线平坦时，Q = 0.707, 即resonanceDB = 10*Ln(0.707), resonanceDB = -3dB，即为截至频率的增益
		double Q = 1 / sqrt(2);

		//  bandwidth单位是八度oct(octaves),1个八度相当于频率高了一倍
		//  对于BPF( Band-Pass Filters)和notch,指的是在-3db频率间的（单位:八度），
		//  对于peaking EQ ,指的是在增益中点gainDB/2(增益)位置，频率间的 （单位:八度）
		double BW = 0;

		//"shelf slope" 仅用于shelfving EQ 滤波器的斜度参数，
		//当S=1时，斜度尽可能陡峭，并保持增益随频率单调增加或减少, 此时Q= 0.707.
		//当s> 1时，shelf会有一个尖峰，而破环单调性
		//对于固定f0/Fs和dBgain的所有其他值，搁板斜率（单位：dB/倍频程）保持与S成比例。
		double S = 1;

	protected:

		double a0 = 1;
		double a1 = 0;
		double a2 = 0;
		double b0 = 1;
		double b1 = 0;
		double b2 = 0;

		double x0 = 0;
		double x1 = 0;
		double x2 = 0;
		double y1 = 0;
		double y2 = 0;
	};
}

#endif
