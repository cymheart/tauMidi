/*
* 调制器(Modulator)
* 调制器由Midi的控制器值输入到输入端，或内部预设输入值输入到输入端，再经过调制方法和控制参量，调节增加目标值到指定值
* 调制器的最终调制目标是区域(Zone)的生成器(generator)的值,最终生成器的值将会影响区域(Zone)中的生成器的值，
* 在区域发声器(ZoneSounder)中的，将使用这些生成器修改值，控制区域声音的发声方式
* 当输入端类型Modulator，说明可以通过调制器形成链式调节，即:
* 调制器1可以调制调制器2的数值，再由调制器2调制调制器3，
* 最终由最后一个调制器调制目标生成器的值，形成一个链式调节
* by cymheart, 2020--2021.
*/

#ifndef _Modulator_h_
#define _Modulator_h_

#include"TauTypes.h"
#include <Midi\MidiTypes.h>

namespace tau
{
	// 调制器类型
	enum class ModulatorType
	{
		//内部的
		Inside,
		//通用的
		Common
	};

	// 输出运算方式
	enum class OutOpType
	{
		//未定义，将使用内部默认设定
		Unknown,

		// 相加
		Add,

		// 相乘
		Mul,

		//替换
		Replace
	};


	// 调制器源变换处理类型
	// xy曲线映射方法,或者自定义方法
	enum class ModSourceTransformType
	{
		//线性变换
		Linear = 0,
		//凹曲线变换
		Concave = 1,
		//凸曲线变换
		Convex = 2,
		//开关变换
		Switch = 3,
		//自定义方法变换
		Func = 4
	};


	// 值处理方式
	enum class ModTransformType
	{
		Linear,   //原值 
		Absolute, //绝对值
		Negative,  //取负
	};

	//输入输出状态
	enum class ModIOState
	{
		//没有输入值
		Non = -1,
		//具有输入值，等待输出
		Inputed = 1,
		//已经输入过，具有输出值
		Ouputed = 0
	};

	//调制器输入端预设输入类型的所有枚举值
	enum class ModInputPreset :int
	{
		//当前端口没有输入
		NoController = 0,
		//当前输入端输入音符的力度值
		NoteOnVelocity = 2,
		//当前输入端输入音符的键值
		NoteOnKeyNumber = 3,
		PolyPressure = 10,
		//当前输入端输入的midi控制的总体平均压力值
		ChannelPressure = 13,
		//滑音
		PitchWheel = 14,
		PitchWheelSensivity = 16,

		//
		CoarseTune,
		FineTune
	};

	// 调制器输入端输入类型
	enum class ModInputType
	{
		//预设类型
		Preset,

		//控制器
		//Midi的控制器值
		MidiController,

		//调制器
		Modulator
	};

	// 调制器输入信息
	class ModInputInfo
	{
	public:
		// 输入类型
		ModInputType inputType = ModInputType::Preset;

		//预设值
		ModInputPreset inputPreset = ModInputPreset::NoController;

		//输入控制器号
		MidiControllerType ctrlType = MidiControllerType::PanMSB;

		// 输入调制器
		Modulator* inputModulator = nullptr;

		// 输入端口
		int inputPort = 0;

		// 输入原生值
		float inputNativeValue = 0;

		// 输入原生值的最小限制
		float inputNativeValueMin = 0;

		// 输入原生值的最大限制
		float inputNativeValueMax = 1;

	};



	/*
	* 调制器(Modulator)
	* 调制器由Midi的控制器值输入到输入端，或内部预设输入值输入到输入端，再经过调制方法和控制参量，调节增加目标值到指定值
	* 调制器的最终调制目标是区域(Zone)的生成器(generator)的值,最终生成器的值将会影响区域(Zone)中的生成器的值，
	* 在区域发声器(ZoneSounder)中的，将使用这些生成器修改值，控制区域声音的发声方式
	* 当输入端类型Modulator，说明可以通过调制器形成链式调节，即:
	* 调制器1可以调制调制器2的数值，再由调制器2调制调制器3，
	* 最终由最后一个调制器调制目标生成器的值，形成一个链式调节
	*/
	class Modulator
	{

	public:

		Modulator();

		~Modulator();

		ModulatorType GetType()
		{
			return type;
		}

		inline void SetType(ModulatorType type)
		{
			this->type = type;
		}

		//判断是否和modulator一样
		//不比较amount
		bool IsSame(Modulator* modulator);

