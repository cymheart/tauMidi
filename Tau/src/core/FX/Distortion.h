#ifndef _EffectDistortion_h_
#define _EffectDistortion_h_

#include "dsignal/Distortion.h"
#include"TauEffect.h"
#include "task/Task.h"
using namespace task;

namespace tauFX
{

	enum class DistortionType
	{
		Clip,
		Overdrive,
		Heavy,
		Fuzz,
		LinFold,
		SinFold
	};

	class DLL_CLASS Distortion : public TauEffect
	{
	public:
		Distortion(Synther* synther = nullptr);
		virtual ~Distortion();

		/** Set DistortionType
		 */
		void SetType(DistortionType type);

		/** Set the amount of drive
		\param drive Works from 0-1
		*/
		void SetDrive(float drive);

		//设置干湿混合度[0,1]
		void SetMix(float mix);

		//设置阈值[0,1], 作用于Clip,Overdrive类型
		void SetThreshold(float threshold);

		//设置阈值, 作用于Clip,Overdrive, Fuzz类型
		//正阈值：[0,1]
		//负阈值: [-1,0]
		void SetThreshold(float positiveThreshold, float negativeThreshold);


		//设置Soft宽度[0,&], 作用于Overdrive类型
		//默认值0.2
		void SetSoftWidth(float w);

		//设置整流值[0,1]
		//rectif 参数值在 0.5 以下时将执行半波整流，在 0.5 和 1 之间时将执行全波整流。
		//半波整流用于让负振幅部分保持在特定电平之上；如图例，可以通过 rectif 参数逐步将起始点 -1 调整为 0，
		//值为 0 时不会应用半波整流，值为 0.5 时则不允许负振幅低于 0，即完全削波。
		//全波整流会更进一步，随着 rectif 参数值上升，输入的负信号部分将逐步变为正信号。
		//值为 1 时将实现全波整流，负信号部分的极性将完全反转。
		//它能起到让基频翻倍的作用，可用于实现“高八度”效果,即全波整流后，所得到的波形分解的基波频率将会提升1倍，频率提升1倍，即音调提升一个8度
		void SetRectif(float rectif);

		//设置滤波器过渡位置[0,1]
		void SetFilterAlpha(float a);

		//设置频率点
		void SetFilterFreq(float freqHZ);

		//设置Q
		void SetFilterQ(float q);

		//开启前置滤波器
		void EnablePreFilter();


		//开启后置滤波器
		void EnablePostFilter();


		//设置不开启滤波器
		void NotEnableFilter();

	public:

		/** Set DistortionType
		*/
		void SetTypeTask(DistortionType type);

		/** Set the amount of drive
			\param drive Works from 0-1
		*/
		void SetDriveTask(float drive);

		//设置干湿混合度[0,1]
		void SetMixTask(float mix);

		//设置阈值[0,1], 作用于Clip,Overdrive类型
		void SetThresholdTask(float threshold);

		//设置阈值, 作用于Clip,Overdrive, Fuzz类型
		//正阈值：[0,1]
		//负阈值: [-1,0]
		void SetThresholdTask(float positiveThreshold, float negativeThreshold);

		//设置Soft宽度[0,&], 作用于Overdrive类型
		//默认值0.2
		void SetSoftWidthTask(float w);


		//设置整流值[0,1]
		//rectif 参数值在 0.5 以下时将执行半波整流，在 0.5 和 1 之间时将执行全波整流。
		//半波整流用于让负振幅部分保持在特定电平之上；如图例，可以通过 rectif 参数逐步将起始点 -1 调整为 0，
		//值为 0 时不会应用半波整流，值为 0.5 时则不允许负振幅低于 0，即完全削波。
		//全波整流会更进一步，随着 rectif 参数值上升，输入的负信号部分将逐步变为正信号。
		//值为 1 时将实现全波整流，负信号部分的极性将完全反转。
		//它能起到让基频翻倍的作用，可用于实现“高八度”效果,即全波整流后，所得到的波形分解的基波频率将会提升1倍，频率提升1倍，即音调提升一个8度
		void SetRectifTask(float rectif);

		//设置滤波器过渡位置[0,1]
		void SetFilterAlphaTask(float a);

		//设置频率点
		void SetFilterFreqTask(float freqHZ);

		//设置Q
		void SetFilterQTask(float q);

		//开启前置滤波器
		void EnablePreFilterTask();


		//开启后置滤波器
		void EnablePostFilterTask();


		//设置不开启滤波器
		void NotEnableFilterTask();


	private:
		static void _SetDriveTask(Task* task);
		static void _SetTypeTask(Task* task);
		static void _SetMixTask(Task* task);
		static void _SetThresholdTask(Task* task);
		static void _SetThresholdTask2(Task* task);
		static void _SetSoftWidthTask(Task* task);
		static void _SetRectifTask(Task* task);
		static void _SetFilterAlphaTask(Task* task);
		static void _SetFilterFreqTask(Task* task);
		static void _SetFilterQTask(Task* task);
		static void _EnablePreFilterTask(Task* task);
		static void _EnablePostFilterTask(Task* task);
		static void _NotEnableFilterTask(Task* task);

	private:
		void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount);

	private:
		dsignal::Distortion* leftDistortion;
		dsignal::Distortion* rightDistortion;
	};
}
#endif
