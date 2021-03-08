#include"RegionModulation.h"
#include"UnitTransform.h"
#include"Generator.h"
#include"Region.h"
#include"Channel.h"
#include"Modulator.h"
namespace ventrue
{
	RegionModulation::RegionModulation()
	{
		presetCombGenList = new GeneratorList();
		presetCombGenList->SetType(RegionType::Preset);

		presetRegionModGenList = new GeneratorList();
		presetGlobalRegionModGenList = new GeneratorList();

		instCombGenList = new GeneratorList();
		instCombGenList->SetType(RegionType::Insttrument);

		instRegionModGenList = new GeneratorList();
		instGlobalRegionModGenList = new GeneratorList();

		modsModGenList = new GeneratorList();
	}

	RegionModulation::~RegionModulation()
	{
		DEL(presetRegionModGenList);
		DEL(presetGlobalRegionModGenList);
		DEL(instRegionModGenList);
		DEL(instGlobalRegionModGenList);
		DEL(modsModGenList);
		DEL(instCombGenList);
		DEL(presetCombGenList);
	}

	void RegionModulation::Clear()
	{
		insideCtrlModulatorList = nullptr;
		genList = nullptr;
		outGenList = nullptr;
		modifyedGenTypes = nullptr;
		channel = nullptr;
		instRegion = nullptr;
		instGlobalRegion = nullptr;
		presetRegion = nullptr;
		presetGlobalRegion = nullptr;
		modifyedGenList = nullptr;
		isInit = false;

	}

	//调制
	void RegionModulation::Modulation()
	{
		if (!isInit)
		{
			InitGenList();
			isInit = true;
		}
		else
		{
			ModGenList();
		}
	}

	// 首次生成乐器调制后的GeneratorList
	void RegionModulation::InitGenList()
	{
		//复合区域的乐器生成器列表和乐器全局生成器列表，预设生成器列表和预设全局生成器列表
		//获取最终的修改生成器列表
		CombInstGenList();
		CombPresetGenList();
		CombInstAndPresetGenList(false);

		//
		ModGenList();
		InitNeedModifyGenParamTypes();
	}


	//复合同一区域的乐器生成列表(instRegion)和乐器全局生成器列表(instGlobalRegion)值到instCombGenList中
	//复合方法: 优先使用instRegion中的值，如果没有将使用instGlobalRegion的值，
	//如果此项在两个列表中都查不到，那将删除instCombGenList中对应项
	//VelRange, KeyRange,这两项不参与复合处理，直接写入最终的modifyedGenList对应项中
	//这个复合处理在regionSounder建立后只需要处理一次，将会直到regionSounder发声结束一直缓存这组值
	void RegionModulation::CombInstGenList()
	{
		GeneratorList* instList = instRegion->GetGenList();
		GeneratorList* instGlobalList = instGlobalRegion->GetGenList();

		for (int i = 0; i < (int)GeneratorType::EndOper; i++)
		{
			GeneratorType type = (GeneratorType)i;

			switch (type)
			{
			case GeneratorType::KeyRange:
			case GeneratorType::VelRange:
				if (!instList->IsEmpty(type)) {
					RangeFloat rangeAmount = instList->GetAmountRange(type);
					modifyedGenList->SetAmountRange(GeneratorType::KeyRange, rangeAmount.min, rangeAmount.max);
				}
				else if (!instGlobalList->IsEmpty(type)) {
					RangeFloat rangeAmount = instGlobalList->GetAmountRange(type);
					modifyedGenList->SetAmountRange(GeneratorType::KeyRange, rangeAmount.min, rangeAmount.max);
				}
				break;

			default:
				if (!instList->IsEmpty(type))
					instCombGenList->SetAmount(type, instList->GetAmount(type));
				else if (!instGlobalList->IsEmpty(type))
					instCombGenList->SetAmount(type, instGlobalList->GetAmount(type));
				else
					instCombGenList->Remove(type);

				break;
			}

		}
	}