		//获取端口数量
		inline size_t GetInputPortCount()
		{
			return inputInfos.size();
		}

		// <summary>
		// 增加一个输入到调制器的指定端口
		// </summary>
		// <param name="inputType">输入类型</param>
		// <param name="inputPort">输入端口</param>
		// <param name="inputNativeValueMin">输入的原生最小值</param>
		// <param name="inputNativeValueMax">输入的原生最大值</param>
		void AddInputInfo(
			ModInputType inputType, ModInputPreset inputPreset, MidiControllerType ctrlType,
			int inputPort, float inputNativeValueMin, float inputNativeValueMax);

		//<summary>
		//增加一个预设输入到调制器的指定端口
		//< / summary>
		//<param name = "inputPort">输入端口< / param>
		//<param name = "inputNativeValueMin">输入的原生最小值< / param>
		//<param name = "inputNativeValueMax">输入的原生最大值< / param>
		void AddPresetInputInfo(
			ModInputPreset inputPreset, int inputPort,
			float inputNativeValueMin, float inputNativeValueMax);

		//<summary>
		//增加一个Midi控制器输入到调制器的指定端口
		//< / summary>
		//<param name = "inputType">输入类型< / param>
		//<param name = "inputPort">输入端口< / param>
		//<param name = "inputNativeValueMin">输入的原生最小值< / param>
		//<param name = "inputNativeValueMax">输入的原生最大值< / param>
		void AddMidiCtrlInputInfo(
			MidiControllerType ctrlType, int inputPort,
			float inputNativeValueMin, float inputNativeValueMax);

		//获取输入端口的输入值
		inline float GetInputValue(int port)
		{
			return inputInfos[port]->inputNativeValue;
		}

		//获取输入端口的输入类型
		inline ModInputType GetInputType(int port)
		{
			return inputInfos[port]->inputType;
		}

		//获取输入端口的控制器类型
		inline MidiControllerType GetInputCtrlType(int port)
		{
			return inputInfos[port]->ctrlType;
		}

		//获取输入端口的预设类型
		inline ModInputPreset GetInputPresetType(int port)
		{
			return inputInfos[port]->inputPreset;
		}

		// <summary>
		// 移除指定端口上的所有输入信息
		// </summary>
		// <param name="port">指定端口</param>
		void RemoveAllInputInfoFromPort(int port);


		// 设置调制器端口上的控制器变换方式
		// <param name="port">端口</param>
		// <param name="type">变换线类型</param>
		// dir 变换线方向
		//0: Positive minY->maxY
		//1: Negative maxY->minY
		// polar 变换线极性  
		//0: Unipolar y值范围: 0->1
		//1: Bipolar  y值范围: -1->1
		inline void SetSourceTransform(int port, ModSourceTransformType type, int dir, int polar)
		{
			if (type == ModSourceTransformType::Func)
				return;

			sourceTransTypes[port] = type;
			this->dir[port] = dir;
			this->polar[port] = polar;
			this->inValueRange[port] = GetMapValueRange(polar);
			this->outValueRange[port] = GetMapValueRange(polar);
		}

		inline void SetSourceTransform(int port, ModTransformCallBack modTransformFunc, RangeFloat inValueRange, RangeFloat outValueRange)
		{
			sourceTransTypes[port] = ModSourceTransformType::Func;
			this->modTransformFunc[port] = modTransformFunc;
			this->inValueRange[port] = inValueRange;
			this->outValueRange[port] = outValueRange;
		}

		// 设置数量值
		inline void SetAmount(float amount)
		{
			this->amount = amount;
		}

		// 获取数量值
		inline float GetAmount()
		{
			return amount;
		}

		// 设置绝对值类型
		inline void SetAbsType(ModTransformType type)
		{
			absType = type;
		}

		// 设置输出目标到生成器类型
		void SetOutTarget(GeneratorType generatorType);

		// 设置输出目标为另一个调制器targetMod的port
		void SetOutTarget(Modulator* targetMod, int port = 0);

		//设置输出的运算方式
		inline void SetOutOp(OutOpType opType)
		{
			outTargetOp = opType;
		}

		// 获取输出的运算方式
		inline OutOpType GetOutOp()
		{
			return outTargetOp;
		}

		//设置输出值的单位变换方式
		void SetOutUnitTransform(UnitTransformCallBack cb) {
			outUnitTransform = cb;
		}


