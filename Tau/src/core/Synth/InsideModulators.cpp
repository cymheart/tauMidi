#include"InsideModulators.h"
#include"UnitTransform.h"

namespace tau
{
	InsideModulators::InsideModulators()
	{
		CreateInsideModulators();
	}

	InsideModulators::~InsideModulators()
	{
		CloseAllInsideModulator();

		Modulator* mod;
		Modulator* nextMod;
		for (int i = 0; i < 128; i++)
		{
			mod = insideCtrlMod[i];

			while (mod != nullptr) {
				nextMod = insideCtrlMod[i]->GetOutTargetModulator();
				DEL(mod);
				mod = nextMod;
			}

			insideCtrlMod[i] = nullptr;
		}


		//
		for (int i = 0; i < 20; i++)
		{
			mod = insidePresetMod[i];

			while (mod != nullptr) {
				nextMod = insidePresetMod[i]->GetOutTargetModulator();
				DEL(mod);
				mod = nextMod;
			}

			insidePresetMod[i] = nullptr;
		}

		//
		for (int i = 0; i < modulators.size(); i++)
			delete modulators[i];
		modulators.clear();
	}


	//生成内置调制器
	void InsideModulators::CreateInsideModulators()
	{
		CreateInsidePanModulator();
		CreateInsideCoarseTuneModulator();
		CreateInsideFineTuneModulator();
		CreateInsideVolumeModulator();
		CreateInsideSustainPedalOnOffModulator();
		CreateInsideModulationWheelModulator();
		CreateInsidePitchBendModulator();
		CreateInsidePloyPressureModulator();
		CreateInsideChordSendModulator();
		CreateInsideReverbSendModulator();
	}

	//根据指定类型启用内部控制器调制器
	void InsideModulators::OpenInsideCtrlModulator(MidiControllerType ctrlType)
	{
		//特例处理
		if (ctrlType == MidiControllerType::ExpressionControllerMSB)
			ctrlType = MidiControllerType::ChannelVolumeMSB;

		if (isUsedInsideCtrlMod[(int)ctrlType] == true)
			return;

		if (insideCtrlMod[(int)ctrlType] == nullptr)
			return;
		modulators.push_back(insideCtrlMod[(int)ctrlType]);
		isUsedInsideCtrlMod[(int)ctrlType] = true;
	}

	//根据指定类型启用内部预设调制器
	void InsideModulators::OpenInsidePresetModulator(ModInputPreset modPresetType)
	{
		if (isUsedInsidePresetMod[(int)modPresetType] == true)
			return;

		if (insidePresetMod[(int)modPresetType] == nullptr)
			return;
		modulators.push_back(insidePresetMod[(int)modPresetType]);
		isUsedInsidePresetMod[(int)modPresetType] = true;
	}


	//根据指定类型关闭内部控制器调制器
	void InsideModulators::CloseInsideCtrlModulator(MidiControllerType ctrlType)
	{
		if (modulators.empty())
			return;

		vector<Modulator*>::iterator it = modulators.begin();
		vector<Modulator*>::iterator end = modulators.end();
		for (; it != end; it++)
		{
			if (*it == insideCtrlMod[(int)ctrlType])
			{
				modulators.erase(it);
				isUsedInsideCtrlMod[(int)ctrlType] = false;
				break;
			}
		}
	}


	//根据指定类型关闭内部预设调制器
	void InsideModulators::CloseInsidePresetModulator(ModInputPreset modPresetType)
	{
		if (modulators.empty())
			return;

		vector<Modulator*>::iterator it = modulators.begin();
		vector<Modulator*>::iterator end = modulators.end();
		for (; it != end; it++)
		{
			if (*it == insidePresetMod[(int)modPresetType])
			{
				modulators.erase(it);
				isUsedInsidePresetMod[(int)modPresetType] = false;
				break;
			}
		}
	}

	//关闭所有内部调制器
	void InsideModulators::CloseAllInsideModulator()
	{
		if (modulators.empty())
			return;
		modulators.clear();
		memset(isUsedInsideCtrlMod, 0, sizeof(bool) * 128);
		memset(isUsedInsidePresetMod, 0, sizeof(bool) * 20);
	}


