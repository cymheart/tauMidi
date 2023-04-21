#include"RegionSounder.h"
#include"KeySounder.h"
#include"Channel.h"
#include"RegionModulation.h"
#include"Lfo.h"
#include"Tau.h"
#include"UnitTransform.h"
#include"VirInstrument.h"
#include"Preset.h"
#include"TauPool.h"
#include"SampleGenerator.h"
using namespace dsignal;

namespace tau {

	RegionSounder::RegionSounder()
	{

		biquad = new Iir::RBJ::LowPass;
		chorus[0] = new daisysp::Chorus();
		chorus[1] = new daisysp::Chorus();

		//biquad = new Biquad;
		modifyedGenList = new GeneratorList();

		vibLfo = new Lfo();
		modLfo = new Lfo();
		volEnv = new Envelope(EnvelopeType::Vol);
		modEnv = new Envelope(EnvelopeType::Mod);

		regionModulation = new RegionModulation();

		for (int i = 0; i < (int)LfoEnvTarget::ModCount; i++)
		{
			lfoInfoLists[i] = new LfoModInfoList;
			envInfoLists[i] = new EnvModInfoList;
		}

	}

	RegionSounder::~RegionSounder()
	{
		DEL(biquad);
		DEL(modifyedGenList);
		DEL(vibLfo);
		DEL(modLfo);
		DEL(volEnv);
		DEL(modEnv);
		DEL(regionModulation);

		leftChannelSamples = nullptr;
		rightChannelSamples = nullptr;

		for (int i = 0; i < (int)LfoEnvTarget::ModCount; i++)
		{
			DEL(lfoInfoLists[i]);
			DEL(envInfoLists[i]);
		}

		DEL(chorus[0]);
		DEL(chorus[1]);
	}

	// 清理
	void RegionSounder::Clear()
	{
		isRealtimeControl = true;
		input = nullptr;
		insideCtrlModulatorList.CloseAllInsideModulator();
		regionModulation->Clear();
		synther = nullptr;
		tau = nullptr;
		virInst = nullptr;
		keySounder = nullptr;
		sample = nullptr;
		sampleGen = nullptr;
		regionSampleGen = nullptr;
		instRegion = nullptr;
		instGlobalRegion = nullptr;
		presetRegion = nullptr;
		presetGlobalRegion = nullptr;

		//
		isHoldDownKey = false;

		//
		isActiveLowPass = false;
		biquadCutoffFrequency = 0;
		biquadQ = ONESQRT2;
		biquad->reset();

		//
		isActivePortamento = false;
		lastKeyBasePitchMul = 1;
		isActiveLegato = false;

		//
		processedSampleCount = 0;
		basePitchMul = 1;
		isLoopSample = true;
		isSampleProcessEnd = false;
		isSoundEnd = false;
		sec = 0;
		isDownNoteKey = false;
		isNeedOffKey = false;
		downNoteKey = 60;
		velocity = 1;

		//
		ValueFadeInfo initFadeInfo;
		velocityFadeInfo = initFadeInfo;

		//
		attenuation = -1;
		attenFadeInfo = initFadeInfo;

		//
		channelGain[0] = -1;
		channelGain[1] = -1;
		channelGainFadeInfo[0] = initFadeInfo;
		channelGainFadeInfo[1] = initFadeInfo;

		chorusDepth = -1;
		chorusDepthFadeInfo = initFadeInfo;

		fcCompute = -1;
		fcComputeFadeInfo = initFadeInfo;

		//
		lastSamplePos = -1;
		sampleStartIdx = 0;
		sampleEndIdx = 0;
		sampleStartLoopIdx = 0;
		sampleEndLoopIdx = 0;
		sampleProcessBlockSize = 64;
		invSampleProcessBlockSize = 1.0f / sampleProcessBlockSize;
		curtCalBasePitchMul = 1;
		loopPlayBack = LoopPlayBackMode::NonLoop;
		Q = 0;
		fc = 0;

		leftChannelSamples = nullptr;
		rightChannelSamples = nullptr;

		modifyedGenList->Clear();
		modifyedGenList->SetType(RegionType::Instrument);

		for (int i = 0; i < (int)LfoEnvTarget::ModCount; i++)
		{
			lfoInfoLists[i]->clear();
			envInfoLists[i]->clear();
		}

		genList = nullptr;
		settingGenList = nullptr;
		globalSettingGenList = nullptr;

		vibLfo->Clear();
		modLfo->Clear();
		volEnv->Clear();
		modEnv->Clear();

	}


	// 释放
	void RegionSounder::Release()
	{
		if (regionSampleGen != nullptr) {
			sampleGen->ReleaseRegionSampleGen(regionSampleGen);
			regionSampleGen = nullptr;
		}

		TauPool::GetInstance().RegionSounderPool().Push(this);
	}


	RegionSounder* RegionSounder::New()
	{
		RegionSounder* regionSounder = TauPool::GetInstance().RegionSounderPool().Pop();
		regionSounder->Clear();
		return regionSounder;
	}

	//初始化
	void RegionSounder::Init()
	{
		if (sampleGen != nullptr)
			regionSampleGen = sampleGen->CreateRegionSampleGen(keySounder->GetOnKey());

		//
		regionModulation->SetUseCommonModulator(tau->UseCommonModulator());

		//
		if (tau->UseRegionInnerChorusEffect())
		{
			chorus[0]->Init(tau->GetSampleProcessRate());
			chorus[0]->SetLfoDepth(1);
			chorus[0]->SetEffectMix(1);
			chorus[0]->SetDelayMs(8);

			chorus[1]->Init(tau->GetSampleProcessRate());
			chorus[1]->SetLfoDepth(1);
			chorus[1]->SetEffectMix(1);
			chorus[1]->SetDelayMs(8);
		}

		//
		switch (tau->GetRenderQuality())
		{
		case RenderQuality::SuperHigh: sampleProcessBlockSize = 1; break;
		case RenderQuality::High: sampleProcessBlockSize = 16; break;
		case RenderQuality::Good: sampleProcessBlockSize = 32; break;
		default: sampleProcessBlockSize = 64; break;
		}
		invSampleProcessBlockSize = 1.0f / sampleProcessBlockSize;

		//
		volEnv->Create();
		modEnv->Create();
		InitRegionModulation();
		AddEnvModTarget(volEnv, 1, GeneratorType::VolEnvToVolume, nullptr, LfoEnvTarget::ModVolume);

		//
		isRegionHavNoteOnKeyModulator = false;
		if (instRegion->IsHavNoteOnKeyModulator() ||
			instGlobalRegion->IsHavNoteOnKeyModulator() ||
			presetRegion->IsHavNoteOnKeyModulator() ||
			presetGlobalRegion->IsHavNoteOnKeyModulator())
		{
			isRegionHavNoteOnKeyModulator = true;
		}
	}

	//为regionModulation设置初始值
	void RegionSounder::InitRegionModulation()
	{
		regionModulation->SetDestModifyedGenList(modifyedGenList);
		regionModulation->SetModifyedGenTypes(&modifyedGenTypes);
		regionModulation->SetInsideCtrlModulatorList(&insideCtrlModulatorList);
		regionModulation->SetChannel(virInst->GetChannel());
		regionModulation->SetRegions(instRegion, instGlobalRegion, presetRegion, presetGlobalRegion);
	}