	//复合同一区域的预设生成列表(presetRegion)和预设全局生成器列表(presetGlobalRegion)值到presetCombGenList
	//复合方法: 优先使用presetRegion中的值，如果没有将使用presetGlobalRegion的值，
	//如果此项在两个列表中都查不到，那将删除instCombGenList中对应项
	//VelRange, KeyRange,SustainPedalOnOff不参与复合处理
	//这个复合处理在regionSounder建立后只需要处理一次，将会直到regionSounder发声结束一直缓存这组值
	void RegionModulation::CombPresetGenList()
	{
		GeneratorList* presetList = presetRegion->GetGenList();
		GeneratorList* presetGlobalList = presetGlobalRegion->GetGenList();

		for (int i = 0; i < (int)GeneratorType::EndOper; i++)
		{
			GeneratorType type = (GeneratorType)i;
			switch (type)
			{
			case GeneratorType::KeyRange:
			case GeneratorType::VelRange:
			case GeneratorType::SustainPedalOnOff:
				break;

			default:
				if (!presetList->IsEmpty(type))
					presetCombGenList->SetAmount(type, presetList->GetAmount(type));
				else if (!presetGlobalList->IsEmpty(type))
					presetCombGenList->SetAmount(type, presetGlobalList->GetAmount(type));
				else
					presetCombGenList->Remove(type);
				break;
			}
		}
	}

	//复合算得的乐器生成器列表(instCombGenList)和算的预设生成器列表(presetCombGenList)
	void RegionModulation::CombInstAndPresetGenList(bool isTestModFlag)
	{
		for (int i = 0; i < (int)GeneratorType::EndOper; i++)
		{
			switch ((GeneratorType)i)
			{
			case GeneratorType::KeyRange:
			case GeneratorType::VelRange:
			case GeneratorType::Keynum:
			case GeneratorType::Velocity:
			case GeneratorType::ExclusiveClass:
			case GeneratorType::EndAddrsCoarseOffset:
			case GeneratorType::EndAddrsOffset:
			case GeneratorType::EndloopAddrsCoarseOffset:
			case GeneratorType::EndloopAddrsOffset:
			case GeneratorType::StartAddrsOffset:
			case GeneratorType::StartAddrsCoarseOffset:
			case GeneratorType::StartloopAddrsCoarseOffset:
			case GeneratorType::StartloopAddrsOffset:
				continue;
			}

			CombGenValue((GeneratorType)i, isTestModFlag);

			if (isTestModFlag && isModedInstGenTypes[i])
			{
				modifyedGenTypes->insert(i);
			}
		}
	}

	void RegionModulation::CombGenValue(GeneratorType genType, bool isTestModFlag)
	{
		float instValue = 0;
		float presetValue = 0;
		bool isHavPresetValue = false;

		if (isTestModFlag && isModedInstGenTypes[(int)genType]) { instValue = instRegionModGenList->GetAmount(genType); }
		else { instValue = instCombGenList->GetAmount(genType); }

		if (isTestModFlag && isModedPresetGenTypes[(int)genType]) { presetValue = presetRegionModGenList->GetAmount(genType); isHavPresetValue = true; }
		else if (!presetCombGenList->IsEmpty(genType)) { presetValue = presetCombGenList->GetAmount(genType); isHavPresetValue = true; }

		if (isHavPresetValue)
		{
			switch (genType)
			{
			case GeneratorType::InitialFilterFc:
			case GeneratorType::DelayVolEnv:
			case GeneratorType::AttackVolEnv:
			case GeneratorType::HoldVolEnv:
			case GeneratorType::DecayVolEnv:
			case GeneratorType::ReleaseVolEnv:
			case GeneratorType::DelayModEnv:
			case GeneratorType::AttackModEnv:
			case GeneratorType::HoldModEnv:
			case GeneratorType::DecayModEnv:
			case GeneratorType::ReleaseModEnv:
			case GeneratorType::DelayVibLFO:
			case GeneratorType::FreqVibLFO:
			case GeneratorType::DelayModLFO:
			case GeneratorType::FreqModLFO:
			case GeneratorType::SustainVolEnv:
			case GeneratorType::SustainModEnv:
				instValue *= presetValue;   //在分或分贝状态下做加法，log转到pow后做乘法
				break;

			case GeneratorType::SustainPedalOnOff:
				break;

			case GeneratorType::ChorusEffectsSend:
			case GeneratorType::ReverbEffectsSend:
				instValue = presetValue;
				break;

			default:
				instValue += presetValue; //在分或分贝状态下做加法
				break;
			}
		}

		float oldInstValue = modifyedGenList->GetAmount(genType);
		if (abs(oldInstValue - instValue) < 0.0001f) {
			isModedInstGenTypes[(int)genType] = false;
		}
		else {
			modifyedGenList->SetAmount(genType, instValue);
			isModedInstGenTypes[(int)genType] = true;
		}

	}


