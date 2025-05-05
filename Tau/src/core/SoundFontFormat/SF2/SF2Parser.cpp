#include"SF2Parser.h"
#include"Synth/SoundFont.h"
#include"Synth/Sample.h"
#include"Synth/Instrument.h"
#include"Synth/Preset.h"
#include"Synth/Zone.h"
#include"Synth/Generator.h"
#include"Synth/UnitTransform.h"
#include"Synth/Modulator.h"

namespace tau
{
	SF2Parser::~SF2Parser() {
		if (sf2 != nullptr){
			delete sf2;
			sf2 = nullptr;
		}

		if (modulators != nullptr) {
			free(modulators);
			modulators = nullptr;
		}
		modulatorBufSize = 0;
		modulatorCount = 0;
	}

	void SF2Parser::Parse(string filePath)
	{
		DEL(sf2);
		sf2 = new SF2(filePath);

		if (sf2 == nullptr || sf2->size == 0) {
			DEL(sf2);
			cout << filePath << "文件解析失败!" << endl;
			return;
		}

		ParseCore();
		DEL(sf2);
	}

	void SF2Parser::ParseCore()
	{
		ParseSampleList();
		ParseInstrumentList();
		ParsePresetList();

		//
		if (modulators != nullptr) {
			free(modulators);
			modulators = nullptr;
		}
		modulatorBufSize = 0;
		modulatorCount = 0;
	}

	//解析样本列表    
	void SF2Parser::ParseSampleList()
	{
		vector<SF2SampleHeader*>& samples = sf2->hydraChunk->shdrSubChunk->samples;
		short* smpls = nullptr;
		byte* sm24 = nullptr;

		if (sf2->soundChunk->smplSubChunk)
			smpls = sf2->soundChunk->smplSubChunk->samples;
		if (sf2->soundChunk->sm24SubChunk)
			sm24 = sf2->soundChunk->sm24SubChunk->samples;

		if (smpls == nullptr)
			return;

		string name;
		short* pcm;
		byte* pcmSM24 = nullptr;
		vector<Sample*> sampleLink;
		struct Info { int linkIdx; Sample* sample; Info(int idx, Sample* s) { linkIdx = idx; sample = s; } };
		vector<Info> sampleLinkInfo;

		for (int i = 0; i < samples.size(); i++)
		{
			pcmSM24 = nullptr;
			name.assign((const char*)samples[i]->sampleName, 20);

			uint32_t start = samples[i]->Start;
			uint32_t end = samples[i]->End;
			size_t len = end - start + 1;
			pcm = smpls + start;
			if (sm24) { pcmSM24 = sm24 + start/2; }

			Sample* oreSample = sf->AddSample(name, pcm, len, pcmSM24);
			oreSample->startIdx = 0;
			oreSample->endIdx = end - start;
			oreSample->startloopIdx = samples[i]->LoopStart - start;
			oreSample->endloopIdx = samples[i]->LoopEnd - start;
			oreSample->centPitchCorrection = samples[i]->PitchCorrection;
			oreSample->sampleRate = (float)samples[i]->SampleRate;
			oreSample->originalPitch = samples[i]->OriginalKey;

			if (samples[i]->SampleType != SF2SampleLink::MonoSample &&
				samples[i]->SampleType != SF2SampleLink::RomMonoSample)
			{
				sampleLinkInfo.push_back(Info(samples[i]->SampleLink, oreSample));
			}

			switch (samples[i]->SampleType)
			{
			case SF2SampleLink::LeftSample: oreSample->sampleType = SampleType::LeftSample; break;
			case SF2SampleLink::LinkedSample: oreSample->sampleType = SampleType::LinkedSample; break;
			case SF2SampleLink::MonoSample: oreSample->sampleType = SampleType::MonoSample; break;
			case SF2SampleLink::RightSample: oreSample->sampleType = SampleType::RightSample; break;
			case SF2SampleLink::RomLeftSample: oreSample->sampleType = SampleType::RomLeftSample; break;
			case SF2SampleLink::RomLinkedSample: oreSample->sampleType = SampleType::RomLinkedSample; break;
			case SF2SampleLink::RomMonoSample: oreSample->sampleType = SampleType::RomMonoSample; break;
			case SF2SampleLink::RomRightSample: oreSample->sampleType = SampleType::RomRightSample; break;
			}
		}


		vector<Sample*>& sampleList = *sf->GetSampleList();

		for (int i = 0; i < sampleLinkInfo.size(); i++)
		{
			sampleLinkInfo[i].sample->SetSampleLink(sampleList[sampleLinkInfo[i].linkIdx]);
		}
	}


