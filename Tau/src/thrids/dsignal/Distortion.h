#ifndef _Distortion_h_
#define _Distortion_h_

#include"scutils/MathUtils.h"
#include <dsignal/MorphLBHFilter.h>
using namespace scutils;

namespace dsignal
{
	enum class DistortionType
	{
		//在振幅达到过载电平时，“Clip”失真类型将采用硬拐点（即无过渡）方式，
		//将正负振幅对称削去相同的电平
		Clip,

		//在振幅达到过载电平时，“Overdrive”失真类型将采用软拐点方式，
		//缓慢过渡至完全削波，将正负振幅对称削去相同的电平
		Overdrive,

		//在振幅达到过载电平时，“Heavy”失真类型将采用软拐点方式，缓慢过渡至完全削波，
		//将正负振幅对称削去相同的电平。传递函数在零值周围曲线更陡峭，所以削波保持的时间会更长。
		//它通常与较大的 Drive 参数值一起使用，以获得重金属失真效果
		Heavy,

		//“Fuzz”失真类型会为正负振幅不对称地削去电平。它为负振幅采用软拐点方式，缓慢过渡至完全削波，
		//而正振幅在高电平时则使用硬拐点。此类型的失真更适合单音独奏型素材，
		//因为在输入复杂的复音时，它会很快变得非常不和谐
		Fuzz,

		//线性折叠失真
		//参考serum插件的效果器distortion
		LinFold,

		//sin波失真
		//参考serum插件的效果器distortion
		SinFold
	};

	/**
	* 失真效果器 2021.9 by cymheart
	* 参考: Wwise Guitar Distortion（Wwise 吉他失真效果器）以及 serum插件的效果器distortion
	* 相关连接: https://www.audiokinetic.com/zh/library/edge/?source=Help&id=wwise_guitar_distortion_plug_in
	*
	*/
	class Distortion
	{
	public:

		void Init();

		//设置失真类型
		inline void SetType(DistortionType type)
		{
			isUpdateCoffe = true;
			this->type = type;
		}

		//设置阈值, 作用于Clip,Overdrive, Fuzz类型
		//阈值：[0,1]
		inline void SetThreshold(float threshold)
		{
			SetThreshold(threshold, -threshold);
		}

		//设置阈值, 作用于Clip,Overdrive, Fuzz类型
		//正阈值：[0,1]
		//负阈值: [-1,0]
		inline void SetThreshold(float positiveThreshold, float negativeThreshold)
		{
			isUpdateCoffe = true;
			n = Clampf(positiveThreshold, 0, 1);
			negN = Clampf(negativeThreshold, -1, 0);
		}

		//设置Soft宽度[0,&], 作用于Overdrive, Fuzz类型
		//默认值0.2
		inline void SetSoftWidth(float w)
		{
			isUpdateCoffe = true;
			softWidth = w;
			if (softWidth < 0)
				softWidth = 0;

		}

		//设置drive[0,1]
		inline void SetDrive(float drive)
		{
			isUpdateCoffe = true;
			this->drive = Clampf(drive, 0, 1);
		}

		//设置整流值[0,1]
		//rectif 参数值在 0.5 以下时将执行半波整流，在 0.5 和 1 之间时将执行全波整流。
		//半波整流用于让负振幅部分保持在特定电平之上；如图例，可以通过 rectif 参数逐步将起始点 -1 调整为 0，
		//值为 0 时不会应用半波整流，值为 0.5 时则不允许负振幅低于 0，即完全削波。
		//全波整流会更进一步，随着 rectif 参数值上升，输入的负信号部分将逐步变为正信号。
		//值为 1 时将实现全波整流，负信号部分的极性将完全反转。
		//它能起到让基频翻倍的作用，可用于实现“高八度”效果,即全波整流后，所得到的波形分解的基波频率将会提升1倍，频率提升1倍，即音调提升一个8度
		inline void SetRectif(float rectif)
		{
			this->rectif = Clampf(rectif, 0, 1);

			if (this->rectif <= 0.5)
				rectifLimit = -1 + rectif / 0.5;
			else
				rectifLimit = (rectif - 0.5) / 0.5;
		}

		//设置滤波器过渡位置0-1
		void SetFilterAlpha(float a);

		//设置频率点
		void SetFilterFreq(float freqHZ);

		//设置Q
		void SetFilterQ(float q);

		//设置开启前置滤波器
		void EnablePreFilter()
		{
			isEnablePostFilter = false;
			isEnablePreFilter = true;
		}

		//设置开启后置滤波器
		void EnablePostFilter()
		{
			isEnablePreFilter = false;
			isEnablePostFilter = true;
		}

		//设置不开启滤波器
		void NotEnableFilter()
		{
			isEnablePreFilter = false;
			isEnablePostFilter = false;
		}

		//设置干湿混合度[0,1]
		inline void SetMix(float mix)
		{
			this->mix = Clampf(mix, 0, 1);
		}

		float Process(float in);