	// 按下对应的键
	void RegionSounder::OnKey(int key, float velocity)
	{
		RangeFloat keyRange = modifyedGenList->GetAmountRange(GeneratorType::KeyRange);
		if (key < keyRange.min || key > keyRange.max)
			return;

		SetNotModParams(key, velocity);
		ModulationParams();

		//使用滑音
		UsePortamento();

		//使用连音
		UseLegato();

		OpenLfos();
		OpenEnvs();

		if (regionSampleGen != nullptr)
			regionSampleGen->Trigger(this->velocity);

	}

	// 松开按键
	void RegionSounder::OffKey(float velocity, float releaseSec)
	{
		//没有使用单音模式时才可以保持按键
		if (isHoldDownKey && !virInst->UseMonoMode()) {
			isNeedOffKey = true;
			return;
		}

		if (isSoundEnd)
			return;

		isDownNoteKey = false;
		isNeedOffKey = false;
		isHoldDownKey = false;
		OffKeyEnvs(releaseSec);

		if (loopPlayBack == LoopPlayBackMode::LoopEndContinue)
			isLoopSample = false;
	}


	//设置不需要调制器调制的参数值
	void RegionSounder::SetNotModParams(int key, float velocity)
	{
		if (!modifyedGenList->IsEmpty(GeneratorType::Keynum))
			key = (int)modifyedGenList->GetAmount(GeneratorType::Keynum);
		downNoteKey = key;

		isDownNoteKey = true;
		sec = 0;
		processedSampleCount = 0;

		SetVelocity(velocity);
	}

	//调制生成器参数
	void RegionSounder::ModulationParams()
	{
		AddInsideControllerModulators();
		regionModulation->Modulation();
		SetParams();
	}

	//调制输入按键生成器参数
	void RegionSounder::ModulationInputKeyParams()
	{
		if (!tau->UseCommonModulator() || !isRegionHavNoteOnKeyModulator)
			return;

		ModulationParams();
	}


	//增加和通道对应的内部控制器相关调制器
	void RegionSounder::AddInsideControllerModulators()
	{
		//增加和通道对应的内部控制器相关调制器
		MidiControllerTypeList& ctrlTypes = virInst->GetChannel()->GetUsedControllerTypeList();
		for (int i = 0; i < ctrlTypes.size(); i++)
			insideCtrlModulatorList.OpenInsideCtrlModulator(ctrlTypes[i]);

		//增加和通道对应的内部预设相关调制器
		ModPresetTypeList& presetTypes = virInst->GetChannel()->GetUsedPresetTypeList();
		for (int i = 0; i < presetTypes.size(); i++)
			insideCtrlModulatorList.OpenInsidePresetModulator(presetTypes[i]);
	}


	//设置控制发声参数
	void RegionSounder::SetParams()
	{
		bool isModifyedBasePitchMul = false;
		bool isModifyedSampleStartIdx = false;
		bool isModifyedSampleEndIdx = false;
		bool isModifyedSampleStartLoopIdx = false;
		bool isModifyedSampleEndLoopIdx = false;
		bool isModifyedVolEnv = false;
		bool isModifyedModEnv = false;
		bool isModifyedBiquadParams = false;

		unordered_set<int>::iterator it = modifyedGenTypes.begin();
		for (; it != modifyedGenTypes.end(); it++)
		{
			switch ((GeneratorType)*it)
			{
			case GeneratorType::SampleModes:
				SetSampleModes();
				break;

			case GeneratorType::ScaleTuning:
			case GeneratorType::OverridingRootKey:
			case GeneratorType::FineTune:
			case GeneratorType::CoarseTune:
				if (!isModifyedBasePitchMul)
				{
					SetBasePitchMul();
					isModifyedBasePitchMul = true;
				}
				break;

			case GeneratorType::InitialAttenuation:
				SetAttenuation();
				break;

			case GeneratorType::Pan:
				SetPan();
				break;

			case GeneratorType::ChorusEffectsSend:
				SetChorusDepth();
				break;

			case GeneratorType::StartAddrsOffset:
			case GeneratorType::StartAddrsCoarseOffset:
				if (!isModifyedSampleStartIdx) {
					SetSampleStartIdx();
					isModifyedSampleStartIdx = true;
				}
				break;

			case GeneratorType::EndAddrsOffset:
			case GeneratorType::EndAddrsCoarseOffset:
				if (!isModifyedSampleEndIdx) {
					SetSampleEndIdx();
					isModifyedSampleEndIdx = true;
				}
				break;

			case GeneratorType::StartloopAddrsOffset:
			case GeneratorType::StartloopAddrsCoarseOffset:
				if (!isModifyedSampleStartLoopIdx) {
					SetSampleStartLoopIdx();
					isModifyedSampleStartLoopIdx = true;
				}
				break;

			case GeneratorType::EndloopAddrsOffset:
			case GeneratorType::EndloopAddrsCoarseOffset:
				if (!isModifyedSampleEndLoopIdx) {
					SetSampleEndLoopIdx();
					isModifyedSampleEndLoopIdx = true;
				}
				break;

			case GeneratorType::VibLfoToPitch:
				SetVibLfoToPitch();
				break;
			case GeneratorType::DelayVibLFO:
				SetDelayVibLFO();
				break;
			case GeneratorType::FreqVibLFO:
				SetFreqVibLFO();
				break;

			case GeneratorType::ModLfoToFilterFc:
				SetModLfoToFilterFc();
				break;

			case GeneratorType::ModLfoToPitch:
				SetModLfoToPitch();
				break;

			case GeneratorType::ModLfoToVolume:
				SetModLfoToVolume();
				break;

			case GeneratorType::DelayModLFO:
				SetDelayModLFO();
				break;
			case GeneratorType::FreqModLFO:
				SetFreqModLFO();
				break;

			case GeneratorType::DelayVolEnv:
			case GeneratorType::AttackVolEnv:
			case GeneratorType::HoldVolEnv:
			case GeneratorType::DecayVolEnv:
			case GeneratorType::SustainVolEnv:
			case GeneratorType::ReleaseVolEnv:
			case GeneratorType::KeynumToVolEnvHold:
			case GeneratorType::KeynumToVolEnvDecay:
				if (!isModifyedVolEnv) {
					SetVolEnv();
					isModifyedVolEnv = true;
				}
				break;

			case GeneratorType::DelayModEnv:
			case GeneratorType::AttackModEnv:
			case GeneratorType::HoldModEnv:
			case GeneratorType::DecayModEnv:
			case GeneratorType::SustainModEnv:
			case GeneratorType::ReleaseModEnv:
			case GeneratorType::KeynumToModEnvHold:
			case GeneratorType::KeynumToModEnvDecay:
				if (!isModifyedModEnv) {
					SetModEnv();
					isModifyedModEnv = true;
				}
				break;

			case GeneratorType::ModEnvToPitch:
				SetModEnvToPitch();
				break;

			case GeneratorType::ModEnvToFilterFc:
				SetModEnvToFilterFc();
				break;


			case GeneratorType::InitialFilterFc:
			case GeneratorType::InitialFilterQ:
				if (!isModifyedBiquadParams) {
					SetBiquadParams();
					isModifyedBiquadParams = true;
				}
				break;

			case GeneratorType::SustainPedalOnOff:
				SetSustainPedalOnOff();
				break;

			case GeneratorType::Pressure:
				SetPressure();
				break;

			default:
				break;

			}
		}
	}