		//获取输入输出状态
		inline ModIOState GetIOState()
		{
			return ioState;
		}

		//往调制器端口输入值
		void Input(int port, float value);

		// 计算输出值   
		float Output();

		// 获取已计算的输出值
		inline float GetOutput()
		{
			return outputValue;
		}

		// 计算输出值的范围
		RangeFloat CalOutputRange();


		// 获取输出值的范围
		inline RangeFloat GetOutputRange()
		{
			return RangeFloat(outputValueMin, outputValueMax);
		}

		// 获取输出目标调制器
		inline  Modulator* GetOutTargetModulator()
		{
			return outTargetModulator;
		}

		// 获取输出目标生成器类型
		inline GeneratorType GetOutTargetGeneratorType()
		{
			return outTargetGeneratorType;
		}

		// 获取最终输出目标生成器类型
		GeneratorType GetLastOutTargetGeneratorType();

		//获取最终输出目标Modulator
		Modulator* GetLastOutTargetModulator();


		//判断是否有调制器输入
		bool HavModulatorInput() 
		{
			for (int i = 0; i < inputInfos.size(); i++)
			{
				if (inputInfos[i]->inputType == ModInputType::Modulator)
					return true;
			}
			return false;
		}

		//设置控制器调制器输入信息到列表
		void SetCtrlModulatorInputInfo(
			MidiControllerType ctrlType,
			int inputPort, float inputNativeValueMin = 0, float inputNativeValueMax = 1);

		// 设置一个已有的调制器输入信息到列表
		void SetModulatorInputInfo(
			ModInputType inputType, ModInputPreset inputPreset, MidiControllerType ctrlType,
			int inputPort, float inputNativeValueMin = 0, float inputNativeValueMax = 1);

	private:
		// 在调制器上增加一个输入的调制器inputMod到指定端口inputPort
		void AddInputModulator(Modulator* inputMod, int inputPort);

		// 移除调制器上的某个端口上的输入调制器inputMod
		void RemoveInputModInfo(Modulator* inputMod);

		// 移除端口上的所有非输入调制器的输入信息，并返回移除的输入信息组
		void RemoveAllCommonInputInfoFromPort(int port);

		// <summary>
		// 获取控制类型曲线的映射范围
		// </summary>
		inline RangeFloat GetMapValueRange(int polar)
		{
			return polar == 0 ? RangeFloat(0, 1) : RangeFloat(-1, 1);
		}

		// 映射输入值到控制类型值域范围
		float MapValueFromInputPort(int port);


	private:

		ModulatorType type = ModulatorType::Common;

		// 输出目标到生成器类型
		GeneratorType outTargetGeneratorType = GeneratorType::None;

		// 输出目标为另一个调制器
		Modulator* outTargetModulator = nullptr;

		// 输出目标到另一个调制器所在端口
		int outTargetModulatorPort = 0;

		// 输出值的绝对值处理
		ModTransformType absType = ModTransformType::Linear;

		//输出值运算方式
		OutOpType outTargetOp = OutOpType::Add;

		//输出值的单位变换
		UnitTransformCallBack outUnitTransform = nullptr; 

		// 输入信息
		vector<ModInputInfo*> inputInfos;

		// 端口控制器类型
		ModSourceTransformType sourceTransTypes[2] = { ModSourceTransformType::Linear };

		// 端口控制器回调
		ModTransformCallBack modTransformFunc[2] = { nullptr };

		RangeFloat inValueRange[2];
		RangeFloat outValueRange[2];

		//输入输出状态
		//-1:未给定输入值
		//1:已经调用input()送入了值，等待输出
		//0:已经调用了output(),可以直接获取输出值
		ModIOState ioState = ModIOState::Non;

		float inputValue[2] = { 0, 0 };
		float inputMinValue[2] = { 0, 0 };
		float inputMaxValue[2] = { 0, 0 };

		//变换线极性
		//0: Unipolar y值范围: 0->1
		//1: Bipolar  y值范围: -1->1
		int polar[2] = { 0 };

		//变换线方向
		//0: Positive minY->maxY
		//1: Negative maxY->minY
		int dir[2] = { 0 };

		// 各个端口被使用的次数
		int usePortCount[2] = { 0, 0 };

		float amount = 1;
		float outputValue = 0;
		float outputValueMin = 0;
		float outputValueMax = 0;
	};

}

#endif
