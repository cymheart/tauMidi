#include"RegionSounder.h"
#include"KeySounder.h"
#include"Channel.h"
#include"RegionModulation.h"
#include"Lfo.h"
#include"Ventrue.h"
#include"UnitTransform.h"
#include"VirInstrument.h"
using namespace dsignal;

namespace ventrue {

	RegionSounder::RegionSounder()
	{

		biquad = new Iir::RBJ::LowPass;


		//biquad = new Biquad;
		modifyedGenList = new GeneratorList();

		vibLfo = new Lfo();
		modLfo = new Lfo();
		volEnv = new Envelope();
		modEnv = new Envelope();

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

	}

	// 清理
	void RegionSounder::Clear()
	{
		isRealtimeControl = true;
		input = nullptr;
		insideCtrlModulatorList.CloseAllInsideModulator();
		regionModulation->Clear();
		ventrue = nullptr;
		virInst = nullptr;
		sample = nullptr;
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
		invSampleProcessRate = 1.0f / 44100.0f;
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
		attenuation = -1;
		dstAttenuation = -1;
		orgAttenuation = -1;
		totalAttenFadeValue = 0;
		startAttenFadeSec = 0;
		totalAttenFadeTime = 0.05f;
		attenFadeComputedValue = 0;

		//
		for (int i = 0; i < 2; i++)
		{
			channelGain[i] = -1;
			dstChannelGain[i] = -1;
			orgChannelGain[i] = -1;
			totalChannelGainFadeValue[i] = 0;
			startChannelGainFadeSec[i] = 0;
			totalChannelGainFadeTime[i] = 0.05f;
			channelGainFadeComputedValue[i] = 0;
		}

		//
		lastSamplePos = -1;
		sampleStartIdx = 0;
		sampleEndIdx = 0;
		sampleStartLoopIdx = 0;
		sampleEndLoopIdx = 0;
		sampleProcessBlockSize = 64;
		invSampleProcessBlockSize = 1.0f / sampleProcessBlockSize;
		childFrameSampleCount = 64;
		sampleProcessRate = 44100;
		curtCalBasePitchMul = 1;
		loopPlayBack = LoopPlayBackMode::NonLoop;
		Q = 0;
		fc = 0;

		leftChannelSamples = nullptr;
		rightChannelSamples = nullptr;

		modifyedGenList->Clear();
		modifyedGenList->SetType(RegionType::Insttrument);

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
		//delete this;
		VentruePool::GetInstance().RegionSounderPool().Push(this);
	}


	RegionSounder* RegionSounder::New()
	{
		RegionSounder* regionSounder = VentruePool::GetInstance().RegionSounderPool().Pop();
		//RegionSounder* regionSounder = new RegionSounder();
		regionSounder->Clear();
		return regionSounder;
	}

	//初始化
	void RegionSounder::Init()
	{
		//
		switch (ventrue->GetRenderQuality())
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

	// 松开按键
	void RegionSounder::OffKey(float velocity, float releaseSec)
	{
		//没有使用单音模式时才可以保持按键
		if (isHoldDownKey && !virInst->UseMonoMode()) {
			isNeedOffKey = true;
			return;
		}

		isDownNoteKey = false;
		isNeedOffKey = false;
		isHoldDownKey = false;
		OffKeyEnvs(releaseSec);

		if (loopPlayBack == LoopPlayBackMode::LoopEndContinue)
			isLoopSample = false;
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

	}


	//设置不需要调制器调制的参数值
	void RegionSounder::SetNotModParams(int key, float velocity)
	{
		if (!modifyedGenList->IsEmpty(GeneratorType::Keynum))
			key = (int)modifyedGenList->GetAmount(GeneratorType::Keynum);
		downNoteKey = key;

		sampleProcessRate = ventrue->GetSampleProcessRate();
		childFrameSampleCount = ventrue->GetChildFrameSampleCount();
		invSampleProcessRate = 1.0f / sampleProcessRate;

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
				SetAttenuationl();
				break;

			case GeneratorType::Pan:
				SetPan();
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
	void RegionSounder::SetAttenuationl()
	{
		dstAttenuation = modifyedGenList->GetAmount(GeneratorType::InitialAttenuation);
		dstAttenuation = UnitTransform::DecibelsToGain(dstAttenuation);

		if (attenuation == -1)
		{
			attenuation = dstAttenuation;
		}
		else
		{
			//衰减的过渡处理
		   //当orgAttenuation 改变到 dstAttenuation时，如果两者之间的差值过大，将会
		   //造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种衰减的改变
			if (abs(attenuation - dstAttenuation) > 0.05f)
			{
				//预设总量为0.2的衰减通过0.05s来平滑改变，其它平滑改变按此比例调整平滑总时间
				totalAttenFadeTime = 0.05f / 0.2f * abs(attenuation - dstAttenuation);
				orgAttenuation = attenuation;
				totalAttenFadeValue = dstAttenuation - orgAttenuation;
				attenFadeComputedValue = totalAttenFadeValue / totalAttenFadeTime;
				startAttenFadeSec = sec;
			}
			else
			{
				attenuation = dstAttenuation;
			}
		}
	}

	//设置声向
	void RegionSounder::SetPan()
	{
		float genPan = modifyedGenList->GetAmount(GeneratorType::Pan);

		//声向音量倍率调制
		dstChannelGain[0] = (50 - genPan) * 0.01f;
		dstChannelGain[1] = (1 - dstChannelGain[0]);

		//声向曲线调制音量倍率
		dstChannelGain[0] = (float)FastSin(dstChannelGain[0] * M_PI / 2);
		dstChannelGain[1] = (float)FastSin(dstChannelGain[1] * M_PI / 2);

		for (int i = 0; i < 2; i++)
		{
			if (channelGain[i] == -1)
			{
				channelGain[i] = dstChannelGain[i];
			}
			else
			{
				//channelGain的过渡处理
				//当orgChannelGain 改变到 dstChannelGain时，如果两者之间的差值过大，将会
				//造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平滑这种衰减的改变
				if (abs(channelGain[i] - dstChannelGain[i]) > 0.05f)
				{
					//预设总量为20的改变通过0.05s来平滑改变，其它平滑改变按此比例调整平滑总时间		
					totalChannelGainFadeTime[i] = 0.05f / 0.2f * abs(channelGain[i] - dstChannelGain[i]);
					orgChannelGain[i] = channelGain[i];
					totalChannelGainFadeValue[i] = dstChannelGain[i] - orgChannelGain[i];
					channelGainFadeComputedValue[i] = totalChannelGainFadeValue[i] / totalChannelGainFadeTime[i];
					startChannelGainFadeSec[i] = sec;
				}
				else
				{
					channelGain[i] = dstChannelGain[i];
				}
			}
		}
	}

	//设置样本起始位置
	void RegionSounder::SetSampleStartIdx()
	{
		sampleStartIdx =
			sample->startIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartAddrsCoarseOffset) * 32768;
	}

	//设置样本结束位置
	void RegionSounder::SetSampleEndIdx()
	{
		sampleEndIdx = sample->endIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::EndAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::EndAddrsCoarseOffset) * 32768;
	}