	//设置采样模式
	void RegionSounder::SetSampleModes()
	{
		loopPlayBack = (LoopPlayBackMode)modifyedGenList->GetAmount(GeneratorType::SampleModes);
		if (loopPlayBack == LoopPlayBackMode::NonLoop)
			isLoopSample = false;
		else if (loopPlayBack == LoopPlayBackMode::LoopEndContinue)
			isLoopSample = true;
		else
			isLoopSample = true;
	}

	//设置基音调
	void RegionSounder::SetBasePitchMul()
	{
		basePitchMul = CalBasePitchMulByKey(downNoteKey);
		curtCalBasePitchMul = basePitchMul;
	}

	//设置音量衰减值
	void RegionSounder::SetAttenuation()
	{
		attenFadeInfo.dstValue = modifyedGenList->GetAmount(GeneratorType::InitialAttenuation);
		attenFadeInfo.dstValue = UnitTransform::DecibelsToGain(attenFadeInfo.dstValue);
		if (attenFadeInfo.dstValue > 1)
			attenFadeInfo.dstValue = 1;

		if (attenuation == -1)
		{
			attenuation = attenFadeInfo.dstValue;
		}
		else
		{

			//衰减的过渡处理
		   //当orgAttenuation 改变到 dstAttenuation时，如果两者之间的差值过大，将会
		   //造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种衰减的改变
			if (abs(attenuation - attenFadeInfo.dstValue) > 0.01f)
			{
				//预设总量为0.2的衰减通过0.05s来平滑改变，其它平滑改变按此比例调整平滑总时间
				attenFadeInfo.totalFadeTime = 0.02f / 0.2f * abs(attenuation - attenFadeInfo.dstValue);
				attenFadeInfo.orgValue = attenuation;
				attenFadeInfo.totalFadeValue = attenFadeInfo.dstValue - attenFadeInfo.orgValue;
				attenFadeInfo.fadeComputedValue = attenFadeInfo.totalFadeValue / attenFadeInfo.totalFadeTime;
				attenFadeInfo.startFadeSec = sec;
			}
			else
			{
				attenuation = attenFadeInfo.dstValue;
			}
		}
	}


	//设置声向
	void RegionSounder::SetPan()
	{
		float genPan = modifyedGenList->GetAmount(GeneratorType::Pan);

		//声向音量倍率调制
		channelGainFadeInfo[0].dstValue = (50 - genPan) * 0.01f;
		channelGainFadeInfo[1].dstValue = (1 - channelGainFadeInfo[0].dstValue);

		//声向曲线调制音量倍率
		channelGainFadeInfo[0].dstValue = (float)FastSin(channelGainFadeInfo[0].dstValue * M_PI / 2);
		channelGainFadeInfo[1].dstValue = (float)FastSin(channelGainFadeInfo[1].dstValue * M_PI / 2);

		for (int i = 0; i < 2; i++)
		{
			if (channelGain[i] == -1)
			{
				channelGain[i] = channelGainFadeInfo[i].dstValue;
			}
			else
			{
				//channelGain的过渡处理
				//当orgChannelGain 改变到 dstChannelGain时，如果两者之间的差值过大，将会
				//造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种衰减的改变
				if (abs(channelGain[i] - channelGainFadeInfo[i].dstValue) > 0.01f)
				{
					//预设总量为20的改变通过0.05s来平滑改变，其它平滑改变按此比例调整平滑总时间		
					channelGainFadeInfo[i].totalFadeTime = 0.02f / 0.1f * abs(channelGain[i] - channelGainFadeInfo[i].dstValue);
					channelGainFadeInfo[i].orgValue = channelGain[i];
					channelGainFadeInfo[i].totalFadeValue = channelGainFadeInfo[i].dstValue - channelGainFadeInfo[i].orgValue;
					channelGainFadeInfo[i].fadeComputedValue = channelGainFadeInfo[i].totalFadeValue / channelGainFadeInfo[i].totalFadeTime;
					channelGainFadeInfo[i].startFadeSec = sec;
				}
				else
				{
					channelGain[i] = channelGainFadeInfo[i].dstValue;
				}
			}
		}
	}


	//设置和声深度
	void RegionSounder::SetChorusDepth()
	{
		chorusDepthFadeInfo.dstValue = modifyedGenList->GetAmount(GeneratorType::ChorusEffectsSend) * 0.01f;
		if (chorusDepthFadeInfo.dstValue > 1)
			chorusDepthFadeInfo.dstValue = 1;

		if (chorusDepth == -1)
		{
			chorusDepth = chorusDepthFadeInfo.dstValue;
		}
		else
		{
			//衰减的过渡处理
		   //当orgAttenuation 改变到 dstAttenuation时，如果两者之间的差值过大，将会
		   //造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种衰减的改变
			if (abs(chorusDepth - chorusDepthFadeInfo.dstValue) > 0.01f)
			{
				//预设总量为0.2的衰减通过0.05s来平滑改变，其它平滑改变按此比例调整平滑总时间
				chorusDepthFadeInfo.totalFadeTime = 0.02f / 0.2f * abs(chorusDepth - attenFadeInfo.dstValue);
				chorusDepthFadeInfo.orgValue = chorusDepth;
				chorusDepthFadeInfo.totalFadeValue = chorusDepthFadeInfo.dstValue - chorusDepthFadeInfo.orgValue;
				chorusDepthFadeInfo.fadeComputedValue = chorusDepthFadeInfo.totalFadeValue / chorusDepthFadeInfo.totalFadeTime;
				chorusDepthFadeInfo.startFadeSec = sec;
			}
			else
			{
				chorusDepth = chorusDepthFadeInfo.dstValue;
			}
		}
	}