	//生成内部pan调制器
	void InsideModulators::CreateInsidePanModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::PanMSB, 0);
		mod->SetOutTarget(GeneratorType::Pan);
		mod->SetOutOp(OutOpType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(500);
		insideCtrlMod[(int)MidiControllerType::PanMSB] = mod;
	}


	//生成内部CoarseTune调制器(以半音为单位校正音调)
	void InsideModulators::CreateInsideCoarseTuneModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);

		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::CoarseTune, MidiControllerType::CC_None,
			0,
			0, 127);

		mod->SetOutTarget(GeneratorType::CoarseTune);
		mod->SetOutOp(OutOpType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(64);
		insidePresetMod[(int)ModInputPreset::CoarseTune] = mod;

	}

	//生成内部FineTune调制器(以音分为单位校正音调)
	void InsideModulators::CreateInsideFineTuneModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);

		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::FineTune, MidiControllerType::CC_None,
			0,
			0, 16383.0f);

		mod->SetOutTarget(GeneratorType::FineTune);
		mod->SetOutOp(OutOpType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(99);
		insidePresetMod[(int)ModInputPreset::FineTune] = mod;
	}


	//生成内部Volume调制器
	void InsideModulators::CreateInsideVolumeModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::ChannelVolumeMSB, 0);
		mod->SetOutTarget(GeneratorType::InitialAttenuation);
		mod->SetOutOp(OutOpType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAbsType(ModTransformType::Negative);
		mod->SetAmount(1);
		insideCtrlMod[(int)MidiControllerType::ChannelVolumeMSB] = mod;
	}


	//生成内部SustainPedalOnOff调制器
	void InsideModulators::CreateInsideSustainPedalOnOffModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::SustainPedalOnOff, 0);
		mod->SetOutTarget(GeneratorType::SustainPedalOnOff);
		mod->SetOutOp(OutOpType::Replace);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAmount(1);
		insideCtrlMod[(int)MidiControllerType::SustainPedalOnOff] = mod;
	}

	//生成内部ModulationWheel调制器
	void InsideModulators::CreateInsideModulationWheelModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::ModulationWheelMSB, 0);
		mod->SetOutTarget(GeneratorType::VibLfoToPitch);
		mod->SetOutOp(OutOpType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(30);
		insideCtrlMod[(int)MidiControllerType::ModulationWheelMSB] = mod;
	}


	//生成内部弯音调制器
	void InsideModulators::CreateInsidePitchBendModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::PitchWheel, MidiControllerType::CC_None,
			0,
			0, 16383.0f);

		mod->SetOutTarget(GeneratorType::CoarseTune);
		mod->SetOutOp(OutOpType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(10);
		insidePresetMod[(int)ModInputPreset::PitchWheel] = mod;
	}

	//生成内部PloyPressure调制器
	void InsideModulators::CreateInsidePloyPressureModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::PolyPressure, MidiControllerType::CC_None,
			0,
			0, 127);

		mod->SetOutTarget(GeneratorType::Pressure);
		mod->SetOutOp(OutOpType::Replace);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAmount(127);
		insidePresetMod[(int)ModInputPreset::PolyPressure] = mod;
	}


	//生成内部和声调制器
	void InsideModulators::CreateInsideChordSendModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::Effects3DepthChorusSend, 0);
		mod->SetOutTarget(GeneratorType::ChorusEffectsSend);
		mod->SetOutOp(OutOpType::Mul);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAmount(1);
		insideCtrlMod[(int)MidiControllerType::Effects3DepthChorusSend] = mod;
	}

	//生成内部混音调制器
	void InsideModulators::CreateInsideReverbSendModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::Effects1DepthReverbSend, 0);
		mod->SetOutTarget(GeneratorType::ReverbEffectsSend);
		mod->SetOutOp(OutOpType::Mul);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAmount(1);
		insideCtrlMod[(int)MidiControllerType::Effects1DepthReverbSend] = mod;
	}

}