	//根据初始化时最终复合计算出的生成器列表，设置需要修改的参数类型
	void RegionModulation::InitNeedModifyGenParamTypes()
	{
		modifyedGenTypes->insert((int)GeneratorType::SampleModes);
		modifyedGenTypes->insert((int)GeneratorType::ScaleTuning);
		modifyedGenTypes->insert((int)GeneratorType::InitialAttenuation);
		modifyedGenTypes->insert((int)GeneratorType::Pan);
		modifyedGenTypes->insert((int)GeneratorType::StartAddrsOffset);
		modifyedGenTypes->insert((int)GeneratorType::EndAddrsOffset);
		modifyedGenTypes->insert((int)GeneratorType::StartloopAddrsOffset);
		modifyedGenTypes->insert((int)GeneratorType::EndloopAddrsOffset);
		modifyedGenTypes->insert((int)GeneratorType::SustainPedalOnOff);

		if (!modifyedGenList->IsEmpty(GeneratorType::VibLfoToPitch))
		{
			modifyedGenTypes->insert((int)GeneratorType::VibLfoToPitch);
			modifyedGenTypes->insert((int)GeneratorType::DelayVibLFO);
			modifyedGenTypes->insert((int)GeneratorType::FreqVibLFO);
		}

		if (!modifyedGenList->IsEmpty(GeneratorType::ModLfoToFilterFc))
			modifyedGenTypes->insert((int)GeneratorType::ModLfoToFilterFc);

		if (!modifyedGenList->IsEmpty(GeneratorType::ModLfoToPitch))
			modifyedGenTypes->insert((int)GeneratorType::ModLfoToPitch);

		if (!modifyedGenList->IsEmpty(GeneratorType::ModLfoToVolume))
			modifyedGenTypes->insert((int)GeneratorType::ModLfoToVolume);


		modifyedGenTypes->insert((int)GeneratorType::DelayModLFO);
		modifyedGenTypes->insert((int)GeneratorType::FreqModLFO);


		bool isActiveModEnv = false;
		if (!modifyedGenList->IsEmpty(GeneratorType::ModEnvToPitch)) {
			modifyedGenTypes->insert((int)GeneratorType::ModEnvToPitch);
			isActiveModEnv = true;
		}

		if (!modifyedGenList->IsEmpty(GeneratorType::ModEnvToFilterFc)) {
			modifyedGenTypes->insert((int)GeneratorType::ModEnvToFilterFc);
			isActiveModEnv = true;
		}

		modifyedGenTypes->insert((int)GeneratorType::AttackVolEnv);

		if (isActiveModEnv) {
			modifyedGenTypes->insert((int)GeneratorType::AttackModEnv);
		}

		if (!modifyedGenList->IsEmpty(GeneratorType::InitialFilterFc) ||
			!modifyedGenList->IsEmpty(GeneratorType::InitialFilterQ))
		{
			modifyedGenTypes->insert((int)GeneratorType::InitialFilterFc);
		}
	}

