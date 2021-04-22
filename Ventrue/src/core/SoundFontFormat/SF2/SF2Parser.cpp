#include"SF2Parser.h"
#include"Synth/Ventrue.h"
#include"Synth/Sample.h"
#include"Synth/Instrument.h"
#include"Synth/Preset.h"
#include"Synth/Region.h"
#include"Synth/Generator.h"
#include"Synth/UnitTransform.h"
#include"Synth/Modulator.h"

namespace ventrue
{
	void SF2Parser::Parse(string filePath)
	{
		DEL(sf2);
		sf2 = new SF2(filePath);
		ParseSampleList();
		ParseInstrumentList();
		ParsePresetList();
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
			pcm = smpls + start;
			if (sm24) { pcmSM24 = sm24 + i; }

			Sample* oreSample = ventrue->AddSample(name, pcm, end - start + 1, pcmSM24);
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


		SampleList& sampleList = *ventrue->GetSampleList();

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
		Region* region;

		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->imodSubChunk->modulators;

		if (modulators != nullptr)
			delete[] modulators;

		if (mods.size() > 0) {
			modulators = new Modulator * [mods.size()];
			modulatorCount = mods.size();
		}

		//
		for (size_t i = 0; i < insts.size() - 1; i++)
		{
			name.assign((const char*)insts[i]->instrumentName, 20);
			Instrument* oreInst = ventrue->AddInstrument(name);
			for (size_t j = insts[i]->InstrumentBagIndex; j < insts[i + 1]->InstrumentBagIndex; j++)
			{
				region = ParseInstRegionGeneratorList((int)j, oreInst);
				ParseInstRegionModulatorList(region, (int)j, oreInst);
			}
		}

		if (modulators != nullptr) {
			delete[] modulators;
			modulators = nullptr;
		}
	}

	//解析乐器区域的生成器列表
	Region* SF2Parser::ParseInstRegionGeneratorList(int bagIdx, Instrument* oreInst)
	{
		//sf区域列表
		vector<SF2Bag*>& bags = sf2->hydraChunk->ibagSubChunk->bags;
		vector<SF2GeneratorList*>& gens = sf2->hydraChunk->igenSubChunk->generators;
		SampleList& sampleList = *ventrue->GetSampleList();
		Region* region;

		int genStart = bags[bagIdx]->GeneratorIndex;
		int genEnd = bags[bagIdx + 1]->GeneratorIndex - 1;

		if (genEnd < genStart)
			return nullptr;

		if (gens[genEnd]->Generator != SF2Generator::SampleID)
		{
			region = oreInst->GetGlobalRegion();
		}
		else
		{
			int sampleIdx = gens[genEnd]->GeneratorAmount.UAmount;
			region = ventrue->SampleBindToInstrument(sampleList[sampleIdx], oreInst);
		}

		if (region == nullptr)
			return nullptr;

		SetGenList(*region->GetGenList(), gens, genStart, genEnd, RegionType::Insttrument);
		return region;
	}

	//解析乐器区域的调制器列表
	void SF2Parser::ParseInstRegionModulatorList(Region* region, int bagIdx, Instrument* oreInst)
	{
		if (region == nullptr)
			return;

		vector<SF2Bag*>& bags = sf2->hydraChunk->ibagSubChunk->bags;
		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->imodSubChunk->modulators;
		int modStart = bags[bagIdx]->ModulatorIndex;
		int modEnd = bags[bagIdx + 1]->ModulatorIndex - 1;

		if (modEnd < modStart)
			return;

		for (int i = modStart; i <= modEnd; i++)
		{
			CreateModulator(mods, i, region);
		}
	}

	// 解析预设列表
	void SF2Parser::ParsePresetList()
	{
		vector<SF2PresetHeader*>& presets = sf2->hydraChunk->phdrSubChunk->presets;
		string name;
		Region* region;

		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->pmodSubChunk->modulators;

		if (modulators != nullptr)
			delete[] modulators;

		if (mods.size() > 0) {
			modulators = new Modulator * [mods.size()];
			modulatorCount = mods.size();
		}

		//
		for (size_t i = 0; i < presets.size() - 1; i++)
		{
			name.assign((const char*)presets[i]->presetName, 20);
			Preset* orePreset = ventrue->AddPreset(name, presets[i]->Bank, 0, presets[i]->Preset);

			if (i + 1 >= presets.size())
				break;

			for (size_t j = presets[i]->PresetBagIndex; j < presets[i + 1]->PresetBagIndex; j++)
			{
				region = ParsePresetRegionGeneratorList((int)j, orePreset);
				ParsePresetRegionModulatorList(region, (int)j, orePreset);
			}

		}

		if (modulators != nullptr) {
			delete[] modulators;
			modulators = nullptr;
		}
	}

