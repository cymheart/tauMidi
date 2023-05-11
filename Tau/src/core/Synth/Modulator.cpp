#include"Modulator.h"
#include"UnitTransform.h"

namespace tau
{
	Modulator::Modulator()
	{
		
	}

	Modulator::~Modulator()
	{
		for (int i = 0; i < inputInfos.size(); i++)
			delete inputInfos[i];
		inputInfos.clear();
	}

	//判断是否和modulator一样
	//不比较amount
	bool Modulator::IsSame(Modulator* modulator)
	{
		if (inputInfos.empty() || modulator->inputInfos.empty() || 
			inputInfos.size() != modulator->inputInfos.size())
			return false;
	
		//
		if (GetOutTargetModulator() != modulator->GetOutTargetModulator())
			return false;
		else if (GetOutTargetModulator() == nullptr && 
			GetOutTargetGeneratorType() != modulator->GetOutTargetGeneratorType()) {
			return false;
		}

		//
		if (sourceTransTypes[0] != modulator->sourceTransTypes[0] ||
			sourceTransTypes[1] != modulator->sourceTransTypes[1] ||
			modTransformFunc[0] != modulator->modTransformFunc[0] ||
			modTransformFunc[1] != modulator->modTransformFunc[1])
		{
			return false;
		}

		if (polar[0] != modulator->polar[0] ||
			polar[1] != modulator->polar[1] ||
			dir[0] != modulator->dir[0] ||
			dir[1] != modulator->dir[1])
		{
			return false;
		}

		for (int i = 0; i < inputInfos.size(); i++)
		{
			ModInputInfo& a = *(inputInfos[i]);
			ModInputInfo& b = *(modulator->inputInfos[i]);

			if (a.ctrlType == b.ctrlType &&
				a.inputModulator == b.inputModulator &&
				a.inputNativeValue == b.inputNativeValue &&
				a.inputNativeValueMax == b.inputNativeValueMax &&
				a.inputNativeValueMin == b.inputNativeValueMin &&
				a.inputPort == b.inputPort &&
				a.inputPreset == b.inputPreset &&
				a.inputType == b.inputType)
			{
				continue;
			}

			return false;
		}

		return true;
	}

	//<summary>
	//增加一个输入到调制器的指定端口
	//< / summary>
	//<param name = "inputType">输入类型< / param>
	//<param name = "inputPort">输入端口< / param>
	//<param name = "inputNativeValueMin">输入的原生最小值< / param>
	//<param name = "inputNativeValueMax">输入的原生最大值< / param>
	void Modulator::AddInputInfo(
		ModInputType inputType, ModInputPreset inputPreset, MidiControllerType ctrlType,
		int inputPort, float inputNativeValueMin, float inputNativeValueMax)
	{
		ModInputInfo* info = new ModInputInfo();
		info->inputType = inputType;
		info->inputPreset = inputPreset;
		info->ctrlType = ctrlType;
		info->inputPort = inputPort;
		info->inputNativeValueMin = inputNativeValueMin;
		info->inputNativeValueMax = inputNativeValueMax;
		inputInfos.push_back(info);
		usePortCount[inputPort]++;
	}


	//<summary>
   //增加一个预设输入到调制器的指定端口
   //< / summary>
   //<param name = "inputPort">输入端口< / param>
   //<param name = "inputNativeValueMin">输入的原生最小值< / param>
   //<param name = "inputNativeValueMax">输入的原生最大值< / param>
	void Modulator::AddPresetInputInfo(
		ModInputPreset inputPreset, int inputPort,
		float inputNativeValueMin, float inputNativeValueMax)
	{
		ModInputInfo* info = new ModInputInfo();
		info->inputType = ModInputType::Preset;
		info->inputPreset = inputPreset;
		info->ctrlType = MidiControllerType::CC_None;
		info->inputPort = inputPort;
		info->inputNativeValueMin = inputNativeValueMin;
		info->inputNativeValueMax = inputNativeValueMax;
		inputInfos.push_back(info);
		usePortCount[inputPort]++;
	}

