#include"ZoneSounder.h"
#include"KeySounder.h"
#include"Channel.h"
#include"Lfo.h"
#include"Tau.h"
#include"UnitTransform.h"
#include"VirInstrument.h"
#include"Preset.h"
#include"TauPool.h"
#include"Zone.h"
#include"SampleGenerator.h"
using namespace dsignal;

namespace tau {

	ZoneSounder::ZoneSounder()
	{

		biquad = new Iir::RBJ::LowPass;
		chorus[0] = new daisysp::Chorus();
		chorus[1] = new daisysp::Chorus();

		//biquad = new Biquad;

		vibLfo = new Lfo();
		modLfo = new Lfo();
		volEnv = new Envelope(EnvelopeType::Vol);
		modEnv = new Envelope(EnvelopeType::Mod);

	}

	ZoneSounder::~ZoneSounder()
	{
		DEL(biquad);
		DEL(vibLfo);
		DEL(modLfo);
		DEL(volEnv);
		DEL(modEnv);

		leftChannelSamples = nullptr;
		rightChannelSamples = nullptr;

		DEL(chorus[0]);
		DEL(chorus[1]);
	}

	// 清理
	void ZoneSounder::Clear()
	{
		isRealtimeControl = true;
		synther = nullptr;
		tau = nullptr;
		virInst = nullptr;
		keySounder = nullptr;
		sample = nullptr;
		sampleGen = nullptr;
		zoneSampleGen = nullptr;
		instZone = nullptr;
		instGlobalZone = nullptr;
		presetZone = nullptr;
		presetGlobalZone = nullptr;
		insideModulators = nullptr;

		//
		isHoldDownKey = false;

		//
		isActiveLowPass = false;
		biquadFcCents = 0;
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
		fcCents = 0;

		leftChannelSamples = nullptr;
		rightChannelSamples = nullptr;

		resultGens.Clear();
		computedGens.Clear();

		//
		vibLfoInfos.clear();
		modLfoInfos.clear();
		volEnvInfos.clear();
		modEnvInfos.clear();

		//
		vibLfo->Clear();
		modLfo->Clear();
		volEnv->Clear();
		modEnv->Clear();

	}


	// 释放
	void ZoneSounder::Release()
	{
		if (zoneSampleGen != nullptr) {
			sampleGen->ReleaseZoneSampleGen(zoneSampleGen);
			zoneSampleGen = nullptr;
		}

		TauPool::GetInstance().ZoneSounderPool().Push(this);
	}


	ZoneSounder* ZoneSounder::New()
	{
		ZoneSounder* zoneSounder = TauPool::GetInstance().ZoneSounderPool().Pop();
		zoneSounder->Clear();
		return zoneSounder;
	}

	//初始化
	void ZoneSounder::Init()
	{
		if (sampleGen != nullptr)
			zoneSampleGen = sampleGen->CreateZoneSampleGen(keySounder->GetOnKey());

		insideModulators = tau->GetInsideModulators();

		ComputeZoneGens();
		ComputeZoneMods();
		

		//
		if (tau->UseZoneInnerChorusEffect())
		{
			chorus[0]->Init(tau->GetSampleProcessRate());
			chorus[0]->SetLfoDepth(0.5f);
			chorus[0]->SetEffectMix(0);
			chorus[0]->SetDelayMs(8);

			chorus[1]->Init(tau->GetSampleProcessRate());
			chorus[1]->SetLfoDepth(0.5f);
			chorus[1]->SetEffectMix(0);
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

		SetLfoEnvModInfo(volEnvInfos, GeneratorType::VolEnvToVolume, nullptr);
	}


	// 按下对应的键
	void ZoneSounder::OnKey(int key, float vel)
	{
		GeneratorAmount keyRange = resultGens.GetAmount(GeneratorType::KeyRange);
		if (key < keyRange.rangeData.low || key >keyRange.rangeData.high)
			return;

		//
		if (!resultGens.IsEmpty(GeneratorType::Keynum))
			key = resultGens.GetAmount(GeneratorType::Keynum).amount;
		downNoteKey = key;
		isDownNoteKey = true;
		sec = 0;
		processedSampleCount = 0;
		SetVelocity(vel);

		//调制
		Modulation();
		//使用滑音
		UsePortamento();
		//使用连音
		UseLegato();

		// 启动所有LFO调制,包络控制
		vibLfo->Open(sec);
		modLfo->Open(sec);
		volEnv->OnKey(downNoteKey, sec);
		modEnv->OnKey(downNoteKey, sec);

		//
		if (zoneSampleGen != nullptr)
			zoneSampleGen->Trigger(velocity);

	}

	
	// 松开按键
	// releaseSec : 重设按键释放时长， -1为不设置
	void ZoneSounder::OffKey(float velocity, float releaseSec)
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

		//
		volEnv->OffKey(sec, releaseSec);
		modEnv->OffKey(sec, releaseSec);

		if (loopPlayBack == LoopPlayBackMode::LoopEndContinue)
			isLoopSample = false;
	}