	//调制发声区域的所有生成器值,生成最终的修改值
	void RegionModulation::ModGenList()
	{
		modifyedGenTypes->clear();
		memset(isModedInstGenTypes, 0, sizeof(bool) * 64);
		memset(isModedPresetGenTypes, 0, sizeof(bool) * 64);

		//首先使用区域调制器和区域内部调制器组成的调制项，来调制区域的instCombGenList
		//提前进行两次调制器设置，这样这两种调制器可以叠加处理同一个生成器项
		SetGenListMods(instRegion->GetModulators(), instCombGenList, instRegionModGenList, isModedInstGenTypes);
		ExecuteGenListMods(instRegion->GetModulators(), instRegionModGenList, isModedInstGenTypes);

		//接着使用乐器全局区域的调制器列表来继续调制instCombGenList
		//此时如果复合乐器生成器项被前面处理过了，将会通过isModedInstGenTypes标志过滤，而不再进行处理
		SetGenListMods(instGlobalRegion->GetModulators(), instCombGenList, instRegionModGenList, isModedInstGenTypes);
		ExecuteGenListMods(instGlobalRegion->GetModulators(), instRegionModGenList, isModedInstGenTypes);

		//
		SetGenListMods(presetRegion->GetModulators(), presetCombGenList, presetRegionModGenList, isModedPresetGenTypes);
		ExecuteGenListMods(presetRegion->GetModulators(), presetRegionModGenList, isModedPresetGenTypes);

		//接着使用预设全局区域的调制器列表来继续调制presetCombGenList
		//此时如果复合预设生成器项被前面处理过了，将会通过isModedPresetGenTypes标志过滤，而不再进行处理
		SetGenListMods(presetGlobalRegion->GetModulators(), presetCombGenList, presetRegionModGenList, isModedPresetGenTypes);
		ExecuteGenListMods(presetGlobalRegion->GetModulators(), presetRegionModGenList, isModedPresetGenTypes);

		//最后再复合乐器和预设生成器的对应项，最终结果写入modifyedGenList
		CombInstAndPresetGenList();


		//内部控制器调制
		SetGenListMods(insideCtrlModulatorList->GetModulators(), modifyedGenList, modifyedGenList);
		ExecuteGenListMods(insideCtrlModulatorList->GetModulators(), modifyedGenList, isModedInstGenTypes, true);
	}


	/// <summary>
	/// 设置区域的生成器列表项的调制器
	/// </summary>
	/// <param name="mods">用于调制输入生成器的调制器</param>
	/// <param name="inGenlist">输入的生成器列表</param>
	/// <param name="outModGenlist">调制后的对应输出的生成器Genlist</param>
	/// <param name="isAlreadyModedGenTypes">标志对应生成器是否被调制过的flag值</param>
	void RegionModulation::SetGenListMods(
		ModulatorVec* mods, GeneratorList* inGenlist,
		GeneratorList* outModGenlist, bool* isAlreadyModedGenTypes)
	{
		if (mods == nullptr)
			return;


		//循环遍历每个调制器的输入项，排除掉非调制Gen的调制器(有的调制器的调制目标时另一个调制器)
		//对于输入项是外部Midi控制器或内部预设输入的进行值输入操作
		//对于要调制的Gen项的存储变量处理
		//以备后面对此项作多次调制时，进行累加
		GeneratorType targetGenType = GeneratorType::None;
		size_t size = mods->size();
		for (int i = 0; i < size; i++)
		{
			targetGenType = (*mods)[i]->GetOutTargetGeneratorType();
			if (targetGenType != GeneratorType::None &&
				isAlreadyModedGenTypes != nullptr &&
				isAlreadyModedGenTypes[(int)targetGenType] == true)
			{
				continue;
			}

			//初始化要调制的值为初始输入值
			if (outGenList != inGenlist)
				outModGenlist->SetAmount(targetGenType, inGenlist->GetAmount(targetGenType));

			//当targetGenType == GeneratorType::None时，此时的调制器调制目标是另一调制器
			//此时也是可以通过测试，使用下面的处理来处理数据的链式结果
			Modulator& mod = *(*mods)[i];
			size_t portCount = mod.GetInputPortCount();
			for (int j = 0; j < portCount; j++)
			{
				if (mod.GetInputType(j) == ModInputType::MidiController)
				{
					//midi控制值通过通道channel获取
					mod.Input(j, channel->GetControllerComputedValue(mod.GetInputCtrlType(j)));
				}
				else if (mod.GetInputType(j) == ModInputType::Preset)
				{
					mod.Input(j, channel->GetModPresetValue(mod.GetInputPresetType(j)));
				}
				else
				{
					//mod.Input(j, mod.GetInputValue(j));
				}
			}
		}
	}