	//解析乐器列表    
	void SF2Parser::ParseInstrumentList()
	{
		vector<SF2Instrument*>& insts = sf2->hydraChunk->instSubChunk->instruments;
		string name;
		Zone* zone;

		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->imodSubChunk->modulators;

		//
		for (size_t i = 0; i < insts.size() - 1; i++)
		{
			name.assign((const char*)insts[i]->instrumentName, 20);
			Instrument* tauInst = sf->AddInstrument(name);
			
			//解析乐器的所有区域
			for (size_t j = insts[i]->InstrumentBagIndex; j < insts[i + 1]->InstrumentBagIndex; j++)
			{
				zone = ParseInstZoneGeneratorList((int)j, tauInst);
				ParseInstZoneModulatorList(zone, (int)j, tauInst);
			}

		}
	}

	//解析乐器区域的生成器列表
	Zone* SF2Parser::ParseInstZoneGeneratorList(int bagIdx, Instrument* tauInst)
	{
		//sf区域列表
		vector<SF2Bag*>& bags = sf2->hydraChunk->ibagSubChunk->bags;
		vector<SF2GeneratorList*>& sf2Gens = sf2->hydraChunk->igenSubChunk->generators;
		vector<Sample*>& sampleList = *sf->GetSampleList();
		Zone* zone;

		int genStart = bags[bagIdx]->GeneratorIndex;
		int genEnd = bags[bagIdx + 1]->GeneratorIndex - 1;

		if (genEnd < genStart)
			return nullptr;

		if (sf2Gens[genEnd]->Generator != SF2Generator::SampleID)
		{
			zone = tauInst->GetGlobalZone();
		}
		else
		{
			int sampleIdx = sf2Gens[genEnd]->GeneratorAmount.UAmount;
			zone = sf->SampleBindToInstrument(sampleList[sampleIdx], tauInst);
		}

		if (zone == nullptr)
			return nullptr;


		//设置生成器列表数据
		GeneratorList& genList = zone->GetGens();
		genList.SetZoneType(ZoneType::Instrument);
		GeneratorAmount genAmount;
		SF2GeneratorAmount sf2GenAmount;
		for (int i = genStart; i <= genEnd; i++)
		{
			GeneratorType genType = (GeneratorType)sf2Gens[i]->Generator;
			sf2GenAmount = sf2Gens[i]->GeneratorAmount;
			genAmount.amount = sf2GenAmount.Amount;
			genList.SetAmount(genType, genAmount);
		}

		return zone;
	}

	//解析乐器区域的调制器列表
	void SF2Parser::ParseInstZoneModulatorList(Zone* zone, int bagIdx, Instrument* tauInst)
	{
		if (zone == nullptr || !isParseModulator)
			return;

		vector<SF2Bag*>& bags = sf2->hydraChunk->ibagSubChunk->bags;
		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->imodSubChunk->modulators;
		int modStart = bags[bagIdx]->ModulatorIndex;
		int modEnd = bags[bagIdx + 1]->ModulatorIndex - 1;

		if (modEnd < modStart)
			return;

		//
		int32_t curtModCount = modEnd - modStart + 1;
		if (curtModCount == 0)
			return;

		if (modulatorBufSize < curtModCount) {
			modulatorBufSize = curtModCount * 2;
			modulators = (Modulator**)realloc(modulators, modulatorBufSize * sizeof(Modulator*));
			if (modulators == nullptr){
				modulatorBufSize = 0;
				modulatorCount = 0;
				throw "未分配modulators内存!";
			}
		}	
		modulatorCount = 0;
		

		//
		for (int i = modStart; i <= modEnd; i++)
			CreateModulator(mods, modStart, i, zone);

		for (int i = modStart; i <= modEnd; i++)
			ResetOutTargetModulator(mods, modStart, i);
	}