	//计算区域的生成器列表
	//复合同一区域的乐器生成列表(instZone)和乐器全局生成器列表(instGlobalZone)值到modifyedGenList中
	//复合方法: 优先使用instZone中的值，如果没有将使用instGlobalZone的值，
	//然后叠加preset的值
	//这个复合处理在ZoneSounder建立后只需要处理一次，将会直到ZoneSounder发声结束一直缓存这组值
	void ZoneSounder::ComputeZoneGens()
	{
		GeneratorList& instGens = instZone->GetGens();
		GeneratorList& instGlobalGens = instGlobalZone->GetGens();
		for (int i = 0; i < (int)GeneratorType::EndOper; i++)
		{
			GeneratorType type = (GeneratorType)i;
			switch (type)
			{
			case GeneratorType::KeyRange:
			case GeneratorType::VelRange:
				if (!instGens.IsEmpty(type)) {
					GeneratorAmount amount = instGens.GetAmount(type);
					computedGens.SetAmount(type, amount);
				}
				else if (!instGlobalGens.IsEmpty(type)) {
					GeneratorAmount amount = instGlobalGens.GetAmount(type);
					computedGens.SetAmount(type, amount);
				}
				break;

			case GeneratorType::ChorusEffectsSend:
			case GeneratorType::ReverbEffectsSend:
			case GeneratorType::SampleID:		
				break;

			case GeneratorType::VolEnvToVolume: 
			{
				GeneratorAmount amount = { 1 };
				computedGens.SetAmount(type, amount);
			}	
			break;

			default:
				if (!instGens.IsEmpty(type)) 
					computedGens.SetAmount(type, instGens.GetAmount(type));
			
				else if (!instGlobalGens.IsEmpty(type)) 
					computedGens.SetAmount(type, instGlobalGens.GetAmount(type));
				break;
			}
		}

		//
		GeneratorAmount presetAmount;
		GeneratorAmount genAmount;
		GeneratorList& presetGens = presetZone->GetGens();
		GeneratorList& presetGlobalGens = presetGlobalZone->GetGens();
		for (int i = 0; i < (int)GeneratorType::EndOper; i++)
		{
			GeneratorType type = (GeneratorType)i;

			switch (type)
			{
			case GeneratorType::KeyRange:
			case GeneratorType::VelRange:
				presetAmount.rangeData.low = 1;
				presetAmount.rangeData.high = 0;
				if (!presetGens.IsEmpty(type))
					presetAmount = presetGens.GetAmount(type);
				else if (!presetGlobalGens.IsEmpty(type))
					presetAmount = presetGlobalGens.GetAmount(type);

				if (presetAmount.rangeData.low <= presetAmount.rangeData.high) {
					genAmount = computedGens.GetAmount(type);
					genAmount.rangeData.low = max(presetAmount.rangeData.low, genAmount.rangeData.low);
					genAmount.rangeData.high = min(presetAmount.rangeData.high, genAmount.rangeData.high);
					computedGens.SetAmount(type, genAmount);
				}
				break;

			//preset中这些值被忽略使用
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
			case GeneratorType::SampleModes:
			case GeneratorType::OverridingRootKey:
			case GeneratorType::SampleID:
				break;

			default:
				if (!presetGens.IsEmpty(type)) {
					genAmount.amount = computedGens.GetAmount(type).amount + presetGens.GetAmount(type).amount;
					computedGens.SetAmount(type, genAmount);
				}
				else if (!presetGlobalGens.IsEmpty(type)) {
					genAmount.amount = computedGens.GetAmount(type).amount + presetGlobalGens.GetAmount(type).amount;
					computedGens.SetAmount(type, genAmount);
				}
				break;
			}
		}

		//
		resultGens.Copy(computedGens);
	}



	//计算区域的调制器列表
	void ZoneSounder::ComputeZoneMods() 
	{
		vector<Modulator*>& instMods = instZone->GetModulators();
		vector<Modulator*>& instGlobalMods = instGlobalZone->GetModulators();

		computedMods.clear();
		computedMods.assign(instMods.begin(), instMods.end());
		int size = computedMods.size();
		bool isSame;
		for (int i = 0; i < instGlobalMods.size(); i++) {
			isSame = false;
			for (int j = 0; j < size; j++) {
				if (computedMods[j]->IsSame(instGlobalMods[i])) {
					isSame = true;
					break;
				}
			}
			if(!isSame)
				computedMods.push_back(instGlobalMods[i]);
		}

		//
		vector<Modulator*>& presetMods = presetZone->GetModulators();
		vector<Modulator*>& presetGlobalMods = presetGlobalZone->GetModulators();
		vector<Modulator*> presetComputedMods;
		presetComputedMods.assign(presetMods.begin(), presetMods.end());
		size = presetComputedMods.size();
		isSame = false;
		for (int i = 0; i < presetGlobalMods.size(); i++) {
			isSame = false;
			for (int j = 0; j < size; j++) {
				if (presetComputedMods[j]->IsSame(presetGlobalMods[i])) {
					isSame = true;
					break;
				}
			}
			if (!isSame)
				presetComputedMods.push_back(presetGlobalMods[i]);
		}

		//
		for (int i = 0; i < computedMods.size(); i++)
			computedModAmounts[i] = computedMods[i]->GetAmount();
		//
		size = computedMods.size();
		for (int i = 0; i < presetComputedMods.size(); i++) 
		{
			isSame = false;
			for (int j = 0; j < size; j++) 
			{
				if (presetComputedMods[i]->IsSame(computedMods[j])) {
					computedModAmounts[j] = computedMods[j]->GetAmount() + presetComputedMods[i]->GetAmount();
					isSame = true;
					break;
				}
			}

			if (!isSame) {
				computedMods.push_back(presetComputedMods[i]);
				computedModAmounts[computedMods.size() - 1] = presetComputedMods[i]->GetAmount();
			}
		}
	}