	//执行区域的Gen项mods调制
	void RegionModulation::ExecuteGenListMods(
		ModulatorVec* mods, GeneratorList* modGenlist, bool* isAlreadyModedGenTypes, bool isCheckMod)
	{
		if (mods == nullptr || isAlreadyModedGenTypes == nullptr)
			return;

		//
		SetGenList(modGenlist);
		GeneratorType targetGenType = GeneratorType::None;

		size_t size = mods->size();
		float modValue;
		for (int i = 0; i < size; i++)
		{
			Modulator& mod = *(*mods)[i];
			targetGenType = mod.GetOutTargetGeneratorType();

			if (targetGenType == GeneratorType::None ||
				mod.GetIOState() != ModIOState::Inputed)
			{
				continue;
			}

			//获取调制值，执行调制
			modValue = mod.Output();

			isAlreadyModedGenTypes[(int)targetGenType] =
				Process(targetGenType, modValue, mod.GetOutModulationType());

			if (isCheckMod)
				modifyedGenTypes->insert((int)targetGenType);
		}
	}


	// 指定生成器类型调制器调制处理
	// <param name="targetGenType"></param>
	// <param name="modValue"></param>
	bool RegionModulation::Process(GeneratorType targetGenType, float modValue, ModulationType modType)
	{
		bool ret = false;
		switch (targetGenType)
		{
		case GeneratorType::CoarseTune: ret = ModulationCoarseTune(modValue, modType); break;
		case GeneratorType::FineTune: ret = ModulationFineTune(modValue, modType); break;
		case GeneratorType::ScaleTuning:ret = ModulationScaleTuning(modValue, modType); break;
		case GeneratorType::OverridingRootKey:ret = ModulationOverridingRootKey(modValue, modType); break;
		case GeneratorType::InitialAttenuation:ret = ModulationAttenuation(modValue, modType); break;
		case GeneratorType::InitialFilterFc:ret = ModulationFc(modValue, modType); break;
		case GeneratorType::InitialFilterQ:ret = ModulationResonance(modValue, modType); break;
		case GeneratorType::Pan: ret = ModulationPan(modValue, modType); break;
		case GeneratorType::SampleModes:ret = ModulationLoopPlayback(modValue, modType); break;
		case GeneratorType::ExclusiveClass:ret = ModulationExclusiveClass(modValue, modType); break;
		case GeneratorType::DelayVibLFO:ret = ModulationVibLfoDelay(modValue, modType); break;
		case GeneratorType::FreqVibLFO:ret = ModulationVibLfoFreq(modValue, modType); break;
		case GeneratorType::VibLfoToPitch: ret = ModulationVibLfoModPitch(modValue, modType); break;
		case GeneratorType::DelayModLFO:ret = ModulationModLfoDelay(modValue, modType); break;
		case GeneratorType::FreqModLFO: ret = ModulationModLfoFreq(modValue, modType); break;
		case GeneratorType::ModLfoToPitch: ret = ModulationModLfoModPitch(modValue, modType); break;
		case GeneratorType::ModLfoToFilterFc: ret = ModulationModLfoModFc(modValue, modType); break;
		case GeneratorType::ModLfoToVolume: ret = ModulationModLfoModVolume(modValue, modType); break;
		case GeneratorType::DelayVolEnv: ret = ModulationVolumeEnvDelaySec(modValue, modType); break;
		case GeneratorType::AttackVolEnv: ret = ModulationVolumeEnvAttackSec(modValue, modType); break;
		case GeneratorType::HoldVolEnv: ret = ModulationVolumeEnvHoldSec(modValue, modType); break;
		case GeneratorType::DecayVolEnv: ret = ModulationVolumeEnvDecaySec(modValue, modType); break;
		case GeneratorType::ReleaseVolEnv: ret = ModulationVolumeEnvReleaseSec(modValue, modType); break;
		case GeneratorType::SustainVolEnv: ret = ModulationVolumeEnvSustainMul(modValue, modType); break;
		case GeneratorType::KeynumToVolEnvHold: ret = ModulationVolumeEnvKeyToHold(modValue, modType); break;
		case GeneratorType::KeynumToVolEnvDecay: ret = ModulationVolumeEnvKeyToDecay(modValue, modType); break;
		case GeneratorType::DelayModEnv: ret = ModulationModEnvDelaySec(modValue, modType); break;
		case GeneratorType::AttackModEnv: ret = ModulationModEnvAttackSec(modValue, modType); break;
		case GeneratorType::HoldModEnv: ret = ModulationModEnvHoldSec(modValue, modType); break;
		case GeneratorType::DecayModEnv: ret = ModulationModEnvDecaySec(modValue, modType); break;
		case GeneratorType::ReleaseModEnv: ret = ModulationModEnvReleaseSec(modValue, modType); break;
		case GeneratorType::SustainModEnv: ret = ModulationModEnvSustainMul(modValue, modType); break;
		case GeneratorType::KeynumToModEnvHold: ret = ModulationModEnvKeyToHold(modValue, modType); break;
		case GeneratorType::KeynumToModEnvDecay: ret = ModulationModEnvKeyToDecay(modValue, modType); break;
		case GeneratorType::ModEnvToPitch: ret = ModulationModEnvModPitch(modValue, modType); break;
		case GeneratorType::ModEnvToFilterFc: ret = ModulationModEnvModFc(modValue, modType); break;
		case GeneratorType::StartAddrsOffset: ret = ModulationStartAddrsOffset(modValue, modType); break;
		case GeneratorType::StartAddrsCoarseOffset: ret = ModulationStartAddrsCoarseOffset(modValue, modType); break;
		case GeneratorType::EndAddrsOffset: ret = ModulationEndAddrsOffset(modValue, modType); break;
		case GeneratorType::EndAddrsCoarseOffset: ret = ModulationEndAddrsCoarseOffset(modValue, modType); break;
		case GeneratorType::StartloopAddrsOffset: ret = ModulationStartloopAddrsOffset(modValue, modType); break;
		case GeneratorType::StartloopAddrsCoarseOffset: ret = ModulationStartloopAddrsCoarseOffset(modValue, modType); break;
		case GeneratorType::EndloopAddrsOffset: ret = ModulationEndloopAddrsOffset(modValue, modType); break;
		case GeneratorType::EndloopAddrsCoarseOffset: ret = ModulationEndloopAddrsCoarseOffset(modValue, modType); break;
		case GeneratorType::SustainPedalOnOff: ret = ModulationSustainPedalOnOff(modValue, modType); break;
		}

		return ret;
	}