	//设置样本循环起始位置
	void RegionSounder::SetSampleStartLoopIdx()
	{
		sampleStartLoopIdx =
			sample->startloopIdx
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartloopAddrsOffset)
			+ (int)modifyedGenList->GetAmount(GeneratorType::StartloopAddrsCoarseOffset) * 32768;
	}

	//设置样本循环结束位置
	void RegionSounder::SetSampleEndLoopIdx()
	{
		sampleEndLoopIdx =
			sample->endloopIdx
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
			biquadSampleRate = ventrue->GetSampleProcessRate();
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

	// 计算按键key相对于根音符的频率偏移倍率
	float RegionSounder::CalBasePitchMulByKey(int key)
	{

		float sampleRate = sample->sampleRate;
		float rateAdjustMul = sampleRate / ventrue->GetSampleProcessRate();   //频率差矫正倍率

		//
		float scaleTuning = modifyedGenList->GetAmount(GeneratorType::ScaleTuning);

		//
		float overridingRootKey = modifyedGenList->GetAmount(GeneratorType::OverridingRootKey);
		if (overridingRootKey < 0 || modifyedGenList->IsEmpty(GeneratorType::OverridingRootKey))
			overridingRootKey = sample->GetOriginalPitch();

		//按键和根音符之间偏移几个半音
		float semit = (key - overridingRootKey) * scaleTuning * 0.01f;

		//校音处理
		float fineTune = modifyedGenList->GetAmount(GeneratorType::FineTune) + sample->GetCentPitchCorrection();  //一个组合的音分偏移值校正
		semit += (modifyedGenList->GetAmount(GeneratorType::CoarseTune) + fineTune / 100.0f) * scaleTuning * 0.01f;

		//限制了按键相对overridingRootKey之间半音的差值的最大数量
		if (semit > 72)
			semit = 72;

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
		else
			this->velocity = velocity / sample->velocity; //归一化力度到浮点数
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

	//渲染的声道buffer是否为0值
	//注意leftChannelSamples，rightChannelSamples是来自外部的所有区域共用buffer,
	//发声区域会依次使用它，所以此功能的调用必须紧跟在发声render()结束之后，buffer被再次使用前，才有效
	//这个范围之外，buf数据将会被替换
	bool RegionSounder::IsZeroValueRenderChannelBuffer()
	{
		if (leftChannelSamples == nullptr || rightChannelSamples == nullptr)
			return true;

		int offset = (int)(childFrameSampleCount * 0.1f);
		for (int i = 0; i < childFrameSampleCount; i += offset)
		{
			//注意此处0.0001f值不能设置过大，默认为0.0001f，过大，会截断声音
			if (fabsf(leftChannelSamples[i]) > 0.0001f ||
				fabsf(rightChannelSamples[i]) > 0.0001f)
				return false;
		}

		return true;
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

	// 时序序列采样块处理
	// <param name="sampleAmount">块中采样点数量</param>
	void RegionSounder::Render()
	{
		if (isSampleProcessEnd)
		{
			isSoundEnd = true;
			return;
		}

		//
		int blockSamples;
		int idx = 0;
		int sampleCount = childFrameSampleCount;
		float sampleValue;
		float volGain;
		float volGainSampleValue;
		float startVolGain = 0;
		float endVolGain = 0;
		float endSec;
		float pitchOffsetMul;
		float curtPitchMul;
		float atten_mul_vel = 0;
		float a;
		RenderQuality renderQuality = ventrue->GetRenderQuality();
		ChannelOutputMode outputMode = ventrue->GetChannelOutputMode();

		while (sampleCount > 0)
		{
			//blockSamples控制调制的精度，blockSamples = 1，将会对每个采样都计算调制参数,
			//blockSamples = 64,则每64个采样点计算一次调制参数
			//注意此参数过大，会导致产生不流畅的卡顿音
			blockSamples = (sampleCount > sampleProcessBlockSize) ? sampleProcessBlockSize : sampleCount;
			sampleCount -= blockSamples;
			endSec = sec + invSampleProcessRate * (blockSamples - 1);
			a = 0;

			//重设低通滤波器
			if (isActiveLowPass)
				ResetLowPassFilter(endSec);

			//采样音调处理
			pitchOffsetMul = LfosAndEnvsModulation(LfoEnvTarget::ModPitch, endSec);

			//处理滑音
			if (isActivePortamento)
				curtCalBasePitchMul = PortamentoProcess(sec);

			curtPitchMul = curtCalBasePitchMul * pitchOffsetMul;


			//衰减的过渡处理
			//当orgAttenuation 改变到 dstAttenuation时，如果两者之间的差值过大，将会
			//造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平缓这种衰减的急剧改变
			if (attenuation != dstAttenuation)
			{
				if (sec - startAttenFadeSec < totalAttenFadeTime)
					attenuation = orgAttenuation + attenFadeComputedValue * (sec - startAttenFadeSec);
				else
					attenuation = dstAttenuation;
			}
			atten_mul_vel = attenuation * velocity;


			//channelGain的过渡处理
			//当orgChannelGain 改变到 dstChannelGain时，如果两者之间的差值过大，将会
			//造成不连续断音的违和感，此时通过一个时间上的过渡处理，来平缓这种衰减的急剧改变
			for (int i = 0; i < 2; i++) {
				if (channelGain[i] != dstChannelGain[i])
				{
					if (sec - startChannelGainFadeSec[i] < totalChannelGainFadeTime[i])
						channelGain[i] = orgChannelGain[i] + channelGainFadeComputedValue[i] * (sec - startChannelGainFadeSec[i]);
					else
						channelGain[i] = dstChannelGain[i];
				}
			}

			//
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
				if (isActiveLowPass)
					sampleValue = biquad->filter(sampleValue);

				if (renderQuality == RenderQuality::Good ||
					renderQuality == RenderQuality::Fast)
				{
					//通过一个采样位置的平缓过渡处理，来平缓精度不足带来的数据阶梯跳跃
					volGain = startVolGain * (1 - a) + endVolGain * a;
					a += invSampleProcessBlockSize;
				}
				else
				{
					volGain = LfosAndEnvsModulation(LfoEnvTarget::ModVolume, sec) * atten_mul_vel;
				}

				volGainSampleValue = volGain * sampleValue;
				leftChannelSamples[idx] = channelGain[0] * volGainSampleValue;
				rightChannelSamples[idx] = channelGain[1] * volGainSampleValue;

				idx++;
				processedSampleCount++;
				sec = processedSampleCount * invSampleProcessRate;

				if ((isSampleProcessEnd || volEnv->IsStop()))
				{
					int bufsize = (childFrameSampleCount - idx) * sizeof(float);
					memset(leftChannelSamples + idx, 0, bufsize);
					memset(rightChannelSamples + idx, 0, bufsize);
					isDownNoteKey = false;
					isSampleProcessEnd = true;
					return;
				}

				blockSamples--;
			}
		}
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

		lastSamplePos = lastSamplePos + sampleSpeed;

		while (lastSamplePos > sampleEndLoopIdx && isComputedLoopSample)
			lastSamplePos -= (sampleEndLoopIdx - sampleStartLoopIdx);

		//找到插值的前后整数位置
		uint32_t prevIntPos = (uint32_t)lastSamplePos;
		uint32_t nextIntPos = (prevIntPos >= sampleEndLoopIdx && isComputedLoopSample ? sampleStartLoopIdx : prevIntPos + 1);

		//限制范围不超出样本的前后总范围
		if (nextIntPos > sampleEndIdx && !isComputedLoopSample)
		{
			isSampleProcessEnd = true;
			nextIntPos = sampleEndIdx;
			if (prevIntPos > nextIntPos)
				prevIntPos = nextIntPos;
		}

		//计算采样点插值
		float a = (float)(lastSamplePos - prevIntPos);
		return (input[prevIntPos] * (1.0f - a) + input[nextIntPos] * a);

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

		double qDiff = biquadQ - Q;
		if (fcResult != biquadCutoffFrequency ||
			(qDiff < -0.0001f || qDiff > 0.0001f))
		{
			biquadCutoffFrequency = fcResult;
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
