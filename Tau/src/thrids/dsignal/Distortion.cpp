#include"Distortion.h"

namespace dsignal
{
	void Distortion::Init()
	{
		type = DistortionType::Overdrive;
		SetDrive(1 / 9);
	}

	//设置滤波器过渡位置0-1
	void Distortion::SetFilterAlpha(float a)
	{
		isUpdateFilterCoffe = true;
		filter.SetAlpha(a);
	}

	//设置频率点
	void Distortion::SetFilterFreq(float freqHZ)
	{
		isUpdateFilterCoffe = true;
		filter.SetFreq(freqHZ);
	}

	//设置Q
	void Distortion::SetFilterQ(float q)
	{
		isUpdateFilterCoffe = true;
		filter.SetQ(q);
	}


	//更新系数值
	void Distortion::UpdateCoffes()
	{
		switch (type)
		{
		case dsignal::DistortionType::Clip:
			a = 1 + 9 * drive;
			leftRange = n / a;
			break;

		case dsignal::DistortionType::Overdrive:
			a = 1 + 9 * drive;

			if (n / a - softWidth / 2 < 0)
				w = n / a * 2;
			else
				w = softWidth;

			//             a(x - n / a + w / 2) ^ 2
			// y =  a * x - ------------------------
			//		              2w
			coff1 = a / (2 * w);
			coff2 = w / 2 - n / a;

			leftRange = n / a - w / 2;
			midRange = n / a + w / 2;
			break;


		case dsignal::DistortionType::Heavy:
			if (drive <= 0.5)
				a = drive / 0.5 * 15;
			else
				a = 15 + (drive - 0.5) / 0.5 * (500 - 15);

			a = Clampf(a, 0, 500);
			break;

		case dsignal::DistortionType::Fuzz:
			a = 1 + 9 * drive;

			if (-negN / a - softWidth / 2 < 0)
				w = -negN / a * 2;
			else
				w = softWidth;

			coff1 = a / (2 * w);
			coff2 = w / 2 - (-negN) / a;

			leftRange = -negN / a - w / 2;
			midRange = -negN / a + w / 2;

			leftRange2 = n / a;
			break;

		case dsignal::DistortionType::LinFold:

			//1个单位长度包含了1/4个波，
			//drive=0时，a为1/4个波
			//drive=1时，a为11+1/4个波
			a = 1.0f / 4.0f + 11 * drive;

			//如果a为1/4，那周期就为4，即默认状态下需要4个时间完成1个完整的波
			period = 1 / a;
			break;

		case dsignal::DistortionType::SinFold:
			//1个单位长度包含了1/4个波，
			//drive=0时，a为1/4个波
			//drive=1时，a为11+1/4个波
			a = 1.0f / 4.0f + 11 * drive;
			break;

		}

	}

	float Distortion::Process(float in)
	{
		if (isUpdateCoffe)
		{
			UpdateCoffes();
			isUpdateCoffe = false;
		}

		if (isUpdateFilterCoffe)
		{
			filter.Compute();
			isUpdateFilterCoffe = false;
		}

		float out = in;

		//前滤波
		if (isEnablePreFilter)
			out = filter.Filtering(in);


		//失真处理
		switch (type)
		{
		case dsignal::DistortionType::Clip:
			out = Clip(out);
			break;
		case dsignal::DistortionType::Overdrive:
			out = Overdrive(out);
			break;
		case dsignal::DistortionType::Heavy:
			out = Heavy(out);
			break;
		case dsignal::DistortionType::Fuzz:
			out = Fuzz(out);
			break;
		case dsignal::DistortionType::LinFold:
			out = LineFold(out);
			break;
		case dsignal::DistortionType::SinFold:
			out = SinFold(out);
			break;
		default:
			break;
		}

		//整流
		out = Rectification(out);

		//后滤波
		if (isEnablePostFilter)
			out = filter.Filtering(out);

		//干湿混合
		//out * mix + in * (1 - mix)
		out = (out - in) * mix + in;

		return out;

	}

