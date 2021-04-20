#include"ModulatorList.h"
#include"UnitTransform.h"

namespace ventrue
{
	ModulatorList::ModulatorList()
	{
		CreateInsideModulators();
	}

	ModulatorList::~ModulatorList()
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

		DEL_OBJS_VECTOR(modulators);
	}


	//生成内置调制器
	void ModulatorList::CreateInsideModulators()
	{
		CreateInsidePanModulator();
		CreateInsideCoarseTuneModulator();
		CreateInsideFineTuneModulator();
		CreateInsideVolumeModulator();
		CreateInsideSustainPedalOnOffModulator();
		CreateInsideModulationWheelModulator();
		CreateInsidePitchBendModulator();
	}

	//根据指定类型启用内部控制器调制器
	void ModulatorList::OpenInsideCtrlModulator(MidiControllerType ctrlType)
	{
		//特例处理
		if (ctrlType == MidiControllerType::ExpressionControllerMSB)
			ctrlType = MidiControllerType::ChannelVolumeMSB;

		if (isUsedInsideCtrlMod[(int)ctrlType] == true)
			return;

		AddModulator(insideCtrlMod[(int)ctrlType]);
		isUsedInsideCtrlMod[(int)ctrlType] = true;
	}

	//根据指定类型启用内部预设调制器
	void ModulatorList::OpenInsidePresetModulator(ModInputPreset modPresetType)
	{
		if (isUsedInsidePresetMod[(int)modPresetType] == true)
			return;

		AddModulator(insidePresetMod[(int)modPresetType]);
		isUsedInsidePresetMod[(int)modPresetType] = true;
	}


	//根据指定类型关闭内部控制器调制器
	void ModulatorList::CloseInsideCtrlModulator(MidiControllerType ctrlType)
	{
		ModulatorVec::iterator it = modulators->begin();
		ModulatorVec::iterator end = modulators->end();
		for (; it != end; it++)
		{
			if (*it == insideCtrlMod[(int)ctrlType])
			{
				modulators->erase(it);
				isUsedInsideCtrlMod[(int)ctrlType] = false;
				break;
			}
		}
	}


	//根据指定类型关闭内部预设调制器
	void ModulatorList::CloseInsidePresetModulator(ModInputPreset modPresetType)
	{
		ModulatorVec::iterator it = modulators->begin();
		ModulatorVec::iterator end = modulators->end();
		for (; it != end; it++)
		{
			if (*it == insidePresetMod[(int)modPresetType])
			{
				modulators->erase(it);
				isUsedInsidePresetMod[(int)modPresetType] = false;
				break;
			}
		}
	}

	//关闭所有内部调制器
	void ModulatorList::CloseAllInsideModulator()
	{
		if (modulators != nullptr)
		{
			ModulatorVec::iterator it = modulators->begin();
			for (; it != modulators->end();)
			{
				if ((*it)->GetType() == ModulatorType::Inside)
					it = modulators->erase(it);
				else
					it++;
			}
		}

		memset(isUsedInsideCtrlMod, 0, sizeof(bool) * 128);
		memset(isUsedInsidePresetMod, 0, sizeof(bool) * 20);
	}


	//生成内部pan调制器
	void ModulatorList::CreateInsidePanModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::PanMSB, 0);
		mod->SetOutTarget(GeneratorType::Pan);
		mod->SetOutModulationType(ModulationType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(500);
		insideCtrlMod[(int)MidiControllerType::PanMSB] = mod;
	}


	//生成内部CoarseTune调制器(以半音为单位校正音调)
	void ModulatorList::CreateInsideCoarseTuneModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);

		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::CoarseTune, MidiControllerType::CC_None,
			0,
			0,
			1);

		mod->SetOutTarget(GeneratorType::CoarseTune);
		mod->SetOutModulationType(ModulationType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(64);
		insidePresetMod[(int)ModInputPreset::CoarseTune] = mod;

	}

	//生成内部FineTune调制器(以音分为单位校正音调)
	void ModulatorList::CreateInsideFineTuneModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);

		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::FineTune, MidiControllerType::CC_None,
			0,
			0,
			32639);

		mod->SetOutTarget(GeneratorType::FineTune);
		mod->SetOutModulationType(ModulationType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(100);
		insidePresetMod[(int)ModInputPreset::FineTune] = mod;
	}


	//生成内部Volume调制器
	void ModulatorList::CreateInsideVolumeModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::ChannelVolumeMSB, 0);
		mod->SetOutTarget(GeneratorType::InitialAttenuation);
		mod->SetOutModulationType(ModulationType::Add);
		mod->SetSourceTransform(0, VolGainTans, RangeFloat(0, 1), RangeFloat(-144, 0));
		mod->SetAmount(1);
		insideCtrlMod[(int)MidiControllerType::ChannelVolumeMSB] = mod;
	}


	//生成内部SustainPedalOnOff调制器
	void ModulatorList::CreateInsideSustainPedalOnOffModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::SustainPedalOnOff, 0);
		mod->SetOutTarget(GeneratorType::SustainPedalOnOff);
		mod->SetOutModulationType(ModulationType::Replace);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAmount(1);
		insideCtrlMod[(int)MidiControllerType::SustainPedalOnOff] = mod;
	}

	//生成内部ModulationWheel调制器
	void ModulatorList::CreateInsideModulationWheelModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);
		mod->SetCtrlModulatorInputInfo(MidiControllerType::ModulationWheelMSB, 0);
		mod->SetOutTarget(GeneratorType::VibLfoToPitch);
		mod->SetOutModulationType(ModulationType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 0);
		mod->SetAmount(50);
		insideCtrlMod[(int)MidiControllerType::ModulationWheelMSB] = mod;
	}


	//生成内部滑音调制器
	void ModulatorList::CreateInsidePitchBendModulator()
	{
		Modulator* mod = new Modulator();
		mod->SetType(ModulatorType::Inside);

		mod->RemoveAllInputInfoFromPort(0);

		mod->AddInputInfo(
			ModInputType::Preset,
			ModInputPreset::PitchWheel, MidiControllerType::CC_None,
			0,
			0,
			16383.0f);

		mod->SetOutTarget(GeneratorType::CoarseTune);
		mod->SetOutModulationType(ModulationType::Add);
		mod->SetSourceTransform(0, ModSourceTransformType::Linear, 0, 1);
		mod->SetAmount(2);
		insidePresetMod[(int)ModInputPreset::PitchWheel] = mod;
	}

	float ModulatorList::VolGainTans(float gain)
	{
		return UnitTransform::GainToDecibels(gain);
	}

	// 新增一个调制器
	// <returns>返回新增的调制器</returns>
	void ModulatorList::AddModulator(Modulator* mod)
	{
		if (mod == nullptr)
			return;

		if (modulators == nullptr)
			modulators = new ModulatorVec;
		modulators->push_back(mod);
	}

}