	//计算区域的结果调制器列表
	void ZoneSounder::ComputeZoneResultMods()
	{
		resultMods.clear();
		resultMods.assign(computedMods.begin(), computedMods.end());
		int size = resultMods.size();
		for (int i = 0; i < computedMods.size(); i++)
			resultModAmounts[i] = computedModAmounts[i];

		//
		vector<Modulator*>& insideMods = insideModulators->GetModulators();
		bool isSame;
		for (int i = 0; i < insideMods.size(); i++)
		{
			isSame = false;
			for (int j = 0; j < size; j++)
			{
				if (insideMods[i]->IsSame(resultMods[j])) {
					resultModAmounts[j] = resultMods[j]->GetAmount() + insideMods[i]->GetAmount();
					isSame = true;
					break;
				}
			}

			if (!isSame) {
				resultMods.push_back(insideMods[i]);
				resultModAmounts[resultMods.size() - 1] = insideMods[i]->GetAmount();
			}
		}
	}

	//增加和通道对应的内部控制器相关调制器
	void ZoneSounder::AddInsideModulators()
	{
		insideModulators->CloseAllInsideModulator();

		//增加和通道对应的内部控制器相关调制器
		MidiControllerTypeList& ctrlTypes = virInst->GetChannel()->GetUsedControllerTypeList();
		for (int i = 0; i < ctrlTypes.size(); i++) 
			insideModulators->OpenInsideCtrlModulator(ctrlTypes[i]);
		
		//增加和通道对应的内部预设相关调制器
		vector<ModInputPreset>& presetTypes = virInst->GetChannel()->GetUsedPresetTypeList();
		for (int i = 0; i < presetTypes.size(); i++)
			insideModulators->OpenInsidePresetModulator(presetTypes[i]);
	}

	//调制
	void ZoneSounder::Modulation()
	{
		AddInsideModulators();
		ComputeZoneResultMods();
		//调制ResultGens
		ModResultGens(); 		
	
		//
		SetSampleModes();
		SetBasePitchMul();
		SetAttenuation();
		SetPan();
		SetSampleStartIdx();
		SetSampleEndIdx();
		SetSampleStartLoopIdx();
		SetSampleEndLoopIdx();	
	  	SetChorusDepth();
		SetVibLfoToPitch();
		SetDelayVibLFO();
		SetFreqVibLFO();
		SetModLfoToFilterFc();
		SetModLfoToPitch();
		SetModLfoToVolume();
		SetDelayModLFO();
		SetFreqModLFO();
		SetVolEnv();
		SetModEnv();
		SetModEnvToPitch();
		SetModEnvToFilterFc();
		SetBiquadParams();
		SetSustainPedalOnOff();
		SetPressure();
	}