	//在振幅达到过载电平时，“Clip”失真类型将采用硬拐点（即无过渡）方式，
	//将正负振幅对称削去相同的电平
	float Distortion::Clip(float in)
	{
		float out;

		if (in < 0)
		{
			if (in > -leftRange)
				out = a * in;
			else
				out = -n;
		}
		else
		{
			if (in < leftRange)
				out = a * in;
			else
				out = n;
		}
		return out;
	}

	//在振幅达到过载电平时，“Overdrive”失真类型将采用软拐点方式，
	//缓慢过渡至完全削波，将正负振幅对称削去相同的电平
	float Distortion::Overdrive(float in)
	{
		float out;

		if (in < 0)
		{
			if (in > -leftRange)
				out = a * in;
			else if (in > -midRange)
				out = -(a * -in - coff1 * FastPower(-in + coff2, 2));
			else
				out = -n;
		}
		else
		{
			if (in < leftRange)
				out = a * in;
			else if (in < midRange) {
				//              a(x - n / a + w / 2) ^ 2
				// y =  a * x - ------------------------
				//		              2w
				out = a * in - coff1 * FastPower(in + coff2, 2);
			}
			else
				out = n;
		}
		return out;
	}

	//在振幅达到过载电平时，“Heavy”失真类型将采用软拐点方式，缓慢过渡至完全削波，
	//将正负振幅对称削去相同的电平。传递函数在零值周围曲线更陡峭，所以削波保持的时间会更长。
	//它通常与较大的 Drive 参数值一起使用，以获得重金属失真效果
	float Distortion::Heavy(float in)
	{
		return ((1 + a) * in) / (1 + a * abs(in));
	}


	//“Fuzz”失真类型会为正负振幅不对称地削去电平。它为负振幅采用软拐点方式，缓慢过渡至完全削波，
	//而正振幅在高电平时则使用硬拐点。此类型的失真更适合单音独奏型素材，
	//因为在输入复杂的复音时，它会很快变得非常不和谐
	float Distortion::Fuzz(float in)
	{
		float out;

		if (in < 0)
		{
			if (in > -leftRange)
				out = a * in;
			else if (in > -midRange)
				out = -(a * -in - coff1 * powf(-in + coff2, 2));
			else
				out = -negN;
		}
		else
		{
			if (in < leftRange2)
				out = a * in;
			else
				out = n;
		}
		return out;
	}

	//线性折叠失真
	//参考serum插件的效果器distortion
	float Distortion::LineFold(float in)
	{
		// a = 1/period  a为周期的倒数
		//
		//平移in值到标准三角波位置(此位置y偏移1)
		in += period * 0.25;

		//计算in到一个周期中
		float p = floor(in * a);
		p = in - p * period;

		//比例尺计算结果值，并下移到中心位置
		float out = abs(2 - 4 * p * a) - 1;
		return out;
	}

	//sin波失真
	//参考serum插件的效果器distortion
	float Distortion::SinFold(float in)
	{
		return sinf(in * 2 * M_PI * a);
	}

	//整流处理
	//rectif 参数值在 0.5 以下时将执行半波整流，在 0.5 和 1 之间时将执行全波整流。
	//半波整流用于让负振幅部分保持在特定电平之上；如图例，可以通过 rectif 参数逐步将起始点 -1 调整为 0，
	//值为 0 时不会应用半波整流，值为 0.5 时则不允许负振幅低于 0，即完全削波。
	//全波整流会更进一步，随着 rectif 参数值上升，输入的负信号部分将逐步变为正信号。
	//值为 1 时将实现全波整流，负信号部分的极性将完全反转。
	//它能起到让基频翻倍的作用，可用于实现“高八度”效果,即全波整流后，所得到的波形分解的基波频率将会提升1倍，频率提升1倍，即音调提升一个8度
	float Distortion::Rectification(float in)
	{
		if (rectifLimit == 0)
			return in;
		else if (rectif <= 0.5)
		{
			if (in < rectifLimit)
				in = rectifLimit;
		}
		else if (in > rectifLimit)
		{
			in = rectifLimit;
		}

		return in;

	}
}