	// 解析预设列表
	void SF2Parser::ParsePresetList()
	{
		vector<SF2PresetHeader*>& presets = sf2->hydraChunk->phdrSubChunk->presets;
		string name;
		Zone* zone;

		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->pmodSubChunk->modulators;

		//
		for (size_t i = 0; i < presets.size() - 1; i++)
		{
			name.assign((const char*)presets[i]->presetName, 20);
			Preset* tauPreset = sf->AddPreset(name, presets[i]->Bank, 0, presets[i]->Preset);

			for (size_t j = presets[i]->PresetBagIndex; j < presets[i + 1]->PresetBagIndex; j++)
			{
				zone = ParsePresetZoneGeneratorList((int)j, tauPreset);
				ParsePresetZoneModulatorList(zone, (int)j, tauPreset);
			}
		}

	}

	//解析预设区域的生成器列表
	Zone* SF2Parser::ParsePresetZoneGeneratorList(int bagIdx, Preset* tauPreset)
	{
		//sf区域列表
		vector<SF2Bag*>& bags = sf2->hydraChunk->pbagSubChunk->bags;
		vector<SF2GeneratorList*>& sf2Gens = sf2->hydraChunk->pgenSubChunk->generators;
		vector<Instrument*>& instList = *sf->GetInstrumentList();

		Zone* zone;

		int genStart = bags[bagIdx]->GeneratorIndex;
		int genEnd = bags[bagIdx + 1]->GeneratorIndex - 1;

		if (genEnd < genStart)
			return nullptr;

		if (sf2Gens[genEnd]->Generator != SF2Generator::Instrument)
		{
			zone = tauPreset->GetGlobalZone();
		}
		else
		{
			int instIdx = sf2Gens[genEnd]->GeneratorAmount.UAmount;
			zone = sf->InstrumentBindToPreset(instList[instIdx], tauPreset);
		}

		if (zone == nullptr)
			return nullptr;

		//设置生成器列表数据
		GeneratorList& genList = zone->GetGens();
		genList.SetZoneType(ZoneType::Preset);
		GeneratorAmount genAmount;
		SF2GeneratorAmount sf2GenAmount;
		for (int i = genStart; i <= genEnd; i++)
		{
			GeneratorType genType = (GeneratorType)sf2Gens[i]->Generator;
			sf2GenAmount = sf2Gens[i]->GeneratorAmount;
			genAmount.amount = sf2GenAmount.Amount;
			genList.SetAmount(genType, genAmount);
		}

		return zone;
	}

	//解析预设区域的调制器列表
	void SF2Parser::ParsePresetZoneModulatorList(Zone* zone, int bagIdx, Preset* orePreset)
	{
		if (zone == nullptr || !isParseModulator)
			return;

		vector<SF2Bag*>& bags = sf2->hydraChunk->pbagSubChunk->bags;
		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->pmodSubChunk->modulators;


		int modStart = bags[bagIdx]->ModulatorIndex;
		int modEnd = bags[bagIdx + 1]->ModulatorIndex - 1;

		if (modEnd < modStart)
			return;

		//
		int32_t curtModCount = modEnd - modStart + 1;
		if (curtModCount == 0)
			return;

		if (modulatorBufSize < curtModCount) {
			modulatorBufSize = curtModCount * 2;
			modulators = (Modulator**)realloc(modulators, modulatorBufSize * sizeof(Modulator*));
			if (modulators == nullptr) {
				modulatorBufSize = 0;
				modulatorCount = 0;
				throw "未分配modulators内存!";
			}
		}
		modulatorCount = 0;
		//
		for (int i = modStart; i <= modEnd; i++)
			CreateModulator(mods, modStart, i, zone);

		for (int i = modStart; i <= modEnd; i++)
			ResetOutTargetModulator(mods, modStart, i);
	}