	//解析预设区域的生成器列表
	Region* SF2Parser::ParsePresetRegionGeneratorList(int bagIdx, Preset* orePreset)
	{
		//sf区域列表
		vector<SF2Bag*>& bags = sf2->hydraChunk->pbagSubChunk->bags;
		vector<SF2GeneratorList*>& gens = sf2->hydraChunk->pgenSubChunk->generators;
		InstrumentList& instList = *ventrue->GetInstrumentList();

		Region* region;

		int genStart = bags[bagIdx]->GeneratorIndex;
		int genEnd = bags[bagIdx + 1]->GeneratorIndex - 1;

		if (genEnd < genStart)
			return nullptr;

		if (gens[genEnd]->Generator != SF2Generator::Instrument)
		{
			region = orePreset->GetGlobalRegion();
		}
		else
		{
			int instIdx = gens[genEnd]->GeneratorAmount.UAmount;
			region = ventrue->InstrumentBindToPreset(instList[instIdx], orePreset);
		}

		if (region == nullptr)
			return nullptr;

		SetGenList(*region->GetGenList(), gens, genStart, genEnd, RegionType::Preset);
		return region;
	}

	//解析预设区域的调制器列表
	void SF2Parser::ParsePresetRegionModulatorList(Region* region, int bagIdx, Preset* orePreset)
	{
		if (region == nullptr)
			return;

		vector<SF2Bag*>& bags = sf2->hydraChunk->pbagSubChunk->bags;
		vector<SF2ModulatorList*>& mods = sf2->hydraChunk->pmodSubChunk->modulators;


		int modStart = bags[bagIdx]->ModulatorIndex;
		int modEnd = bags[bagIdx + 1]->ModulatorIndex - 1;

		if (modEnd < modStart)
			return;

		for (int i = modStart; i <= modEnd; i++)
		{
			CreateModulator(mods, i, region);
		}
	}

	//
	void SF2Parser::SetGenList(GeneratorList& genList, vector<SF2GeneratorList*>& sf2Gens, int start, int end, RegionType regionType)
	{

		SF2GeneratorAmount genAmount;

		for (int i = start; i <= end; i++)
		{
			genAmount = sf2Gens[i]->GeneratorAmount;
			float val = (float)genAmount.Amount;

			switch (sf2Gens[i]->Generator)
			{
			case SF2Generator::KeyRange:
				genList.SetAmountRange(GeneratorType::KeyRange, genAmount.ranges.byLo, genAmount.ranges.byHi);
				break;
			case SF2Generator::VelRange:
				genList.SetAmountRange(GeneratorType::VelRange, genAmount.ranges.byLo, genAmount.ranges.byHi);
				break;

			case SF2Generator::StartAddrsOffset: genList.SetAmount(GeneratorType::StartAddrsOffset, val); break;
			case SF2Generator::EndAddrsOffset: genList.SetAmount(GeneratorType::EndAddrsOffset, val); break;
			case SF2Generator::StartloopAddrsOffset: genList.SetAmount(GeneratorType::StartloopAddrsOffset, val); break;
			case SF2Generator::EndloopAddrsOffset: genList.SetAmount(GeneratorType::EndloopAddrsOffset, val); break;
			case SF2Generator::StartAddrsCoarseOffset: genList.SetAmount(GeneratorType::StartAddrsCoarseOffset, val); break;
			case SF2Generator::ModLfoToPitch: genList.SetAmount(GeneratorType::ModLfoToPitch, val); break;
			case SF2Generator::VibLfoToPitch: genList.SetAmount(GeneratorType::VibLfoToPitch, val); break;

			case SF2Generator::ModEnvToPitch:
				genList.SetAmount(GeneratorType::ModEnvToPitch, val);
				break;

			case SF2Generator::InitialFilterFc:

				if (regionType == RegionType::Preset)
					val = UnitTransform::TimecentsToSecsf(val);
				else
					val = UnitTransform::CentsToHertz(val);

				genList.SetAmount(GeneratorType::InitialFilterFc, val);
				break;

			case SF2Generator::InitialFilterQ:
				val = UnitTransform::CentibelsToDecibels(val);
				genList.SetAmount(GeneratorType::InitialFilterQ, val);
				break;


			case SF2Generator::ModLfoToFilterFc: genList.SetAmount(GeneratorType::ModLfoToFilterFc, val);  break;
			case SF2Generator::ModEnvToFilterFc: genList.SetAmount(GeneratorType::ModEnvToFilterFc, val);  break;
			case SF2Generator::EndAddrsCoarseOffset: genList.SetAmount(GeneratorType::EndAddrsCoarseOffset, val);  break;

			case SF2Generator::ModLfoToVolume:
				val = UnitTransform::CentibelsToDecibels(val);
				genList.SetAmount(GeneratorType::ModLfoToVolume, val);
				break;

			case SF2Generator::ChorusEffectsSend:
				genList.SetAmount(GeneratorType::ChorusEffectsSend, val / 10.0f);
				break;

			case SF2Generator::ReverbEffectsSend:
				genList.SetAmount(GeneratorType::ReverbEffectsSend, val / 10.0f);
				break;

			case SF2Generator::Pan: genList.SetAmount(GeneratorType::Pan, val / 10.0f);
				break;

			case SF2Generator::DelayModLFO:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::DelayModLFO, val);
				break;

			case SF2Generator::FreqModLFO:
				if (regionType == RegionType::Preset)
					val = UnitTransform::TimecentsToSecsf(val);
				else
					val = UnitTransform::CentsToHertz(val);
				genList.SetAmount(GeneratorType::FreqModLFO, val);
				break;

			case SF2Generator::DelayVibLFO:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::DelayVibLFO, val);
				break;

			case SF2Generator::FreqVibLFO:
				if (regionType == RegionType::Preset)
					val = UnitTransform::TimecentsToSecsf(val);
				else
					val = UnitTransform::CentsToHertz(val);
				genList.SetAmount(GeneratorType::FreqVibLFO, val);
				break;

			case SF2Generator::DelayModEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::DelayModEnv, val);
				break;

