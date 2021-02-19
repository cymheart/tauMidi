#ifndef _EffectCompressorCmd_h_
#define _EffectCompressorCmd_h_

#include"Effect/EffectCompressor.h"
#include"EffectCmd.h"


namespace ventrue
{
	class DLL_CLASS EffectCompressorCmd :public EffectCmd
	{
	public:
		EffectCompressorCmd(Ventrue* ventrue, VentrueEffect* effect)
			:EffectCmd(ventrue, effect)
		{

		}

		//设置采样频率
		void SetSampleFreq(float freq);

		//设置Attack时长
		void SetAttackSec(float sec);

		//设置Release时长
		void SetReleaseSec(float sec);

		//设置比值
		void SetRadio(float radio);

		//设置门限
		void SetThreshold(float threshold);

		//设置拐点的软硬
		void SetKneeWidth(float width);

		//设置增益补偿
		void SetMakeupGain(float gain);

		//设置是否自动增益补偿
		void SetAutoMakeupGain(bool isAuto);

		//计算系数
		void CalculateCoefficients();

	private:

		//设置采样频率
		static	void _SetSampleFreq(Task* task);

		//设置Attack时长
		static void _SetAttackSec(Task* task);

		//设置Release时长
		static void _SetReleaseSec(Task* task);

		//设置比值
		static void _SetRadio(Task* task);

		//设置门限
		static void _SetThreshold(Task* task);

		//设置拐点的软硬
		static void _SetKneeWidth(Task* task);

		//设置增益补偿
		static void _SetMakeupGain(Task* task);

		//设置是否自动增益补偿
		static void _SetAutoMakeupGain(Task* task);

		//计算系数
		static void _CalculateCoefficients(Task* task);
	};
}

#endif