	// 调制器调制校音(半音)
	bool RegionModulation::ModulationCoarseTune(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::CoarseTune, modValue, modType);
	}


	// 调制器调制校音(音分)
	bool RegionModulation::ModulationFineTune(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::FineTune, modValue, modType);
	}


	// 调制器调制音阶
	bool RegionModulation::ModulationScaleTuning(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::ScaleTuning, modValue, modType);
	}


	// 调制器调制根音符
	bool RegionModulation::ModulationOverridingRootKey(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::OverridingRootKey, modValue, modType);
	}


	// 调制器调制衰减
	// 单位:dB
	bool RegionModulation::ModulationAttenuation(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::InitialAttenuation, modValue, modType);
	}


	// 调制器调制滤波截至频率
	// 单位：mul
	bool RegionModulation::ModulationFc(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::InitialFilterFc, mul, modType);
	}


	// 调制器调制滤波共振峰
	// 单位：dB
	// Q = 10^(resonanceDB/20)
	bool RegionModulation::ModulationResonance(float resonanceDB, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = (float)pow(10, resonanceDB * 0.1f / 20.0f);
		return ModulationGenType(GeneratorType::InitialFilterQ, mul, modType);
	}


	// 调制器调制音量平衡
	bool RegionModulation::ModulationPan(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		modValue *= 0.1f;
		return ModulationGenType(GeneratorType::Pan, modValue, modType);
	}


	// 调制器调制循环播放模式
	bool RegionModulation::ModulationLoopPlayback(float modValue, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		LoopPlayBackMode loopPlayBack;
		int modeValue = (int)modValue;
		if (modeValue > 2) loopPlayBack = LoopPlayBackMode::LoopEndContinue;
		else if (modeValue < 0) loopPlayBack = LoopPlayBackMode::NonLoop;
		else loopPlayBack = (LoopPlayBackMode)modValue;

		return ModulationGenType(GeneratorType::SampleModes, (float)loopPlayBack, modType);
	}


	// 调制器调制独占发音归类
	bool RegionModulation::ModulationExclusiveClass(float value, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::ExclusiveClass, value, modType);
	}



	// 调制器调制vibLfo的延迟时长
	bool RegionModulation::ModulationVibLfoDelay(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::DelayVibLFO, mul, modType);
	}


	// 调制器调制vibLfo的频率
	// cent转HzMul : 8.176 * 2^(cent/1200)
	bool RegionModulation::ModulationVibLfoFreq(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToHertz(cents);
		return ModulationGenType(GeneratorType::FreqVibLFO, mul, modType);
	}


	// 调制器调制vibLfo的pitch
	bool RegionModulation::ModulationVibLfoModPitch(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::VibLfoToPitch, cents, modType);
	}



	// 调制器调制ModLfo的延迟时长
	bool RegionModulation::ModulationModLfoDelay(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::TimecentsToSecsf(cents);
		return ModulationGenType(GeneratorType::DelayModLFO, mul, modType);
	}


	// 调制器调制ModLfo的频率
	// cent转HzMul : 8.176 * 2^(cent/1200)
	bool RegionModulation::ModulationModLfoFreq(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToHertz(cents);
		return ModulationGenType(GeneratorType::FreqModLFO, mul, modType);
	}


	// 调制器调制ModLfo的pitch
	bool RegionModulation::ModulationModLfoModPitch(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::ModLfoToPitch, cents, modType);
	}


	// 调制器调制ModLfo的fc
	bool RegionModulation::ModulationModLfoModFc(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::ModLfoToFilterFc, cents, modType);
	}


	// 调制器调制ModLfo的volume
	bool RegionModulation::ModulationModLfoModVolume(float volDbGain, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::ModLfoToVolume, volDbGain / 10.0f, modType);
	}


	// 调制器调制音量包络的延迟时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationVolumeEnvDelaySec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::DelayVolEnv, mul, modType);
	}


	// 调制器调制音量包络的起音时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationVolumeEnvAttackSec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::AttackVolEnv, mul, modType);
	}


	// 调制器调制音量包络的保持时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationVolumeEnvHoldSec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::HoldVolEnv, mul, modType);
	}


	// 调制器调制音量包络的衰减时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationVolumeEnvDecaySec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::DecayVolEnv, mul, modType);
	}


	// 调制器调制音量包络的释音时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationVolumeEnvReleaseSec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::ReleaseVolEnv, mul, modType);
	}


	// 调制器调制音量包络的延音分贝
	// sustainMul值域:[0, 1]
	// 可以从db[0, 144]线性转换为[0,1]，db/144 
	bool RegionModulation::ModulationVolumeEnvSustainMul(float db, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		float val = db * 0.1f / 144.0f;
		return ModulationGenType(GeneratorType::SustainVolEnv, val, modType);
	}


	// 调制器调制音量包络的keyToHold
	bool RegionModulation::ModulationVolumeEnvKeyToHold(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::KeynumToVolEnvHold, cents / 100.0f, modType);
	}


	// 调制器调制音量包络的keyToDecay
	bool RegionModulation::ModulationVolumeEnvKeyToDecay(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::KeynumToVolEnvDecay, cents / 100.0f, modType);
	}



	// 调制器调制Mod包络的延迟时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationModEnvDelaySec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::DelayModEnv, mul, modType);
	}


	// 调制器调制mod包络的起音时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationModEnvAttackSec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::AttackModEnv, mul, modType);
	}


	// 调制器调制mod包络的保持时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationModEnvHoldSec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::HoldModEnv, mul, modType);
	}


	// 调制器调制mod包络的衰减时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationModEnvDecaySec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::DecayModEnv, mul, modType);
	}


	// 调制器调制mod包络的释音时长
	// <param name="cents">以音分为单位</param>
	bool RegionModulation::ModulationModEnvReleaseSec(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float mul = UnitTransform::CentsToMul(cents);
		return ModulationGenType(GeneratorType::ReleaseModEnv, mul, modType);
	}


	// 调制器调制mod包络的延音分贝
	// sustainMul值域:[0, 1]
	// 可以从[0, 100]线性转换为[0,1]，db/100
	bool RegionModulation::ModulationModEnvSustainMul(float value, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		float val = value * 0.1f / 100.0f;
		return ModulationGenType(GeneratorType::SustainModEnv, val, modType);
	}


	// 调制器调制mod包络的keyToHold
	bool RegionModulation::ModulationModEnvKeyToHold(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		cents /= 100.0f;
		return ModulationGenType(GeneratorType::KeynumToModEnvHold, cents, modType);
	}


	// 调制器调制mod包络的keyToDecay
	bool RegionModulation::ModulationModEnvKeyToDecay(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		cents /= 100.0f;
		return ModulationGenType(GeneratorType::KeynumToModEnvDecay, cents, modType);
	}


	// 调制器调制音mod包络调制Pitch,单位:音分
	bool RegionModulation::ModulationModEnvModPitch(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::ModEnvToPitch, cents, modType);
	}


	// 调制器调制音mod包络调制截至频率,单位:音分
	//  cents转倍率: pitchMul = 1.00057779^cents 倍
	bool RegionModulation::ModulationModEnvModFc(float cents, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Mul;

		float modMul = (float)pow(1.00057779, cents);
		return ModulationGenType(GeneratorType::ModEnvToFilterFc, modMul, modType);
	}

	// 调制器调制StartAddrsOffset
	bool RegionModulation::ModulationStartAddrsOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::StartAddrsOffset, offset, modType);
	}


	// 调制器调制StartAddrsCoarseOffset
	// 粗粒度调制
	bool RegionModulation::ModulationStartAddrsCoarseOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::StartAddrsCoarseOffset, offset, modType);
	}


	// 调制器调制EndAddrsOffset
	bool RegionModulation::ModulationEndAddrsOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::EndAddrsOffset, offset, modType);
	}


	// 调制器调制EndAddrsCoarseOffset
	// 粗粒度调制
	bool RegionModulation::ModulationEndAddrsCoarseOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::EndAddrsCoarseOffset, offset, modType);
	}



	// 调制器调制StartloopAddrsOffset
	bool RegionModulation::ModulationStartloopAddrsOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::StartloopAddrsOffset, offset, modType);
	}


	// 调制器调制StartloopAddrsCoarseOffset
	// 粗粒度调制
	bool RegionModulation::ModulationStartloopAddrsCoarseOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::StartloopAddrsCoarseOffset, offset, modType);
	}


	// 调制器调制EndloopAddrsOffset
	bool RegionModulation::ModulationEndloopAddrsOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::EndloopAddrsOffset, offset, modType);
	}


	// 调制器调制EndloopAddrsCoarseOffset
	// 粗粒度调制
	bool RegionModulation::ModulationEndloopAddrsCoarseOffset(float offset, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Add;

		return ModulationGenType(GeneratorType::EndloopAddrsCoarseOffset, offset, modType);
	}

	// 调制器调制SustainPedalOnOff
	bool  RegionModulation::ModulationSustainPedalOnOff(float onoff, ModulationType modType)
	{
		if (modType == ModulationType::Unknown)
			modType = ModulationType::Replace;

		return ModulationGenType(GeneratorType::SustainPedalOnOff, onoff, modType);
	}
}