	//<summary>
	//增加一个Midi控制器输入到调制器的指定端口
	//< / summary>
	//<param name = "inputType">输入类型< / param>
	//<param name = "inputPort">输入端口< / param>
	//<param name = "inputNativeValueMin">输入的原生最小值< / param>
	//<param name = "inputNativeValueMax">输入的原生最大值< / param>
	void Modulator::AddMidiCtrlInputInfo(
		MidiControllerType ctrlType, int inputPort,
		float inputNativeValueMin, float inputNativeValueMax)
	{
		ModInputInfo* info = new ModInputInfo();
		info->inputType = ModInputType::MidiController;
		info->inputPreset = ModInputPreset::NoController;
		info->ctrlType = ctrlType;
		info->inputPort = inputPort;
		info->inputNativeValueMin = inputNativeValueMin;
		info->inputNativeValueMax = inputNativeValueMax;
		inputInfos.push_back(info);
		usePortCount[inputPort]++;
	}


	//移除指定端口上的所有输入信息
	//<param name = "port">指定端口< / param>
	void Modulator::RemoveAllInputInfoFromPort(int port)
	{
		ModInputInfo* inputInfo = nullptr;
		auto it = inputInfos.begin();
		auto end = inputInfos.end();
		for (; it != end;)
		{
			inputInfo = *it;
			if (inputInfo != nullptr &&
				inputInfo->inputPort == port)
			{
				if (inputInfo->inputType == ModInputType::Modulator &&
					inputInfo->inputModulator != nullptr)
				{
					inputInfo->inputModulator->SetOutTarget(nullptr, 0);
				}
				else
				{
					usePortCount[inputInfo->inputPort]--;
				}

				DEL(inputInfo);
				it = inputInfos.erase(it);
			}
			else
			{
				it++;
			}
		}

		usePortCount[port] = 0;
	}


	//设置输出目标到生成器类型
	void Modulator::SetOutTarget(GeneratorType generatorType)
	{
		if (outTargetModulator != nullptr)
			outTargetModulator->RemoveInputModInfo(this);

		outTargetModulator = nullptr;
		outTargetGeneratorType = generatorType;
	}


	//设置输出目标为另一个调制器targetMod的port
	void Modulator::SetOutTarget(Modulator* targetMod, int port)
	{
		if (targetMod == outTargetModulator)
			return;

		if (outTargetModulator != nullptr)
			outTargetModulator->RemoveInputModInfo(this);

		outTargetModulator = targetMod;
		outTargetModulatorPort = port;
		outTargetGeneratorType = GeneratorType::None;

		if (targetMod == nullptr)
			return;

		targetMod->RemoveAllCommonInputInfoFromPort(port);
		targetMod->AddInputModulator(this, port);

	}

	//设置控制器调制器输入信息到列表
	void Modulator::SetCtrlModulatorInputInfo(
		MidiControllerType ctrlType,
		int inputPort, float inputNativeValueMin, float inputNativeValueMax)
	{
		RemoveAllInputInfoFromPort(inputPort);

		AddInputInfo(
			ModInputType::MidiController,
			ModInputPreset::NoController, ctrlType,
			inputPort,
			inputNativeValueMin,
			inputNativeValueMax);
	}

	// 设置一个已有的调制器输入信息到列表
	void Modulator::SetModulatorInputInfo(
		ModInputType inputType, ModInputPreset inputPreset, MidiControllerType ctrlType,
		int inputPort, float inputNativeValueMin, float inputNativeValueMax)
	{
		RemoveAllInputInfoFromPort(inputPort);
		AddInputInfo(inputType, inputPreset, ctrlType, inputPort, inputNativeValueMin, inputNativeValueMax);
	}