	//设置样本起始位置
	void RegionSounder::SetSampleStartIdx()
	{
		int startIdx = sample != nullptr ? sample->startIdx : 0;
		sampleStartIdx =
			startIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartAddrsCoarseOffset) * 32768;
	}

	//设置样本结束位置
	void RegionSounder::SetSampleEndIdx()
	{
		int endIdx = sample != nullptr ? sample->endIdx : 0;
		sampleEndIdx =
			endIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::EndAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::EndAddrsCoarseOffset) * 32768;
	}

	//设置样本循环起始位置
	void RegionSounder::SetSampleStartLoopIdx()
	{
		int startloopIdx = sample != nullptr ? sample->startloopIdx : 0;
		sampleStartLoopIdx =
			startloopIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartloopAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartloopAddrsCoarseOffset) * 32768;
	}

	//设置样本循环结束位置
	void RegionSounder::SetSampleEndLoopIdx()
	{
		int endloopIdx = sample != nullptr ? sample->endloopIdx : 0;
		sampleEndLoopIdx =
			endloopIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::EndloopAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::EndloopAddrsCoarseOffset) * 32768;
	}

	//设置颤音lfo调制的音调
	void RegionSounder::SetVibLfoToPitch()
	{
		int idx = GetLfoModTargetIdx(LfoEnvTarget::ModPitch, GeneratorType::VibLfoToPitch);
		if (idx == -1)
		{
			AddLfoModTarget(vibLfo, 1, GeneratorType::VibLfoToPitch, UnitTransform::CentsToMul, LfoEnvTarget::ModPitch);
			idx = GetLfoModTargetIdx(LfoEnvTarget::ModPitch, GeneratorType::VibLfoToPitch);
		}

		LfoModInfo modinfo = (*lfoInfoLists[(int)LfoEnvTarget::ModPitch])[idx];
		modinfo.modValue = modifyedGenList->GetAmount(GeneratorType::VibLfoToPitch);
		(*lfoInfoLists[(int)LfoEnvTarget::ModPitch])[idx] = modinfo;
	}

	//设置颤音lfo的延迟
	void RegionSounder::SetDelayVibLFO()
	{
		vibLfo->delay = modifyedGenList->GetAmount(GeneratorType::DelayVibLFO);
	}

	//设置颤音lfo的频率
	void RegionSounder::SetFreqVibLFO()
	{
		vibLfo->freq = modifyedGenList->GetAmount(GeneratorType::FreqVibLFO);
	}

	//设置modlfo调制截止频率
	void RegionSounder::SetModLfoToFilterFc()
	{
		int idx = GetLfoModTargetIdx(LfoEnvTarget::ModFilterCutoff, GeneratorType::ModLfoToFilterFc);
		if (idx == -1)
		{
			AddLfoModTarget(modLfo, 1, GeneratorType::ModLfoToFilterFc, UnitTransform::CentsToMul, LfoEnvTarget::ModFilterCutoff);
			idx = GetLfoModTargetIdx(LfoEnvTarget::ModFilterCutoff, GeneratorType::ModLfoToFilterFc);
		}

		LfoModInfo modinfo = (*lfoInfoLists[(int)LfoEnvTarget::ModFilterCutoff])[idx];
		modinfo.modValue = modifyedGenList->GetAmount(GeneratorType::ModLfoToFilterFc);
		(*lfoInfoLists[(int)LfoEnvTarget::ModFilterCutoff])[idx] = modinfo;
	}

	//设置modlfo调制音调
	void RegionSounder::SetModLfoToPitch()
	{
		int idx = GetLfoModTargetIdx(LfoEnvTarget::ModPitch, GeneratorType::ModLfoToPitch);
		if (idx == -1)
		{
			AddLfoModTarget(modLfo, 1, GeneratorType::ModLfoToPitch, UnitTransform::CentsToMul, LfoEnvTarget::ModPitch);
			idx = GetLfoModTargetIdx(LfoEnvTarget::ModPitch, GeneratorType::ModLfoToPitch);
		}

		LfoModInfo modinfo = (*lfoInfoLists[(int)LfoEnvTarget::ModPitch])[idx];
		modinfo.modValue = modifyedGenList->GetAmount(GeneratorType::ModLfoToPitch);
		(*lfoInfoLists[(int)LfoEnvTarget::ModPitch])[idx] = modinfo;
	}

	//设置modlfo调制音量
	void RegionSounder::SetModLfoToVolume()
	{
		int idx = GetLfoModTargetIdx(LfoEnvTarget::ModVolume, GeneratorType::ModLfoToVolume);
		if (idx == -1)
		{
			AddLfoModTarget(modLfo, 1, GeneratorType::ModLfoToVolume, UnitTransform::DecibelsToGain, LfoEnvTarget::ModVolume);
			idx = GetLfoModTargetIdx(LfoEnvTarget::ModVolume, GeneratorType::ModLfoToVolume);
		}

		LfoModInfo modinfo = (*lfoInfoLists[(int)LfoEnvTarget::ModVolume])[idx];
		modinfo.modValue = modifyedGenList->GetAmount(GeneratorType::ModLfoToVolume);
		(*lfoInfoLists[(int)LfoEnvTarget::ModVolume])[idx] = modinfo;
	}

	//设置modlfo的延迟
	void RegionSounder::SetDelayModLFO()
	{
		modLfo->delay = modifyedGenList->GetAmount(GeneratorType::DelayModLFO);
	}

	//设置modlof的频率
	void RegionSounder::SetFreqModLFO()
	{
		modLfo->freq = modifyedGenList->GetAmount(GeneratorType::FreqModLFO);
	}

	//设置音量包络线控制参数
	void RegionSounder::SetVolEnv()
	{
		volEnv->delaySec = modifyedGenList->GetAmount(GeneratorType::DelayVolEnv);
		volEnv->attackSec = modifyedGenList->GetAmount(GeneratorType::AttackVolEnv);
		volEnv->holdSec = modifyedGenList->GetAmount(GeneratorType::HoldVolEnv);
		volEnv->decaySec = modifyedGenList->GetAmount(GeneratorType::DecayVolEnv);
		volEnv->sustainY = modifyedGenList->GetAmount(GeneratorType::SustainVolEnv);
		volEnv->releaseSec = modifyedGenList->GetAmount(GeneratorType::ReleaseVolEnv);
		volEnv->keyToHold = modifyedGenList->GetAmount(GeneratorType::KeynumToVolEnvHold);
		volEnv->keyToDecay = modifyedGenList->GetAmount(GeneratorType::KeynumToVolEnvDecay);

		volEnv->Reset(sec);
	}

	//设置调制包络线控制参数
	void RegionSounder::SetModEnv()
	{
		modEnv->delaySec = modifyedGenList->GetAmount(GeneratorType::DelayModEnv);
		modEnv->attackSec = modifyedGenList->GetAmount(GeneratorType::AttackModEnv);
		modEnv->holdSec = modifyedGenList->GetAmount(GeneratorType::HoldModEnv);
		modEnv->decaySec = modifyedGenList->GetAmount(GeneratorType::DecayModEnv);
		modEnv->sustainY = modifyedGenList->GetAmount(GeneratorType::SustainModEnv);
		modEnv->releaseSec = modifyedGenList->GetAmount(GeneratorType::ReleaseModEnv);
		modEnv->keyToHold = modifyedGenList->GetAmount(GeneratorType::KeynumToModEnvHold);
		modEnv->keyToDecay = modifyedGenList->GetAmount(GeneratorType::KeynumToModEnvDecay);

		modEnv->Reset(sec);
	}
	//设置调制包络线调制音调
	void RegionSounder::SetModEnvToPitch()
	{
		int pitchIdx = GetEnvModTargetIdx(LfoEnvTarget::ModPitch, GeneratorType::ModEnvToPitch);
		if (pitchIdx == -1)
		{
			AddEnvModTarget(modEnv, 1, GeneratorType::ModEnvToPitch, UnitTransform::CentsToMul, LfoEnvTarget::ModPitch);
			pitchIdx = GetEnvModTargetIdx(LfoEnvTarget::ModPitch, GeneratorType::ModEnvToPitch);
		}

		EnvModInfo pitchModinfo = (*envInfoLists[(int)LfoEnvTarget::ModPitch])[pitchIdx];
		float modEnvModPitchInfoModValue = modifyedGenList->GetAmount(GeneratorType::ModEnvToPitch);

		if (pitchModinfo.modValue != modEnvModPitchInfoModValue)
		{
			pitchModinfo.modValue = modEnvModPitchInfoModValue;
			(*envInfoLists[(int)LfoEnvTarget::ModPitch])[pitchIdx] = pitchModinfo;
		}
	}

	//设置调制包络线调制截止频率
	void RegionSounder::SetModEnvToFilterFc()
	{
		int fcIdx = GetEnvModTargetIdx(LfoEnvTarget::ModFilterCutoff, GeneratorType::ModEnvToFilterFc);
		if (fcIdx == -1)
		{
			AddEnvModTarget(modEnv, 1, GeneratorType::ModEnvToFilterFc, UnitTransform::CentsToMul, LfoEnvTarget::ModFilterCutoff);
			fcIdx = GetEnvModTargetIdx(LfoEnvTarget::ModFilterCutoff, GeneratorType::ModEnvToFilterFc);
		}

		EnvModInfo fcModinfo = (*envInfoLists[(int)LfoEnvTarget::ModFilterCutoff])[fcIdx];
		float modEnvModFcInfoModValue = modifyedGenList->GetAmount(GeneratorType::ModEnvToFilterFc);

		if (fcModinfo.modValue != modEnvModFcInfoModValue)
		{
			fcModinfo.modValue = modEnvModFcInfoModValue;
			(*envInfoLists[(int)LfoEnvTarget::ModFilterCutoff])[fcIdx] = fcModinfo;
		}
	}

	// 设置双二阶Biquad滤波器参数
	void RegionSounder::SetBiquadParams()
	{
		isActiveLowPass = true;
		fc = modifyedGenList->GetAmount(GeneratorType::InitialFilterFc);
		Q = modifyedGenList->GetAmount(GeneratorType::InitialFilterQ);
		Q = UnitTransform::ResonanceDbToFilterQ(Q);

		double qDiff = biquadQ - Q;
		if (fc != biquadCutoffFrequency ||
			(qDiff < -0.0001f || qDiff > 0.0001f))
		{
			biquadSampleRate = tau->GetSampleProcessRate();
			biquadCutoffFrequency = fc;
			biquadQ = Q;
			biquad->setup(biquadSampleRate, biquadCutoffFrequency, biquadQ);
		}
	}

	// 设置延音踏板开关值
	void RegionSounder::SetSustainPedalOnOff()
	{
		float value = modifyedGenList->GetAmount(GeneratorType::SustainPedalOnOff);
		bool isHold = value < 0.5f ? false : true;

		if (isDownNoteKey == false)
			return;

		//设置是否保持按键状态
		isHoldDownKey = isHold;

		if (!isHoldDownKey && isNeedOffKey && isDownNoteKey)
			OffKey(127);

	}

	// 设置压力值
	void RegionSounder::SetPressure()
	{
		float value = modifyedGenList->GetAmount(GeneratorType::Pressure);
		velocityFadeInfo.dstValue = value / sample->velocity;

		if (velocityFadeInfo.dstValue > 1)
			velocityFadeInfo.dstValue = 1;

		if (velocity == -1)
		{
			velocity = velocityFadeInfo.dstValue;
		}
		else
		{
			//衰减的过渡处理
		   //当orgAttenuation 改变到 dstAttenuation时，如果两者之间的差值过大，将会
		   //造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种衰减的改变
			if (abs(velocity - velocityFadeInfo.dstValue) > 0.01f)
			{
				//预设总量为0.2的衰减通过0.05s来平滑改变，其它平滑改变按此比例调整平滑总时间
				velocityFadeInfo.totalFadeTime = 0.02f / 0.2f * abs(velocity - velocityFadeInfo.dstValue);
				velocityFadeInfo.orgValue = velocity;
				velocityFadeInfo.totalFadeValue = velocityFadeInfo.dstValue - velocityFadeInfo.orgValue;
				velocityFadeInfo.fadeComputedValue = velocityFadeInfo.totalFadeValue / velocityFadeInfo.totalFadeTime;
				velocityFadeInfo.startFadeSec = sec;
			}
			else
			{
				velocity = velocityFadeInfo.dstValue;
			}
		}
	}


	// 计算按键key相对于根音符的频率偏移倍率
	float RegionSounder::CalBasePitchMulByKey(int key)
	{
		float sampleRate = sample != nullptr ? sample->sampleRate : tau->GetSampleProcessRate();
		float rateAdjustMul = sampleRate / tau->GetSampleProcessRate();   //频率差矫正倍率

		//
		float scaleTuning = modifyedGenList->GetAmount(GeneratorType::ScaleTuning);

		//
		float overridingRootKey = modifyedGenList->GetAmount(GeneratorType::OverridingRootKey);
		if (overridingRootKey < 0 || modifyedGenList->IsEmpty(GeneratorType::OverridingRootKey))
			overridingRootKey = sample != nullptr ? sample->GetOriginalPitch() : key;

		//按键和根音符之间偏移几个半音
		float semit = (key - overridingRootKey) * scaleTuning * 0.01f;

		//校音处理
		float sampleCentPitchCorrection = sample != nullptr ? sample->GetCentPitchCorrection() : 0;
		float fineTune = modifyedGenList->GetAmount(GeneratorType::FineTune) + sampleCentPitchCorrection;  //一个组合的音分偏移值校正
		semit += (modifyedGenList->GetAmount(GeneratorType::CoarseTune) + fineTune / 100.0f);

		//限制了按键相对overridingRootKey之间半音的差值的最大数量
		//if (semit > 72)
			//semit = 72;

		//半音转倍率
		float baseMul = UnitTransform::SemitoneToMul(semit);

		//偏移半音数量转化为频率偏移倍率
		return baseMul * rateAdjustMul;
	}

	// 设置力度
	void RegionSounder::SetVelocity(float velocity)
	{
		//如果设置了固定力度，那么外置力度将不起效
		if (!modifyedGenList->IsEmpty(GeneratorType::Velocity))
			velocity = modifyedGenList->GetAmount(GeneratorType::Velocity);

		//超出力度范围，力度将设为0，即无力度
		RangeFloat velRange = modifyedGenList->GetAmountRange(GeneratorType::VelRange);
		if (velocity > velRange.max || velocity < velRange.min)
			this->velocity = 0;
		else {

			if (sampleGen != nullptr)
				this->velocity = velocity / 127.0f;
			else
				this->velocity = velocity / sample->velocity; //归一化力度到浮点数

			//暂定
			//this->velocity = powf(this->velocity, 2.0f);
		}

		velocityFadeInfo.dstValue = this->velocity;
	}



	// 使用滑音
	void RegionSounder::UsePortamento()
	{
		if (virInst->UsePortamento() && virInst->GetPortaTime() != 0)
		{
			RegionSounder* regionSounder = virInst->FindLastSameRegion(instRegion);
			if (regionSounder == nullptr)
			{
				KeySounder* lastKeySounder = virInst->GetLastOnKeyStateSounder();
				if (lastKeySounder == nullptr)
				{
					if (virInst->AlwaysUsePortamento())
						lastKeySounder = virInst->GetLastKeySounder();
				}

				if (lastKeySounder != nullptr)
				{
					int lastOnKey = lastKeySounder->GetOnKey();
					lastKeyBasePitchMul = CalBasePitchMulByKey(lastOnKey);
					isActivePortamento = true;
				}
				else
				{
					isActivePortamento = false;
				}
			}
			else
			{
				isActivePortamento = true;
				lastKeyBasePitchMul = regionSounder->GetCurtCalBasePitchMul();
			}
		}
	}


	// 使用连音
	// 连音的定义:根据当前发音区域的乐器的最后一个keySounder的查找与当前regionSounder对应的
	//instRegion,如果找到并且处于Sustain阶段，将直接使当前发音区域从Sustain阶段发音,
	void RegionSounder::UseLegato()
	{
		if (virInst->UseLegato() && virInst->GetLastOnKeyStateSounder() != nullptr)
		{
			KeySounder* keySounder = virInst->GetLastOnKeyStateSounder();
			EnvStage stage;
			RegionSounderList* regionSounderList = keySounder->GetRegionSounderList();
			size_t size = regionSounderList->size();
			for (int i = 0; i < size; i++)
			{
				if ((*regionSounderList)[i]->instRegion == instRegion)
				{
					isActiveLegato = true;
					stage = (*regionSounderList)[i]->GetVolEnvStage();
					if (stage == EnvStage::Sustain)
					{
						volEnv->SetStage(EnvStage::Sustain);
					}
					else
					{
						volEnv->SetBaseSec(
							(*regionSounderList)[i]->volEnv->GetCurtSec() +
							(*regionSounderList)[i]->volEnv->GetBaseSec());
					}
				}
			}

			if (!isActiveLegato)
			{
				stage = (*regionSounderList)[0]->GetVolEnvStage();
				if (stage == EnvStage::Sustain)
				{
					volEnv->SetStage(EnvStage::Sustain);
				}
				else
				{
					volEnv->SetBaseSec(
						(*regionSounderList)[0]->volEnv->GetCurtSec() +
						(*regionSounderList)[0]->volEnv->GetBaseSec());
				}
			}
		}
	}

	// 启动所有LFO调制
	void RegionSounder::OpenLfos()
	{
		for (int i = 0; i < (int)LfoEnvTarget::ModCount; i++)
		{
			for (int j = 0; j < lfoInfoLists[i]->size(); j++)
			{
				(*(lfoInfoLists[i]))[j].lfo->Open(sec);
			}
		}
	}


	// 启动所有包络控制
	void RegionSounder::OpenEnvs()
	{
		for (int i = 0; i < (int)LfoEnvTarget::ModCount; i++)
		{
			for (int j = 0; j < envInfoLists[i]->size(); j++)
			{
				(*(envInfoLists[i]))[j].env->OnKey(downNoteKey, sec);
			}
		}
	}


	void RegionSounder::OffKeyEnvs(float releaseSec)
	{
		for (int i = 0; i < (int)LfoEnvTarget::ModCount; i++)
		{
			for (int j = 0; j < envInfoLists[i]->size(); j++)
			{
				(*(envInfoLists[i]))[j].env->OffKey(sec, releaseSec);
			}
		}
	}



	// 增加一个Lfo调制到目标
	// <param name="lfo"></param>
	// <param name="modValue">调制最大值</param>
	// <param name="target">调制目标</param>
	int RegionSounder::AddLfoModTarget(Lfo* lfo, float modValue, GeneratorType genType, UnitTransformCallBack unitTrans, LfoEnvTarget target)
	{
		LfoModInfo info(lfo, modValue, genType, unitTrans);
		lfoInfoLists[(int)target]->push_back(info);
		return (int)(lfoInfoLists[(int)target]->size() - 1);
	}

	int RegionSounder::GetLfoModTargetIdx(LfoEnvTarget target, GeneratorType genType)
	{
		for (int i = 0; i < lfoInfoLists[(int)target]->size(); i++)
		{
			if ((*(lfoInfoLists[(int)target]))[i].genType == genType)
				return i;
		}
		return -1;
	}


	// 增加一个包络线调制到目标
	// <param name="env">包络线</param>
	// <param name="modValue">调制最大值</param>
	// <param name="target">调制目标</param>
	int RegionSounder::AddEnvModTarget(Envelope* env, float modValue, GeneratorType genType, UnitTransformCallBack unitTrans, LfoEnvTarget target)
	{
		EnvModInfo info(env, modValue, genType, unitTrans);
		envInfoLists[(int)target]->push_back(info);
		return (int)(envInfoLists[(int)target]->size() - 1);
	}

	int RegionSounder::GetEnvModTargetIdx(LfoEnvTarget target, GeneratorType genType)
	{
		for (int i = 0; i < envInfoLists[(int)target]->size(); i++)
		{
			if ((*(envInfoLists[(int)target]))[i].genType == genType)
				return i;
		}
		return -1;
	}


	void RegionSounder::SetFrameBuffer(float* leftChannelBuf, float* rightChannelBuf)
	{
		leftChannelSamples = leftChannelBuf;
		rightChannelSamples = rightChannelBuf;
	}



	//处理不能听到发声的滞留区域，使其结束发音
	void RegionSounder::EndBlockSound()
	{
		//滞留发声区域判定
		//有的区域由于一直不发送0ffkey命令，导致发声区域一直不被关闭，但又没有发声
		//如果不是实时控制，而是在midi播放时，这些发音区域是可以直接关闭的
		//如果是实时控制的，不直接关闭的原因是可能这些区域通过regionModulation调制包络后，包络延音值会被提高，而重新发音
		if (!isRealtimeControl &&
			(GetVolEnvStage() == EnvStage::Decay ||
				GetVolEnvStage() == EnvStage::Sustain) &&
			IsZeroValueRenderChannelBuffer())
		{
			EndSound();
		}
	}

	//处理不能听到发声的滞留区域，使其结束发音
	void RegionSounder::EndSoundLevel(int endLevel)
	{
		switch (endLevel)
		{
		case 0:
			EndBlockSound();
			break;

		case 1:
			//滞留发声区域判定
			//有的区域由于一直不发送0ffkey命令，导致发声区域一直不被关闭，但又没有发声
			if ((GetVolEnvStage() == EnvStage::Decay ||
				GetVolEnvStage() == EnvStage::Sustain ||
				GetVolEnvStage() == EnvStage::Release) &&
				IsZeroValueRenderChannelBuffer())
			{
				EndSound();
			}
			break;

		case 2:
			EndSound();
			break;


		default:
			break;
		}

	}

	//渲染的声道buffer是否为0值
	//注意leftChannelSamples，rightChannelSamples是来自外部的所有区域共用buffer,
	//发声区域会依次使用它，所以此功能的调用必须紧跟在发声synther()结束之后，buffer被再次使用前，才有效
	//这个范围之外，buf数据将会被替换
	bool RegionSounder::IsZeroValueRenderChannelBuffer()
	{
		if (leftChannelSamples == nullptr || rightChannelSamples == nullptr)
			return true;

		int offset = (int)(tau->GetChildFrameSampleCount() * 0.1f);
		for (int i = 0; i < tau->GetChildFrameSampleCount(); i += offset)
		{
			//注意此处0.0001f值不能设置过大，默认为0.0001f，过大，会截断声音
			if (abs(leftChannelSamples[i]) > 0.0001f ||
				abs(rightChannelSamples[i]) > 0.0001f)
				return false;
		}

		return true;
	}

	//检测由效果器带来的尾音是否结束
	bool RegionSounder::IsEffectSoundStop()
	{
		//检测由效果器带来的尾音是否结束
		int offset = (int)(tau->GetChildFrameSampleCount() * 0.03f);
		for (int i = 0; i < tau->GetChildFrameSampleCount(); i += offset)
		{
			if (abs(leftChannelSamples[i]) > 0.001f ||
				abs(rightChannelSamples[i]) > 0.001f)
				return false;
		}

		return true;
	}

	// 时序序列采样块处理
	// <param name="sampleAmount">块中采样点数量</param>
	void RegionSounder::Render()
	{
		if (isSoundEnd)
			return;

		if (isSampleProcessEnd)
		{
			isSoundEnd = true;
			if (tau->UseRegionInnerChorusEffect() && chorusDepth > 0.0001)
			{
				for (int i = 0; i < tau->GetChildFrameSampleCount(); i++)
				{
					chorus[0]->Process(0);
					chorus[1]->Process(0);
					leftChannelSamples[i] = chorus[0]->GetLeft() + chorus[1]->GetLeft();
					rightChannelSamples[i] = chorus[0]->GetRight() + chorus[1]->GetRight();
				}

				if (!IsEffectSoundStop())
					isSoundEnd = false;
			}

			return;
		}

		//
		int blockSamples;
		int idx = 0;
		int sampleCount = tau->GetChildFrameSampleCount();
		float sampleValue;
		float volGain = 1;
		float volGainSampleValue;
		float startVolGain = 0;
		float endVolGain = 0;
		float endSec;
		float pitchOffsetMul;
		float curtPitchMul;
		float atten_mul_vel = 0;
		float a;
		RenderQuality renderQuality = tau->GetRenderQuality();
		ChannelOutputMode outputMode = tau->GetChannelOutputMode();
		float invSampleProcessRate = tau->GetInvSampleProcessRate();


		//
		while (sampleCount > 0)
		{
			//blockSamples控制调制的精度，blockSamples = 1，将会对每个采样都计算调制参数,
			//blockSamples = 64,则每64个采样点计算一次调制参数
			//注意此参数过大，会导致产生不流畅的卡顿音
			blockSamples = (sampleCount > sampleProcessBlockSize) ? sampleProcessBlockSize : sampleCount;
			sampleCount -= blockSamples;
			endSec = sec + invSampleProcessRate * (blockSamples - 1);
			a = 0;

			//过渡参数值
			FadeParams();

			atten_mul_vel = attenuation * velocity;
			chorus[0]->SetEffectMix(chorusDepth);
			chorus[1]->SetEffectMix(chorusDepth);


			//重设低通滤波器
			if (isActiveLowPass) {
				FadeSetLowPassFilter();
				ResetLowPassFilter(endSec);
			}

			//采样音调处理
			pitchOffsetMul = LfosAndEnvsModulation(LfoEnvTarget::ModPitch, endSec);

			//处理滑音
			if (isActivePortamento)
				curtCalBasePitchMul = PortamentoProcess(sec);

			curtPitchMul = curtCalBasePitchMul * pitchOffsetMul;

			if (renderQuality == RenderQuality::Good || renderQuality == RenderQuality::Fast)
			{
				//此时计算的volGain会处理blockSamples(预设64个采样点)个数据，粒度比较粗糙，数据有可能不够平缓，而导致卡顿音  
				//此时通过一个时间上的过渡处理，来平缓数据的粗糙度
				startVolGain = volGain = LfosAndEnvsModulation(LfoEnvTarget::ModVolume, sec) * atten_mul_vel;
				endVolGain = LfosAndEnvsModulation(LfoEnvTarget::ModVolume, endSec) * atten_mul_vel;
			}

			//
			while (blockSamples > 0)
			{

				sampleValue = NextAdjustPitchSample(curtPitchMul);

				//低通滤波处理
				if (isActiveLowPass) {
					sampleValue = biquad->filter(sampleValue);
				}

				if (renderQuality == RenderQuality::Good ||
					renderQuality == RenderQuality::Fast)
				{
					//通过一个采样位置的平缓过渡处理，来平缓精度不足带来的数据阶梯跳跃
					if (startVolGain != endVolGain) {
						volGain = startVolGain * (1 - a) + endVolGain * a;
						a += invSampleProcessBlockSize;
					}
				}
				else
				{
					volGain = LfosAndEnvsModulation(LfoEnvTarget::ModVolume, sec) * atten_mul_vel;
				}

				volGainSampleValue = volGain * sampleValue;
				leftChannelSamples[idx] = channelGain[0] * volGainSampleValue;
				rightChannelSamples[idx] = channelGain[1] * volGainSampleValue;

				if (tau->UseRegionInnerChorusEffect() && chorusDepth > 0.0001)
					ChorusProcessSample(idx);

				idx++;
				processedSampleCount++;
				sec = processedSampleCount * invSampleProcessRate;

				if ((isSampleProcessEnd || volEnv->IsStop()))
				{
					int bufsize = (tau->GetChildFrameSampleCount() - idx) * sizeof(float);
					memset(leftChannelSamples + idx, 0, bufsize);
					memset(rightChannelSamples + idx, 0, bufsize);

					if (tau->UseRegionInnerChorusEffect() && chorusDepth > 0.0001)
					{
						for (int i = idx; i < tau->GetChildFrameSampleCount(); i++)
							ChorusProcessSample(i);
					}

					isDownNoteKey = false;
					isSampleProcessEnd = true;
					return;
				}

				blockSamples--;
			}
		}
	}

	//过渡参数值
	void RegionSounder::FadeParams()
	{

		//力度的过渡处理
		velocity = FadeValue(velocity, velocityFadeInfo);

		//衰减的过渡处理
		attenuation = FadeValue(attenuation, attenFadeInfo);

		//channelGain的过渡处理
		for (int i = 0; i < 2; i++) {
			channelGain[i] = FadeValue(channelGain[i], channelGainFadeInfo[i]);
		}

		//和声深度的过渡处理
		chorusDepth = FadeValue(chorusDepth, chorusDepthFadeInfo);

	}


	//值的过渡处理
	//当orgValue 改变到 dstValue 时，如果两者之间的差值过大，将会
	//造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平缓这种急剧改变
	float RegionSounder::FadeValue(float orgValue, ValueFadeInfo fadeInfo)
	{
		if (orgValue != fadeInfo.dstValue)
		{
			if (sec - fadeInfo.startFadeSec < fadeInfo.totalFadeTime)
				orgValue = fadeInfo.orgValue + fadeInfo.fadeComputedValue * (sec - fadeInfo.startFadeSec);
			else
				orgValue = fadeInfo.dstValue;
		}

		return orgValue;
	}

	void RegionSounder::ChorusProcessSample(int idx)
	{
		chorus[0]->Process(leftChannelSamples[idx]);
		chorus[1]->Process(rightChannelSamples[idx]);
		leftChannelSamples[idx] = chorus[0]->GetLeft() + chorus[1]->GetLeft();
		rightChannelSamples[idx] = chorus[0]->GetRight() + chorus[1]->GetRight();

	}


	// 音调偏移，实际上就是在pcm现有频率的基础上增加或减少指定的频率，达到增高或降低音调的目的
	// 当音调升高时，原始pcm采样会被压缩到更小的时间范围内，以加快采样的播放频率
	// 当音调降低是，原始pcm采样被拉伸到到更大的时间范围内，以减慢采样的播放频率
	// 八度之间的频率倍率为2
	// 半音之间的频率倍率为2^(1/12) = 1.059463f
	// cents之间的频率倍率为 2^(1/1200) = 1.00057779f
	//  1.00057779^100是一个半音之间的频率倍率
	//  1.00057779^1200是一个八度音之间的频率倍率
	//  当cents = 1200时 即带入  1.00057779^x中的x的意思,值为2,即为一个八度之间的频率倍率
	// <param name="octave">偏移数值，单位为八度</param>
	// <param name="semi">偏移数值，单位为半音，是1个octave的1/12，12个半音频率相乘为一个八度</param>
	// <param name="cents">偏移数值，单位是cents,为1个semi的1/100， 100个cents频率相乘为1个半音， 1200个cents频率相乘为1个八度</param>
	//改变音调后目标位置在原始源中位置的计算公式:
	//lastSamplePos：最后在原始源中定位的采样位置
	//curtSamplePos：改变音调后，在变音源中采样位置加1后(往后采样一个样本点)，变换到原始源的位置
	// curtSamplePos = (lastSamplePos/pitchMul + 1) * pitchMul   
	// lastSamplePos/pitchMul: 对lastSamplePos采样位置进行压缩或者拉伸
	//即: curtSamplePos = lastSamplePos + sampleSpeed 
	 //(也可以解释为:lastSamplePos后下一采样点是根据sampleSpeed的速率来移动采样头获取的，当采样头的sampleSpeed快时，下一个采样点的位置跨距就大，
	 //否则，下一个点也可以是lastSamplePos加上一个小数结果，此时要插值计算最后的采样值)
	//最后将在原始源中采用插值平滑curtSamplePos的结果，
	//即使用curtSamplePos前后两个整数位置点的值插值出curtSamplePos位置的值
	//sampleSpeed: 采样速率，相对于原始样本的频率偏移倍率，sampleSpeed == pitchMul;
	float RegionSounder::NextAdjustPitchSample(float sampleSpeed)
	{
		bool isComputedLoopSample = isLoopSample;
		//当循环范围<=0时，将不作为循环样本来对待
		if (sampleEndLoopIdx - sampleStartLoopIdx <= 0)
			isComputedLoopSample = false;

		if (lastSamplePos == -1) {
			lastSamplePos = 0;
			return 0;
		}

		//注意，此处为保证累加精度，必须使用double类型
		//不然变调声音会不平滑
		lastSamplePos += sampleSpeed;

		while (lastSamplePos > sampleEndLoopIdx && isComputedLoopSample)
			lastSamplePos -= (sampleEndLoopIdx - sampleStartLoopIdx);

		//找到插值的前后整数位置
		int prevIntPos = (int)lastSamplePos;
		int nextIntPos = (int)(prevIntPos >= sampleEndLoopIdx && isComputedLoopSample ? sampleStartLoopIdx : prevIntPos + 1);

		//限制范围不超出样本的前后总范围
		if (sample != nullptr && nextIntPos > sampleEndIdx && !isComputedLoopSample)
		{
			isSampleProcessEnd = true;
			nextIntPos = (uint32_t)sampleEndIdx;
			if (prevIntPos > nextIntPos)
				prevIntPos = nextIntPos;
		}

		//计算采样点插值
		double a = lastSamplePos - prevIntPos;

		if (input != nullptr)  //如果存在样本，将对样本采样
			return (input[prevIntPos] * (1.0f - a) + input[nextIntPos] * a);
		else //否则，如果存在样本生成器，将通过生成器生成当前值
			return regionSampleGen->Out(prevIntPos, nextIntPos, a);

	}


	// 滑音处理
	float RegionSounder::PortamentoProcess(float sec)
	{
		if (sec >= virInst->GetPortaTime())
		{
			isActivePortamento = false;
			return basePitchMul;
		}

		float a = sec / virInst->GetPortaTime();
		return (lastKeyBasePitchMul * (1 - a) + a * basePitchMul);
	}


	// 重设低通滤波器
	void RegionSounder::ResetLowPassFilter(float computedSec)
	{
		float fcMul = LfosAndEnvsModulation(LfoEnvTarget::ModFilterCutoff, computedSec);
		float fcResult = fc * fcMul;
		if (fcResult > 19912) { fcResult = 19912; }
		else if (fcResult < 0) { fcResult = 0; }
		if (fcCompute == -1)
			fcCompute = fc;

		fcComputeFadeInfo.dstValue = fcResult;

		//衰减的过渡处理
	   //当fcCompute 改变到 fcComputeFadeInfo.dstValue时，如果两者之间的差值过大，将会
	   //造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种改变
		if (abs(fcCompute - fcComputeFadeInfo.dstValue) > 500)
		{
			//预设总量为0.2的衰减通过0.02s来平滑改变，其它平滑改变按此比例调整平滑总时间
			fcComputeFadeInfo.totalFadeTime = 0.02f / 1000 * abs(fcCompute - fcComputeFadeInfo.dstValue);
			fcComputeFadeInfo.orgValue = fcCompute;
			fcComputeFadeInfo.totalFadeValue = fcComputeFadeInfo.dstValue - fcComputeFadeInfo.orgValue;
			fcComputeFadeInfo.fadeComputedValue = fcComputeFadeInfo.totalFadeValue / fcComputeFadeInfo.totalFadeTime;
			fcComputeFadeInfo.startFadeSec = sec;
		}
		else
		{
			fcCompute = fcComputeFadeInfo.dstValue;
		}

		double qDiff = biquadQ - Q;
		if (fcCompute != biquadCutoffFrequency ||
			(qDiff < -0.0001f || qDiff > 0.0001f))
		{
			biquadCutoffFrequency = fcCompute;
			biquadQ = Q;
			biquad->setup(biquadSampleRate, biquadCutoffFrequency, biquadQ);
		}
	}

	//过渡设置低通滤波
	void RegionSounder::FadeSetLowPassFilter()
	{
		if (fcCompute == fcComputeFadeInfo.dstValue)
			return;

		fcCompute = FadeValue(fcCompute, fcComputeFadeInfo);

		double qDiff = biquadQ - Q;
		if (fcCompute != biquadCutoffFrequency ||
			(qDiff < -0.0001f || qDiff > 0.0001f))
		{
			biquadCutoffFrequency = fcCompute;
			biquadQ = Q;
			biquad->setup(biquadSampleRate, biquadCutoffFrequency, biquadQ);
		}
	}


	// lfos, envs调制
	float RegionSounder::LfosAndEnvsModulation(LfoEnvTarget modTarget, float computedSec)
	{
		float result = 1;
		size_t size;


		LfoModInfoList& lfoInfoList = *(lfoInfoLists[(int)modTarget]);
		size = lfoInfoList.size();
		for (int i = 0; i < size; i++)
		{
			float lfoVal = lfoInfoList[i].lfo->SinWave(computedSec);
			lfoVal *= lfoInfoList[i].modValue;
			if (lfoInfoList[i].unitTransform != nullptr)
				result *= lfoInfoList[i].unitTransform(lfoVal);
			else
				result *= lfoVal;
		}


		EnvModInfoList& envInfoList = *(envInfoLists[(int)modTarget]);
		size = envInfoList.size();
		for (int i = 0; i < size; i++)
		{
			float envVal = envInfoList[i].env->GetEnvValue(computedSec);
			envVal *= envInfoList[i].modValue;

			if (envInfoList[i].unitTransform != nullptr)
				result *= envInfoList[i].unitTransform(envVal);
			else
				result *= envVal;
		}


		return result;
	}

}