	//调制ResultGens
	void ZoneSounder::ModResultGens()
	{
		//恢复resultGens数据到未用调制器调制状态
		if(!resultMods.empty())
			resultGens.Copy(computedGens);

		//循环遍历每个调制器的输入项，排除掉非调制Gen的调制器(有的调制器的调制目标时另一个调制器)
		//对于输入项是外部Midi控制器或内部预设输入的进行值输入操作
		Channel* channel = virInst->GetChannel();
		GeneratorType targetGenType = GeneratorType::None;
		for (int i = 0; i < resultMods.size(); i++)
		{
			Modulator& mod = *(resultMods[i]);
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

					if (mod.GetType() == ModulatorType::Inside &&
						mod.GetInputPresetType(j) == ModInputPreset::PitchWheel)
					{
						mod.SetAmount(channel->GetPitchBendRange());
					}
				}
			}
		}

		//执行调制，获取调制值
		float oldAmount;
		float modValue;
		GeneratorAmount genAmount;
		for (int i = 0; i < resultMods.size(); i++)
		{
			Modulator& mod = *(resultMods[i]);
			targetGenType = mod.GetOutTargetGeneratorType();

			if (mod.GetIOState() != ModIOState::Inputed ||   //跳过没有处于输入状态的
				mod.HavModulatorInput() ||  	             //跳过输入源是调制器的
				targetGenType == GeneratorType::None)        //跳过输出目标不是生成器目标的
				continue;

			oldAmount = mod.GetAmount();
			//
			//执行调制，获取调制值
			mod.SetAmount(resultModAmounts[i]);
			modValue = mod.Output();
			genAmount = resultGens.GetAmount(targetGenType);
			if (mod.GetOutOp() == OutOpType::Add) { modValue += genAmount.amount; }
			else if (mod.GetOutOp() == OutOpType::Mul) { modValue *= genAmount.amount; }
			genAmount.amount = modValue;
			resultGens.SetAmount(targetGenType, genAmount);
			//
			mod.SetAmount(oldAmount);
		}
	}


	//设置采样模式
	void ZoneSounder::SetSampleModes()
	{
		loopPlayBack = (LoopPlayBackMode)resultGens.GetAmount(GeneratorType::SampleModes).amount;
		if (loopPlayBack == LoopPlayBackMode::NonLoop)
			isLoopSample = false;
		else if (loopPlayBack == LoopPlayBackMode::LoopEndContinue)
			isLoopSample = true;
		else
			isLoopSample = true;
	}

	//设置基音调
	void ZoneSounder::SetBasePitchMul()
	{
		curtCalBasePitchMul = basePitchMul = CalBasePitchMulByKey(downNoteKey);
	}

	//设置音量衰减值
	void ZoneSounder::SetAttenuation()
	{
		float att = -resultGens.GetAmount(GeneratorType::InitialAttenuation).amount * 0.1f * 0.7f;
		att += virInst->GetChannel()->GetVolumeGain();
		float newAttenDstValue = UnitTransform::DecibelsToGain(att);
		if (attenFadeInfo.dstValue == newAttenDstValue)
			return;

		attenFadeInfo.dstValue = newAttenDstValue;
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
	void ZoneSounder::SetPan()
	{
		float genPan = resultGens.GetAmount(GeneratorType::Pan).amount * 0.1f;

		//声向音量倍率调制
		channelGainFadeInfo[0].dstValue = (50 - genPan) * 0.01f;
		channelGainFadeInfo[1].dstValue = (1 - channelGainFadeInfo[0].dstValue);

		//声向曲线调制音量倍率
		channelGainFadeInfo[0].dstValue =  (float)FastSin(channelGainFadeInfo[0].dstValue * M_PI / 2);
		channelGainFadeInfo[1].dstValue =  (float)FastSin(channelGainFadeInfo[1].dstValue * M_PI / 2);

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
	void ZoneSounder::SetChorusDepth()
	{
		float newChorusDstValue = resultGens.GetAmount(GeneratorType::ChorusEffectsSend).amount * 0.001f;
		if (chorusDepthFadeInfo.dstValue == newChorusDstValue)
			return;

		chorusDepthFadeInfo.dstValue = newChorusDstValue;
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
	void ZoneSounder::SetSampleStartIdx()
	{
		int startIdx = sample != nullptr ? sample->startIdx : 0;
		sampleStartIdx =
			startIdx
			+ resultGens.GetAmount(GeneratorType::StartAddrsOffset).amount
			+ resultGens.GetAmount(GeneratorType::StartAddrsCoarseOffset).amount * 32768;
	}

	//设置样本结束位置
	void ZoneSounder::SetSampleEndIdx()
	{
		int endIdx = sample != nullptr ? sample->endIdx : 0;
		sampleEndIdx =
			endIdx
			+ resultGens.GetAmount(GeneratorType::EndAddrsOffset).amount
			+ resultGens.GetAmount(GeneratorType::EndAddrsCoarseOffset).amount * 32768;
	}

	//设置样本循环起始位置
	void ZoneSounder::SetSampleStartLoopIdx()
	{
		int startloopIdx = sample != nullptr ? sample->startloopIdx : 0;
		sampleStartLoopIdx =
			startloopIdx
			+ resultGens.GetAmount(GeneratorType::StartloopAddrsOffset).amount
			+ resultGens.GetAmount(GeneratorType::StartloopAddrsCoarseOffset).amount * 32768;
	}

	//设置样本循环结束位置
	void ZoneSounder::SetSampleEndLoopIdx()
	{
		int endloopIdx = sample != nullptr ? sample->endloopIdx : 0;
		sampleEndLoopIdx =
			endloopIdx
			+ resultGens.GetAmount(GeneratorType::EndloopAddrsOffset).amount
			+ resultGens.GetAmount(GeneratorType::EndloopAddrsCoarseOffset).amount * 32768;
	}


	//设置音量包络线控制参数
	void ZoneSounder::SetVolEnv()
	{
		volEnv->delaySec = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::DelayVolEnv));
		volEnv->attackSec = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::AttackVolEnv));
		volEnv->sustain = -resultGens.GetAmount(GeneratorType::SustainVolEnv).amount * 0.1f;
		volEnv->releaseSec = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::ReleaseVolEnv));
		volEnv->keyToHold = resultGens.GetAmount(GeneratorType::KeynumToVolEnvHold).amount;
		volEnv->keyToDecay = resultGens.GetAmount(GeneratorType::KeynumToVolEnvDecay).amount;
		volEnv->holdTimecents = resultGens.GetAmount(GeneratorType::HoldVolEnv).amount;
		volEnv->decayTimecents = resultGens.GetAmount(GeneratorType::DecayVolEnv).amount;
		volEnv->Reset(sec);
	}

	//设置调制包络线控制参数
	void ZoneSounder::SetModEnv()
	{
		modEnv->delaySec = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::DelayModEnv));
		modEnv->attackSec = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::AttackModEnv));
		modEnv->sustain = 1 - resultGens.GetAmount(GeneratorType::SustainModEnv).amount * 0.001f;
		modEnv->releaseSec = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::ReleaseModEnv));
		modEnv->keyToHold = resultGens.GetAmount(GeneratorType::KeynumToModEnvHold).amount;
		modEnv->keyToDecay = resultGens.GetAmount(GeneratorType::KeynumToModEnvDecay).amount;
		modEnv->holdTimecents = resultGens.GetAmount(GeneratorType::HoldModEnv).amount;
		modEnv->decayTimecents = resultGens.GetAmount(GeneratorType::DecayModEnv).amount;
		modEnv->Reset(sec);
	}

	//设置lfo或者包络的调制信息
	void ZoneSounder::SetLfoEnvModInfo(
		vector<LfoModInfo>& infos, GeneratorType genType, 
		UnitTransformCallBack unitTransform, float amountMul)
	{
		short amount = resultGens.GetAmount(genType).amount;
		if (amount == 0) {
			auto it = infos.begin();
			for (; it != infos.end(); it++) {
				if ((*it).genType == genType) {
					infos.erase(it);
					break;
				}
			}
			return;
		}

		//
		for (int i = 0; i < infos.size(); i++) {
			if (infos[i].genType == genType) {
				infos[i].modValue = amount * amountMul;
				return;
			}
		}

		//
		LfoModInfo info;
		info.genType = genType;
		info.unitTransform = unitTransform;
		info.modValue = amount * amountMul;
		infos.push_back(info);
	}

	//设置颤音lfo调制的音调
	void ZoneSounder::SetVibLfoToPitch()
	{
		SetLfoEnvModInfo(vibLfoInfos, GeneratorType::VibLfoToPitch, UnitTransform::CentsToMul);
	}

	//设置颤音lfo的延迟
	void ZoneSounder::SetDelayVibLFO()
	{
		vibLfo->delay = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::DelayVibLFO));
	}

	//设置颤音lfo的频率
	void ZoneSounder::SetFreqVibLFO()
	{
		vibLfo->freq = UnitTransform::CentsToHertz(resultGens.GetAmount(GeneratorType::FreqVibLFO));
	}

	//设置modlfo调制截止频率
	void ZoneSounder::SetModLfoToFilterFc()
	{
		SetLfoEnvModInfo(modLfoInfos, GeneratorType::ModLfoToFilterFc, nullptr);
	}

	//设置modlfo调制音调
	void ZoneSounder::SetModLfoToPitch()
	{
		SetLfoEnvModInfo(modLfoInfos, GeneratorType::ModLfoToPitch, UnitTransform::CentsToMul);
	}

	//设置modlfo调制音量
	void ZoneSounder::SetModLfoToVolume()
	{
		SetLfoEnvModInfo(modLfoInfos, GeneratorType::ModLfoToVolume, UnitTransform::DecibelsToGain, 0.1f);
	}

	//设置modlfo的延迟
	void ZoneSounder::SetDelayModLFO()
	{
		modLfo->delay = UnitTransform::TimecentsToSec(resultGens.GetAmount(GeneratorType::DelayModLFO));
	}

	//设置modlof的频率
	void ZoneSounder::SetFreqModLFO()
	{
		modLfo->freq = UnitTransform::CentsToHertz(resultGens.GetAmount(GeneratorType::FreqModLFO));
	}

	//设置调制包络线调制音调
	void ZoneSounder::SetModEnvToPitch()
	{
		SetLfoEnvModInfo(modEnvInfos, GeneratorType::ModEnvToPitch, UnitTransform::CentsToMul);
	}

	//设置调制包络线调制截止频率
	void ZoneSounder::SetModEnvToFilterFc()
	{
		SetLfoEnvModInfo(modEnvInfos, GeneratorType::ModEnvToFilterFc, nullptr);
	}

	
	// 设置双二阶Biquad滤波器参数
	void ZoneSounder::SetBiquadParams()
	{
		fcCents = resultGens.GetAmount(GeneratorType::InitialFilterFc).amount;
		if (fcCents == 13500) {
			isActiveLowPass = false;
			Q = 0.707f;
			biquadFcCents = fcCents;
			return;
		}
		//
		isActiveLowPass = true;
		Q = resultGens.GetAmount(GeneratorType::InitialFilterQ).amount * 0.1f;
		Q = UnitTransform::ResonanceDbToFilterQ(Q);
		double qDiff = biquadQ - Q;
		if (fcCents != biquadFcCents ||
			(qDiff < -0.0001f || qDiff > 0.0001f))
		{
			biquadSampleRate = tau->GetSampleProcessRate();
			biquadFcCents = fcCents;
			float fc = UnitTransform::CentsToHertz(fcCents);
			biquadQ = Q;
			biquad->setup(biquadSampleRate, fc, biquadQ);
		}
	}

	// 设置延音踏板开关值
	void ZoneSounder::SetSustainPedalOnOff()
	{
		float value = resultGens.GetAmount(GeneratorType::SustainPedalOnOff).amount;
		if (value == -1)
			return;

		bool isHold = value < 0.5f ? false : true;
		if (isDownNoteKey == false)
			return;

		//设置是否保持按键状态
		isHoldDownKey = isHold;

		if (!isHoldDownKey && isNeedOffKey && isDownNoteKey)
			OffKey(127);

	}

	// 设置压力值
	void ZoneSounder::SetPressure()
	{
		float value = resultGens.GetAmount(GeneratorType::Pressure).amount;
		if (value == -1)
			return;

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
	float ZoneSounder::CalBasePitchMulByKey(int key)
	{
		float sampleRate = sample != nullptr ? sample->sampleRate : tau->GetSampleProcessRate();
		float rateAdjustMul = sampleRate / tau->GetSampleProcessRate();   //频率差矫正倍率

		//
		float scaleTuning = resultGens.GetAmount(GeneratorType::ScaleTuning).amount;

		//
		float overridingRootKey = resultGens.GetAmount(GeneratorType::OverridingRootKey).amount;
		if (overridingRootKey < 0 || resultGens.IsEmpty(GeneratorType::OverridingRootKey))
			overridingRootKey = sample != nullptr ? sample->GetOriginalPitch() : key;

		//按键和根音符之间偏移几个半音
		float semit = (key - overridingRootKey) * scaleTuning * 0.01f;

		//校音处理
		float sampleCentPitchCorrection = sample != nullptr ? sample->GetCentPitchCorrection() : 0;
		float fineTune = resultGens.GetAmount(GeneratorType::FineTune).amount + sampleCentPitchCorrection;  //一个组合的音分偏移值校正
		semit += (resultGens.GetAmount(GeneratorType::CoarseTune).amount + fineTune / 100.0f);

		//限制了按键相对overridingRootKey之间半音的差值的最大数量
		//if (semit > 72)
			//semit = 72;

		//半音转倍率
		float baseMul = UnitTransform::SemitoneToMul(semit);

		//偏移半音数量转化为频率偏移倍率
		return baseMul * rateAdjustMul;
	}

	// 设置力度
	void ZoneSounder::SetVelocity(float vel)
	{
		//如果设置了固定力度，那么外置力度将不起效
		if (!resultGens.IsEmpty(GeneratorType::Velocity))
			vel = resultGens.GetAmount(GeneratorType::Velocity).amount;

		if (sampleGen != nullptr)
			velocity = vel / 127.0f;
		else
			velocity = vel / sample->velocity; //归一化力度到浮点数
		//暂定
		//this->velocity = powf(this->velocity, 2.0f);
		velocityFadeInfo.dstValue = velocity;
	}



	// 使用滑音
	void ZoneSounder::UsePortamento()
	{
		if (virInst->UsePortamento() && virInst->GetPortaTime() != 0)
		{
			ZoneSounder* zoneSounder = virInst->FindLastSameZone(instZone);
			if (zoneSounder == nullptr)
			{
				KeySounder* lastKeySounder = virInst->GetLastOnKeyStateSounder();
				if (lastKeySounder == nullptr && virInst->AlwaysUsePortamento())
					lastKeySounder = virInst->GetLastKeySounder();
				
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
				lastKeyBasePitchMul = zoneSounder->GetCurtCalBasePitchMul();
			}
		}
	}


	// 连奏模式（Legato Mode）：在合成器中，启用Legato模式时，
	// 新触发的音符不会重新触发包络，而是保持之前的包络状态，从而平滑过渡。
	// 在合成器中，启用连奏模式后，新音符不会重置包络，而是继承前一个音符的包络状态，从而保持连贯性。
	// 例如，若前一个音符处于释音阶段，新音符的包络会从当前值继续而非重新触发
	void ZoneSounder::UseLegato()
	{
		//根据当前发音区域的乐器的最后一个keySounder的查找与当前ZoneSounder对应的
	    //instZone,如果找到并且处于Sustain阶段，将直接使当前发音区域从Sustain阶段发音
		if (virInst->UseLegato() && virInst->GetLastOnKeyStateSounder() != nullptr)
		{
			KeySounder* keySounder = virInst->GetLastOnKeyStateSounder();
			auto zoneSounders = keySounder->GetZoneSounders();
			for (int i = 0; i < zoneSounders.size(); i++)
			{
				//为同一个发音区域
				if (zoneSounders[i]->instZone == instZone)
				{
					isActiveLegato = true;
					volEnv->SetBaseSec(zoneSounders[i]->volEnv->GetCurtSec());
				}
			}

			if (!isActiveLegato)
			{
				volEnv->SetBaseSec(zoneSounders[0]->volEnv->GetCurtSec());
			}
		}
	}


	void ZoneSounder::SetFrameBuffer(float* leftChannelBuf, float* rightChannelBuf)
	{
		leftChannelSamples = leftChannelBuf;
		rightChannelSamples = rightChannelBuf;
	}


	//处理不能听到发声的滞留区域，使其结束发音
	void ZoneSounder::EndBlockSound()
	{
		//滞留发声区域判定
		//有的区域由于一直不发送0ffkey命令，导致发声区域一直不被关闭，但又没有发声
		//如果不是实时控制，而是在midi播放时，这些发音区域是可以直接关闭的
		//如果是实时控制的，不直接关闭的原因是可能这些区域通过ZoneModulation调制包络后，包络延音值会被提高，而重新发音
		if (!isRealtimeControl &&
			(GetVolEnvStage() == EnvStage::Decay ||
				GetVolEnvStage() == EnvStage::Sustain) &&
			IsZeroValueRenderChannelBuffer())
		{
			EndSound();
		}
	}

	//处理不能听到发声的滞留区域，使其结束发音
	void ZoneSounder::EndSoundLevel(int endLevel)
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
	bool ZoneSounder::IsZeroValueRenderChannelBuffer()
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
	bool ZoneSounder::IsEffectSoundStop()
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
	void ZoneSounder::Render()
	{
		if (isSoundEnd)
			return;

		if (isSampleProcessEnd)
		{
			isSoundEnd = true;
			if (tau->UseZoneInnerChorusEffect() && chorusDepth > 0.0001)
			{
				for (int i = 0; i < tau->GetChildFrameSampleCount(); i++)
				{
					chorus[0]->Process(0);
					chorus[1]->Process(0);
					leftChannelSamples[i] = (chorus[0]->GetLeft() + chorus[1]->GetLeft())*0.5f;
					rightChannelSamples[i] = (chorus[0]->GetRight() + chorus[1]->GetRight()) * 0.5f;
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
		float pitchOffsetMulStart;
		float pitchOffsetMulEnd;
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
			if (isActiveLowPass) 
				ResetLowPassFilter(endSec);
			
			//采样音调处理
			pitchOffsetMulStart = LfosAndEnvsPitch(sec);
			pitchOffsetMulEnd =  LfosAndEnvsPitch(endSec);

			//处理滑音
			if (isActivePortamento)
				curtCalBasePitchMul = PortamentoProcess(sec);

			if (renderQuality == RenderQuality::Good || renderQuality == RenderQuality::Fast)
			{
				//此时计算的volGain会处理blockSamples(预设64个采样点)个数据，粒度比较粗糙，数据有可能不够平缓，而导致卡顿音  
				//此时通过一个时间上的过渡处理，来平缓数据的粗糙度
				startVolGain = volGain = LfosAndEnvsVolume(sec) * atten_mul_vel;
				endVolGain = LfosAndEnvsVolume(endSec) * atten_mul_vel;
			}

			//
			while (blockSamples > 0)
			{
				pitchOffsetMul = pitchOffsetMulStart * (1 - a) + pitchOffsetMulEnd * a;
				curtPitchMul = curtCalBasePitchMul * pitchOffsetMul;
				sampleValue = NextAdjustPitchSample(curtPitchMul);

				//低通滤波处理
				if (isActiveLowPass) 
					sampleValue = biquad->filter(sampleValue);
				
				//
				if (renderQuality == RenderQuality::Good ||
					renderQuality == RenderQuality::Fast)
				{
					//通过一个采样位置的平缓过渡处理，来平缓精度不足带来的数据阶梯跳跃
					if (startVolGain != endVolGain)
						volGain = startVolGain*(1 - a) + endVolGain * a;
					
				}
				else
				{
					volGain = LfosAndEnvsVolume(sec) * atten_mul_vel;
				}

				volGainSampleValue = volGain * sampleValue;
				leftChannelSamples[idx] = channelGain[0] * volGainSampleValue;
				rightChannelSamples[idx] = channelGain[1] * volGainSampleValue;

				if (tau->UseZoneInnerChorusEffect() && chorusDepth > 0.0001)
					ChorusProcessSample(idx);

				idx++;
				processedSampleCount++;
				sec = processedSampleCount * invSampleProcessRate;

				if ((isSampleProcessEnd || volEnv->IsStop()))
				{
					int bufsize = (tau->GetChildFrameSampleCount() - idx) * sizeof(float);
					memset(leftChannelSamples + idx, 0, bufsize);
					memset(rightChannelSamples + idx, 0, bufsize);

					if (tau->UseZoneInnerChorusEffect() && chorusDepth > 0.0001)
					{
						for (int i = idx; i < tau->GetChildFrameSampleCount(); i++)
							ChorusProcessSample(i);
					}

					isDownNoteKey = false;
					isSampleProcessEnd = true;
					return;
				}

				blockSamples--;
				a += invSampleProcessBlockSize;
			}
		}
	}

	//过渡参数值
	void ZoneSounder::FadeParams()
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
	float ZoneSounder::FadeValue(float orgValue, ValueFadeInfo fadeInfo)
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

	void ZoneSounder::ChorusProcessSample(int idx)
	{
		chorus[0]->Process(leftChannelSamples[idx]);
		chorus[1]->Process(rightChannelSamples[idx]);
		leftChannelSamples[idx] = (chorus[0]->GetLeft() + chorus[1]->GetLeft())*0.5f;
		rightChannelSamples[idx] = (chorus[0]->GetRight() + chorus[1]->GetRight()) *0.5f;
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
	float ZoneSounder::NextAdjustPitchSample(float sampleSpeed)
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
		if (sample != nullptr) //如果存在样本，将对样本采样
			return sample->GetValue(prevIntPos) * (1 - a) + sample->GetValue(nextIntPos) * a;
		else if(zoneSampleGen != nullptr) //否则，如果存在动态样本生成器，将通过生成器生成当前值
			return zoneSampleGen->Out(prevIntPos, nextIntPos, a);
		return 0;
	}


	// 滑音处理
	float ZoneSounder::PortamentoProcess(float sec)
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
	void ZoneSounder::ResetLowPassFilter(float computedSec)
	{
		short modFcCents = LfosAndEnvsFc(computedSec);
		fcCompute = fcCents + modFcCents;
		if (fcCompute > 13500) fcCompute = 13500;
		else if (fcCompute < 1500) fcCompute = 1500;
		//
		double qDiff = biquadQ - Q;
		if (fcCompute != biquadFcCents ||
			(qDiff < -0.0001f || qDiff > 0.0001f))
		{
			biquadFcCents = fcCompute;
			biquadQ = Q;
			float fc = UnitTransform::CentsToHertz(biquadFcCents);
			biquad->setup(biquadSampleRate, fc, biquadQ);
		}
	}

	
	// lfos, envs调制音调
	float ZoneSounder::LfosAndEnvsPitch(float sec)
	{
		float result = 1;

		//
		for (int i = 0; i < vibLfoInfos.size(); i++) {
			if (vibLfoInfos[i].genType == GeneratorType::VibLfoToPitch)
			{
				float lfoVal = vibLfo->SinWave(sec);
				lfoVal *= vibLfoInfos[i].modValue;
				if (vibLfoInfos[i].unitTransform != nullptr)
					lfoVal = vibLfoInfos[i].unitTransform(lfoVal);
				result *= lfoVal;
				break;
			}
		}

		//
		for (int i = 0; i < modLfoInfos.size(); i++) {
			if (modLfoInfos[i].genType == GeneratorType::ModLfoToPitch)
			{
				float lfoVal = modLfo->SinWave(sec);
				lfoVal *= modLfoInfos[i].modValue;
				if (modLfoInfos[i].unitTransform != nullptr)
					lfoVal = modLfoInfos[i].unitTransform(lfoVal);
				result *= lfoVal;
				break;
			}
		}

		//
		for (int i = 0; i < modEnvInfos.size(); i++) {
			if (modEnvInfos[i].genType == GeneratorType::ModEnvToPitch)
			{
				float envVal = modEnv->GetEnvValue(sec);
				envVal *= modEnvInfos[i].modValue;
				if (modEnvInfos[i].unitTransform != nullptr)
					envVal = modEnvInfos[i].unitTransform(envVal);
				result *= envVal;
				break;
			}
		}

		return result;
	}


	// lfos, envs调制截至频率(返回值单位:cents)
	short ZoneSounder::LfosAndEnvsFc(float sec)
	{
		short result = 0;

		//
		for (int i = 0; i < modLfoInfos.size(); i++) {
			if (modLfoInfos[i].genType == GeneratorType::ModLfoToFilterFc)
			{
				float lfoVal = modLfo->SinWave(sec);
				lfoVal *= modLfoInfos[i].modValue;
				result += lfoVal;
				break;
			}
		}

		//
		for (int i = 0; i < modEnvInfos.size(); i++) {
			if (modEnvInfos[i].genType == GeneratorType::ModEnvToFilterFc)
			{
				float envVal = modEnv->GetEnvValue(sec);
				envVal *= modEnvInfos[i].modValue;
				result += envVal;
				break;
			}
		}

		return result;
	}


	// lfos, envs调制声音
	float ZoneSounder::LfosAndEnvsVolume(float sec)
	{
		float result = 1;

		//
		for (int i = 0; i < modLfoInfos.size(); i++) {
			if (modLfoInfos[i].genType == GeneratorType::ModLfoToVolume)
			{
				float lfoVal = modLfo->SinWave(sec);
				lfoVal *= modLfoInfos[i].modValue;
				if (modLfoInfos[i].unitTransform != nullptr)
					lfoVal = modLfoInfos[i].unitTransform(lfoVal);
				result *= lfoVal;
				break;
			}
		}

		//
		for (int i = 0; i < volEnvInfos.size(); i++) {
			if (volEnvInfos[i].genType == GeneratorType::VolEnvToVolume)
			{
				float envVal = UnitTransform::DecibelsToGain(volEnv->GetEnvValue(sec));
				envVal *= volEnvInfos[i].modValue;
				if (volEnvInfos[i].unitTransform != nullptr)
					envVal = volEnvInfos[i].unitTransform(envVal);
				result *= envVal;
				break;
			}
		}

		return result;
	}
}