	private:
		//更新系数值
		void UpdateCoffes();

		//在振幅达到过载电平时，“Clip”失真类型将采用硬拐点（即无过渡）方式，
		//将正负振幅对称削去相同的电平
		float Clip(float in);


		/*
		* 在振幅达到过载电平时，“Overdrive”失真类型将采用软拐点方式，
		*缓慢过渡至完全削波，将正负振幅对称削去相同的电平
		*Overdrive方程:
		*        |      a*x                                   x<(t - w / 2)                (1)
		*        |
		*        |          a(x-n/a + w/2)^2
		*  y =   | a*x - ------------------------            (t-w/2) <= x <= (t+w/2)      (2)
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
		* 设置二阶贝塞尔曲线已知点为: P0(n/a-w/2, n-wa/2)
		*                             P1(n/a, n)
		*                             P2(n/a+w/2, n)
		* 分别带入点的x,y到 P = (1-t)^2*P0 + 2*t*(1-t)*P1 + t^2*P2 ,得到:
		* x = (1-t)^2*x0 + 2*t*(1-t)*x1 + t^2*x2      (4)
		* y = (1-t)^2*y0 + 2*t*(1-t)*y1 + t^2*y2      (5)
		* 化解得到:
		* x = (x0-2x1+x2)*t^2 + 2(x1-x0)*t + x0  (6)
		* y = (y0-2y1+y2)*t^2 + 2(y1-y0)*t + y0   (7)
		* 代入已知点数值
		* 可知: x0-2x1+x2 = n/a - w/2 - 2n/a + n/a + w/2 = 0
		*       x1-x0 = n/a - n/a + w/2 = w/2
		*       x0 = n/a - w/2
		*
		*       y0-2y1+y2 = n - wa/2 - 2n + n = (-wa)/2
		*       y1-y0 =n- n + wa/2 = wa/2
		*       y0 = n-wa/2
		*由上代入到6,7式，得到
		*          x = wt + n/a - w/2                 (8)
		*          y = -wa/2*t^2 + wat + n-wa/2       (9)
		*  8式带入到9式
		*  即可求出2式
		*/
		float Overdrive(float in);

		//在振幅达到过载电平时，“Heavy”失真类型将采用软拐点方式，缓慢过渡至完全削波，
		//将正负振幅对称削去相同的电平。传递函数在零值周围曲线更陡峭，所以削波保持的时间会更长。
		//它通常与较大的 Drive 参数值一起使用，以获得重金属失真效果
		float Heavy(float in);

		//“Fuzz”失真类型会为正负振幅不对称地削去电平。它为负振幅采用软拐点方式，缓慢过渡至完全削波，
		//而正振幅在高电平时则使用硬拐点。此类型的失真更适合单音独奏型素材，
		//因为在输入复杂的复音时，它会很快变得非常不和谐
		float Fuzz(float in);

		//线性折叠失真
		//参考serum插件的效果器distortion
		float LineFold(float in);

		//sin波失真
		//参考serum插件的效果器distortion
		float SinFold(float in);


		//整流处理
		//rectif 参数值在 0.5 以下时将执行半波整流，在 0.5 和 1 之间时将执行全波整流。
		//半波整流用于让负振幅部分保持在特定电平之上；如图例，可以通过 rectif 参数逐步将起始点 -1 调整为 0，
		//值为 0 时不会应用半波整流，值为 0.5 时则不允许负振幅低于 0，即完全削波。
		//全波整流会更进一步，随着 rectif 参数值上升，输入的负信号部分将逐步变为正信号。
		//值为 1 时将实现全波整流，负信号部分的极性将完全反转。
		//它能起到让基频翻倍的作用，可用于实现“高八度”效果,即全波整流后，所得到的波形分解的基波频率将会提升1倍，频率提升1倍，即音调提升一个8度
		float Rectification(float in);

	private:
		DistortionType type = DistortionType::Overdrive;

		float leftRange = 0;
		float leftRange2 = 0;
		float midRange = 0;
		float coff1, coff2;

		//设置的拐点宽度
		float softWidth = 0.2;
		float drive = 1 / 9;
		float mix = 0.5;

		//overdrive:a为直线(1)斜率
		//linflod:a为周期的倒数
		float a = 2;

		//n为直线(3)的高度
		float n = 1;
		float negN = -0.1;

		//w为软拐点连接曲线宽度
		float w = 0.2;

		//周期值
		float period = 0;
		float recipPI = 2 / M_PI;

		//整流程度0~1
		float rectif = 0;
		//整流限制值
		float rectifLimit = -1;

		//滤波器
		MorphLBHFilter filter;

		//是否启用前置滤波器
		bool isEnablePreFilter = false;

		//是否启用后置滤波器
		bool isEnablePostFilter = false;


		//是否更新系数值
		bool isUpdateCoffe = true;

		//是否更新滤波器系数值
		bool isUpdateFilterCoffe = true;

	};

}

#endif