			case SF2Generator::AttackModEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::AttackModEnv, val);
				break;

			case SF2Generator::HoldModEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::HoldModEnv, val);
				break;

			case SF2Generator::DecayModEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::DecayModEnv, val);
				break;

			case SF2Generator::SustainModEnv:
				genList.SetAmount(GeneratorType::SustainModEnv, 1 - val / 1000.0f);
				break;

			case SF2Generator::ReleaseModEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::ReleaseModEnv, val);
				break;

			case SF2Generator::KeynumToModEnvHold:
				genList.SetAmount(GeneratorType::KeynumToModEnvHold, val / 100.0f);
				break;

			case SF2Generator::KeynumToModEnvDecay:
				genList.SetAmount(GeneratorType::KeynumToModEnvDecay, val / 100.0f);
				break;

			case SF2Generator::DelayVolEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::DelayVolEnv, val);
				break;

			case SF2Generator::AttackVolEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::AttackVolEnv, val);
				break;

			case SF2Generator::HoldVolEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::HoldVolEnv, val);
				break;

			case SF2Generator::DecayVolEnv:

				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::DecayVolEnv, val);
				break;

			case SF2Generator::SustainVolEnv:

				val = UnitTransform::DecibelsToGain(-val / 10);
				genList.SetAmount(GeneratorType::SustainVolEnv, val);
				break;

			case SF2Generator::ReleaseVolEnv:
				val = UnitTransform::TimecentsToSecsf(val);
				genList.SetAmount(GeneratorType::ReleaseVolEnv, val);
				break;

			case SF2Generator::KeynumToVolEnvHold:
				genList.SetAmount(GeneratorType::KeynumToVolEnvHold, val / 100.0f);
				break;

			case SF2Generator::KeynumToVolEnvDecay:
				genList.SetAmount(GeneratorType::KeynumToVolEnvDecay, val / 100.0f);
				break;

			case SF2Generator::StartloopAddrsCoarseOffset:  genList.SetAmount(GeneratorType::StartloopAddrsCoarseOffset, val); break;
			case SF2Generator::Keynum: genList.SetAmount(GeneratorType::Keynum, val); break;
			case SF2Generator::Velocity:  genList.SetAmount(GeneratorType::Velocity, val); break;

			case SF2Generator::InitialAttenuation:
				genList.SetAmount(GeneratorType::InitialAttenuation, -val * 0.1f);
				break;

			case SF2Generator::EndloopAddrsCoarseOffset:genList.SetAmount(GeneratorType::EndloopAddrsCoarseOffset, val); break;
			case SF2Generator::CoarseTune: genList.SetAmount(GeneratorType::CoarseTune, val); break;
			case SF2Generator::FineTune:  genList.SetAmount(GeneratorType::FineTune, val); break;
			case SF2Generator::SampleModes:  genList.SetAmount(GeneratorType::SampleModes, val); break;
			case SF2Generator::ScaleTuning:genList.SetAmount(GeneratorType::ScaleTuning, val); break;
			case SF2Generator::ExclusiveClass: genList.SetAmount(GeneratorType::ExclusiveClass, val);  break;
			case SF2Generator::OverridingRootKey: genList.SetAmount(GeneratorType::OverridingRootKey, val);  break;

			}
		}
	}

	//生成调制器
	void SF2Parser::CreateModulator(vector<SF2ModulatorList*>& mods, int idx, Region* region)
	{
		Modulator* modulator = new Modulator();
		uint16_t source;
		SF2ModulatorSource stSource;

		source = mods[idx]->ModulatorSource;
		stSource.index = source & 0x7f;
		stSource.midiControllerFlag = (source >> 7) & 0x1f;
		stSource.direction = (source >> 8) & 0x1f;
		stSource.polarities = (source >> 9) & 0x1f;
		stSource.type = (source >> 10) & 0x3f;

		ModInputType inputType = ModInputType::Preset;
		if (stSource.midiControllerFlag == 1) {
			inputType = ModInputType::MidiController;
		}
		else {
			if (stSource.index == 127)
				inputType = ModInputType::Modulator;
			else
				inputType = ModInputType::Preset;
		}

		ModSourceTransformType modSourceTransType = (ModSourceTransformType)stSource.type;
		modulator->SetSourceTransform(0, modSourceTransType, stSource.direction, stSource.polarities);

		if (inputType != ModInputType::Modulator)
		{
			modulator->AddInputInfo(inputType, (ModInputPreset)stSource.index, (MidiControllerType)stSource.index, 0, 0, 127);
		}
		else
		{
			uint16_t destIdx = (uint16_t)(mods[idx]->ModulatorDestination);
			if (modulators[destIdx] == nullptr)
				CreateModulator(mods, destIdx, region);

			modulators[destIdx]->SetOutTarget(modulator, 0);
		}

		//
		source = mods[idx]->ModulatorAmountSource;
		stSource.index = source & 0x7f;
		stSource.midiControllerFlag = (source >> 7) & 0x1f;
		stSource.direction = (source >> 8) & 0x1f;
		stSource.polarities = (source >> 9) & 0x1f;
		stSource.type = (source >> 10) & 0x3f;

		inputType = ModInputType::Preset;
		if (stSource.midiControllerFlag == 1) {
			inputType = ModInputType::MidiController;
		}
		else {
			if (stSource.index == 127)
				inputType = ModInputType::Modulator;
			else
				inputType = ModInputType::Preset;
		}

		if (!((inputType == ModInputType::Preset && stSource.index == 0) || inputType == ModInputType::Modulator))
		{
			modSourceTransType = (ModSourceTransformType)stSource.type;
			modulator->SetSourceTransform(1, modSourceTransType, stSource.direction, stSource.polarities);
			modulator->AddInputInfo(inputType, (ModInputPreset)stSource.index, (MidiControllerType)stSource.index, 1, 0, 127);
		}

		modulator->SetAmount(mods[idx]->ModulatorAmount);
		modulator->SetAbsType((ModTransformType)mods[idx]->ModulatorTransform);
		modulator->SetOutTarget((GeneratorType)mods[idx]->ModulatorDestination);

		ReplaceRegionPrevSameModulator(modulator, region);
		modulators[idx] = modulator;
	}

	//替换区域中前一个相似调制器
	void SF2Parser::ReplaceRegionPrevSameModulator(Modulator* modulator, Region* region)
	{
		if (region->GetModulators() == nullptr)
		{
			region->AddModulator(modulator);
			return;
		}

		bool isSame = false;
		Modulator* tmp;
		ModulatorVec& modulatorVec = *(region->GetModulators());

		for (int i = 0; i < modulatorVec.size(); i++)
		{
			isSame = modulatorVec[i]->IsSame(modulator);
			if (isSame)
			{
				tmp = modulatorVec[i];
				modulatorVec[i] = modulator;
				delete tmp;
				return;
			}
		}

		region->AddModulator(modulator);
	}
}