	//往调制器端口输入值
	void Modulator::Input(int port, float value)
	{
		inputInfos[port]->inputNativeValue = value;
		Modulator* lastModualtor = GetLastOutTargetModulator();
		lastModualtor->ioState = ModIOState::Inputed;
	}

	//计算输出值
	float Modulator::Output()
	{
		float mapValue[2] = { 0, 0 };
		int port;
		size_t size = inputInfos.size();
		for (int i = 0; i < size; i++)
		{
			port = inputInfos[i]->inputPort;

			if (inputInfos[i]->inputType == ModInputType::Preset &&
				inputInfos[i]->inputPreset == ModInputPreset::NoController) {
				mapValue[port] = 1;
			}
			else {
				if (inputInfos[i]->inputType == ModInputType::Modulator)
				{
					inputValue[port] = inputInfos[i]->inputModulator->Output();
					RangeFloat range = inputInfos[i]->inputModulator->CalOutputRange();
					inputMinValue[port] = range.min;
					inputMaxValue[port] = range.max;
				}
				else
				{
					inputValue[port] = inputInfos[i]->inputNativeValue;
					if (sourceTransTypes[port] != ModSourceTransformType::Func) {
						inputMinValue[port] = inputInfos[i]->inputNativeValueMin;
						inputMaxValue[port] = inputInfos[i]->inputNativeValueMax;
					}
					else
					{
						inputMinValue[port] = inValueRange[port].min;
						inputMaxValue[port] = inValueRange[port].max;
					}
				}

				if (sourceTransTypes[port] != ModSourceTransformType::Func) {
					mapValue[port] += MapValueFromInputPort(port);
				}
				else {
					RangeFloat xRange = GetMapValueRange(polar[port]);
					float a = (inputValue[port] - inputMinValue[port]) / (inputMaxValue[port] - inputMinValue[port]);
					float x = xRange.min + a * (xRange.max - xRange.min);
					mapValue[port] += modTransformFunc[port](x);
				}
			}
		}

		if (usePortCount[0] == 0) { mapValue[0] = 1; }
		if (usePortCount[1] == 0) { mapValue[1] = 1; }

		outputValue = amount * mapValue[0] * mapValue[1];

		if (outUnitTransform != nullptr)
			outputValue = outUnitTransform(outputValue);

		if (absType == ModTransformType::Absolute)
			outputValue = abs(outputValue);
		else if(absType == ModTransformType::Negative)
			outputValue = -outputValue;

		ioState = ModIOState::Ouputed;
		return outputValue;
	}



	//计算输出值的范围
	RangeFloat Modulator::CalOutputRange()
	{
		RangeFloat range0 = outValueRange[0];
		RangeFloat range1 = outValueRange[1];

		if (usePortCount[0] == 0)
		{
			range0.min = 1;
			range0.max = 1;
		}
		if (usePortCount[1] == 0)
		{
			range1.min = 1;
			range1.max = 1;
		}

		float minval = min(range0.min, range1.min);
		float maxval = max(range0.max, range1.max);

		outputValueMin = amount * minval;
		outputValueMax = amount * maxval;

		return GetOutputRange();
	}

	//获取最终输出目标生成器类型
	GeneratorType Modulator::GetLastOutTargetGeneratorType()
	{
		if (outTargetModulator != nullptr && outTargetGeneratorType == GeneratorType::None)
			return outTargetModulator->GetLastOutTargetGeneratorType();

		return outTargetGeneratorType;
	}

	//获取最终输出目标Modulator
	Modulator* Modulator::GetLastOutTargetModulator()
	{
		if (outTargetModulator != nullptr && outTargetGeneratorType == GeneratorType::None)
			return outTargetModulator->GetLastOutTargetModulator();

		return this;
	}


	//在调制器上增加一个输入的调制器inputMod到指定端口inputPort
	void Modulator::AddInputModulator(Modulator* inputMod, int inputPort)
	{
		ModInputInfo* info = new ModInputInfo();
		info->inputType = ModInputType::Modulator;
		info->inputPort = inputPort;
		info->inputModulator = inputMod;
		usePortCount[inputPort]++;
		inputInfos.push_back(info);
	}

