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

		//设置滤波衰减值
		void SetAttenuation(float atten)
		{
			atten_ = atten;
		}

		//获取滤波衰减值
		float GetAttenuation()
		{
			return atten_;
		}

		//设置系数(分子分母对半分)
		void SetCoefficient(double coeff[], int count);

		//设置分子系数
		//分子系数: b[0], b[1], b[2] .... b[n]
		void SetNumCoefficient(double nums[], int len);

		//设置分母系数
		//分母系数: a[0], a[1], a[2] .... a[n]
		void SetDenCoefficient(double dens[], int len);

		//设置伯德图处理函数
		void SetBodeFreqzProcess(BodeFreqzProcessCallBack  bodeFreqz, void* data)
		{
			this->bodeFreqz = bodeFreqz;
			this->bodeFreqzData = data;
			isEnableBodeFreqzProcess = true;
		}

		//设置是否启用BodeFreqzProcess
		void SetEnableBodeFreqzProcess(bool isEnable)
		{
			isEnableBodeFreqzProcess = isEnable;
		}

		//判断是否启用了BodeFreqzProcess 
		bool IsEnableBodeFreqzProcess()
		{
			return isEnableBodeFreqzProcess;
		}


		/// <summary>
		///	sign = 0 时，x_out 为频率响应的实部， y_out 为频率响应的虚部
		///	sign = 1 时，x_out 为频率响应的模， y_out 为频率响应的幅角
		///	sign = 2 时，x_out 为以 dB 为单位的频率响应， y_out 为频率响应的幅角
		/// </summary>
		/// <param name="x_out"></param>
		/// <param name="y_out"></param>
		/// <param name="len">频率响应的取样点数</param>
		/// <param name="start">取样点开始位置</param>
		/// <param name="end">取样点结束位置</param>
		/// <param name="sign"></param>
		virtual bool Freqz(double x_out[], double y_out[], int len, int start, int end, int sign);

		bool FreqzBySampleRegionInfo(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo, int sign);

		bool BodeFreqz(double x_out[], double y_out[], FreqzSampleRegionInfo& regionInfo);

	private:
		//是否开启
		bool isEnable = true;

		//分子系数 b[0], b[1], b[2] .... b[n]
		vector<double> num;

		//分母系数 a[0], a[1], a[2] .... a[n]
		vector<double> den;

		//滤波衰减值
		float atten_ = 1;

		BodeFreqzProcessCallBack bodeFreqz = nullptr;
		void* bodeFreqzData = nullptr;
		//是否启用BodeFreqzProcess 
		bool isEnableBodeFreqzProcess = false;

	};
}
#endif