	//生成调制器
	void SF2Parser::CreateModulator(vector<SF2ModulatorList*>& mods, int start, int idx, Zone* zone)
	{
		Modulator* modulator = new Modulator();
		uint16_t source;
		SF2ModulatorSource sfMod;

		source = mods[idx]->ModulatorSource;
		sfMod.index = source & 0x7f;
		sfMod.midiControllerFlag = (source >> 7) & 0x1f;
		sfMod.direction = (source >> 8) & 0x1f;
		sfMod.polarities = (source >> 9) & 0x1f;
		sfMod.type = (source >> 10) & 0x3f;

		ModInputType inputType = ModInputType::Preset;
		if (sfMod.midiControllerFlag == 1)
			inputType = ModInputType::MidiController;
		else
			inputType = ModInputType::Preset;

		int minValue = 0;
		int maxValue = 127;
		if (inputType == ModInputType::Preset && 
			(ModInputPreset)sfMod.index == ModInputPreset::NoController) {
			maxValue = 1;
		}

		ModSourceTransformType modSourceTransType = (ModSourceTransformType)sfMod.type;
		modulator->SetSourceTransform(0, modSourceTransType, sfMod.direction, sfMod.polarities);
		modulator->AddInputInfo(inputType, (ModInputPreset)sfMod.index, (MidiControllerType)sfMod.index, 0, minValue, maxValue);


		//
		source = mods[idx]->ModulatorAmountSource;
		sfMod.index = source & 0x7f;
		sfMod.midiControllerFlag = (source >> 7) & 0x1f;
		sfMod.direction = (source >> 8) & 0x1f;
		sfMod.polarities = (source >> 9) & 0x1f;
		sfMod.type = (source >> 10) & 0x3f;

		if ((source >> 15) == 0) {
			inputType = ModInputType::Preset;
			if (sfMod.midiControllerFlag == 1)
				inputType = ModInputType::MidiController;
			else
				inputType = ModInputType::Preset;

			maxValue = 127;
			if (inputType == ModInputType::Preset &&
				(ModInputPreset)sfMod.index == ModInputPreset::NoController) {
				maxValue = 1;
			}

			modSourceTransType = (ModSourceTransformType)sfMod.type;
			modulator->SetSourceTransform(1, modSourceTransType, sfMod.direction, sfMod.polarities);
			modulator->AddInputInfo(inputType, (ModInputPreset)sfMod.index, (MidiControllerType)sfMod.index, 1, minValue, maxValue);
		}

		//
		modulator->SetAmount(mods[idx]->ModulatorAmount);
		modulator->SetAbsType((ModTransformType)mods[idx]->ModulatorTransform);

		//
		uint16_t dest = (uint16_t)mods[idx]->ModulatorDestination;
		if ((dest >> 15) == 1) {
			uint16_t destIdx = dest & 0x7fff;
			if (destIdx < modulatorCount)
				modulator->SetOutTarget(modulators[destIdx - start], 0);
		}
		else {
			modulator->SetOutTarget((GeneratorType)mods[idx]->ModulatorDestination);
		}

		Modulator* oldSameMod = GetZoneSameModulator(modulator, zone);
		if (oldSameMod == nullptr) {
			zone->AddModulator(modulator);
			modulators[idx - start] = modulator;
		}
		else {
			modulators[idx - start] = oldSameMod;
			delete modulator;
		}

		modulatorCount++;
	}

	//重设输出目标调制器
	void SF2Parser::ResetOutTargetModulator(vector<SF2ModulatorList*>& mods, int start, int idx)
	{
		uint16_t dest = (uint16_t)mods[idx]->ModulatorDestination;
		if ((dest >> 15) == 1) {
			int destIdx = dest & 0x7fff;
			modulators[idx - start]->SetOutTarget(modulators[destIdx - start], 0);
		}
	}


	//获取区域中前一个相同的调制器
	Modulator* SF2Parser::GetZoneSameModulator(Modulator* modulator, Zone* zone)
	{
		bool isSame = false;
		vector<Modulator*>& modulators = zone->GetModulators();
		for (int i = 0; i < modulators.size(); i++)
		{
			isSame = modulators[i]->IsSame(modulator);
			if (isSame)
				return modulators[i];
		}

		return nullptr;
	}

}