	//移除调制器上的某个端口上的输入调制器inputMod
	//<param name = "inputMod">输入到端口的调制器< / param>
	void Modulator::RemoveInputModInfo(Modulator* inputMod)
	{
		ModInputInfo* modInputInfo = nullptr;
		auto it = inputInfos.begin();
		for (; it != inputInfos.end(); )
		{
			modInputInfo = *it;

			if (modInputInfo != nullptr &&
				modInputInfo->inputType == ModInputType::Modulator &&
				modInputInfo->inputModulator == inputMod)
			{
				usePortCount[modInputInfo->inputPort]--;
				it = inputInfos.erase(it);
				DEL(modInputInfo);
			}
			else
				it++;
		}
	}

	//移除端口上的所有非输入调制器的输入信息，并返回移除的输入信息组
	void Modulator::RemoveAllCommonInputInfoFromPort(int port)
	{
		auto it = inputInfos.begin();
		for (; it != inputInfos.end(); )
		{
			if ((*it)->inputType != ModInputType::Modulator &&
				(*it)->inputPort == port)
			{
				usePortCount[(*it)->inputPort]--;
				DEL(*it);
				it = inputInfos.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	// 映射输入值到控制类型值域范围
	float Modulator::MapValueFromInputPort(int port)
	{
		RangeFloat xRange = GetMapValueRange(polar[port]);
		float a = (inputValue[port] - inputMinValue[port]) / (inputMaxValue[port] - inputMinValue[port]);
		float x = xRange.min + a * (xRange.max - xRange.min);
		float y = 0;

		switch (sourceTransTypes[port]) {
			case ModSourceTransformType::Linear:
				if (polar[port] == 0 && dir[port] == 0)	y = x;
				else if (polar[port] == 0 && dir[port] == 1) y = -x + 1;
				else if (polar[port] == 1 && dir[port] == 0) y = x;
				else if (polar[port] == 1 && dir[port] == 1) y = -x;
				break;

			case ModSourceTransformType::Concave:
				if (polar[port] == 0 && dir[port] == 0)	y = 1 - sqrt(1 - pow(x, 2));
				else if (polar[port] == 0 && dir[port] == 1) y = 1 - sqrt(1 - pow(x-1, 2));
				else if (polar[port] == 1 && dir[port] == 0) { if (x < 0) y = -sqrt(1 - pow(x+1, 2)); else y = sqrt(1 - pow(x-1, 2)); }
				else if (polar[port] == 1 && dir[port] == 1) { if (x < 0) y = 1- sqrt(1 - pow(x, 2)); else y = sqrt(1 - pow(x, 2)) - 1; }
				break;

			case ModSourceTransformType::Convex:
				if (polar[port] == 0 && dir[port] == 0)	y = sqrt(1 - pow(x-1, 2));
				else if (polar[port] == 0 && dir[port] == 1) y = sqrt(1 - pow(x, 2));
				else if (polar[port] == 1 && dir[port] == 0) { if (x < 0) y = sqrt(1 - pow(x, 2)) - 1; else y = -sqrt(1 - pow(x, 2)) + 1; }
				else if (polar[port] == 1 && dir[port] == 1) { if (x < 0) y = sqrt(1 - pow(x+1, 2)); else y = -sqrt(1 - pow(x-1, 2)); }
				break;

			case ModSourceTransformType::Switch:
				if (polar[port] == 0 && dir[port] == 0) { if (x < 0.5f) y = 0; else y = x; }
				else if (polar[port] == 0 && dir[port] == 1) { if (x < 0.5f) y = x; else y = 0; }
				else if (polar[port] == 1 && dir[port] == 0) { if (x < 0) y = -x; else y = x; }
				else if (polar[port] == 1 && dir[port] == 1) { if (x < 0) y = x; else y = -x; }
				break;

		}

		return y;
	}
}
