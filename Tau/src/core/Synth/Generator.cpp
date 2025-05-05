#include"Generator.h"
namespace tau
{
	GeneratorList::~GeneratorList()
	{
		Clear();
	}

	void GeneratorList::Clear()
	{
		for (int i = 0; i < 64; i++)
		{
			gens[i].type = GeneratorType::None;
			gens[i].genAmount.amount = 0;
		}
	}

	void GeneratorList::Remove(GeneratorType type)
	{
		gens[(int)type].type = GeneratorType::None;
		gens[(int)type].genAmount.amount = 0;
	}

	void GeneratorList::Copy(GeneratorList& orgGens) {
		int size = (int)GeneratorType::EndOper + 1;
		memcpy(gens, orgGens.gens, sizeof(Generator) * size);
	}

	GeneratorAmount GeneratorList::GetAmount(GeneratorType type)
	{
		return (gens[(int)type].type == GeneratorType::None ?
			GetDefaultValue(type) : ClampValueRange(type, gens[(int)type].genAmount));
	}


	// 根据生成器类型，设置生成器数据值
	void GeneratorList::SetAmount(GeneratorType type, GeneratorAmount amount)
	{
		gens[(int)type].type = type;
		gens[(int)type].genAmount = ClampValueRange(type, amount);
	}

	//获取默认值
	GeneratorAmount GeneratorList::GetDefaultValue(GeneratorType genType)
	{
		GeneratorAmount genAmount;

		switch (genType)
		{
		case GeneratorType::StartAddrsOffset: genAmount.amount = 0; break;
		case GeneratorType::EndAddrsOffset: genAmount.amount = 0; break;
		case GeneratorType::StartloopAddrsOffset: genAmount.amount = 0; break;
		case GeneratorType::EndloopAddrsOffset: genAmount.amount = 0; break;
		case GeneratorType::StartAddrsCoarseOffset: genAmount.amount = 0; break;
		case GeneratorType::EndAddrsCoarseOffset: genAmount.amount = 0;  break;
		case GeneratorType::ModLfoToPitch: genAmount.amount = 0; break;
		case GeneratorType::VibLfoToPitch: genAmount.amount = 0; break;
		case GeneratorType::ModEnvToPitch: genAmount.amount = 0; break;
		//
		case GeneratorType::InitialFilterFc: genAmount.amount = 13500; break;
		case GeneratorType::InitialFilterQ: genAmount.amount = 0;  break;
		case GeneratorType::ModLfoToFilterFc: genAmount.amount = 0;  break;
		case GeneratorType::ModEnvToFilterFc: genAmount.amount = 0;  break;
		case GeneratorType::ModLfoToVolume: genAmount.amount = 0;  break;
		case GeneratorType::ChorusEffectsSend: genAmount.amount = 0;  break;
		case GeneratorType::ReverbEffectsSend: genAmount.amount = 0;  break;
		case GeneratorType::Pan: genAmount.amount = 0;  break;
		case GeneratorType::DelayModLFO: genAmount.amount = -12000;  break;
		case GeneratorType::FreqModLFO: genAmount.amount = 0;  break;
		case GeneratorType::DelayVibLFO: genAmount.amount = -12000;  break;
		case GeneratorType::FreqVibLFO: genAmount.amount = 0;  break;

		//
		case GeneratorType::DelayModEnv: genAmount.amount = -12000; break;
		case GeneratorType::AttackModEnv: genAmount.amount = -12000; break;
		case GeneratorType::DecayModEnv:  genAmount.amount = -12000; break;
		case GeneratorType::HoldModEnv: genAmount.amount = -12000; break;
		case GeneratorType::SustainModEnv: genAmount.amount = 0; break;
		case GeneratorType::ReleaseModEnv: genAmount.amount = -12000; break;
		case GeneratorType::KeynumToModEnvHold: genAmount.amount = 0; break;
		case GeneratorType::KeynumToModEnvDecay: genAmount.amount = 0; break;

		//	
		case GeneratorType::DelayVolEnv: genAmount.amount = -12000; break;
		case GeneratorType::AttackVolEnv: genAmount.amount = -12000; break;
		case GeneratorType::DecayVolEnv: genAmount.amount = -12000; break;
		case GeneratorType::HoldVolEnv: genAmount.amount = -12000; break;
		case GeneratorType::SustainVolEnv: genAmount.amount = 0; break;
		case GeneratorType::ReleaseVolEnv: genAmount.amount = -12000; break;
		case GeneratorType::KeynumToVolEnvHold: genAmount.amount = 0; break;
		case GeneratorType::KeynumToVolEnvDecay: genAmount.amount = 0; break;

		//
		case GeneratorType::KeyRange: 
		case GeneratorType::VelRange:
			genAmount.rangeData.low = 0; 
			genAmount.rangeData.high = 127;
			break;

		case GeneratorType::Keynum:  genAmount.amount = -1; break;
		case GeneratorType::Velocity:  genAmount.amount = -1; break;
		case GeneratorType::InitialAttenuation: genAmount.amount = 0; break;
		//
		case GeneratorType::ScaleTuning: genAmount.amount = 100; break;
		case GeneratorType::CoarseTune: genAmount.amount = 0; break;
		case GeneratorType::FineTune: genAmount.amount = 0; break;
		case GeneratorType::OverridingRootKey: genAmount.amount = -1; break;
		case GeneratorType::ExclusiveClass: genAmount.amount = 0; break;

		//
		case GeneratorType::SustainPedalOnOff:
		case GeneratorType::Pressure:
			genAmount.amount = -1;
			break;

		default:
			genAmount.amount = 0; break;
		}

		return genAmount;
	}


	// 钳位类型值的取值范围
	GeneratorAmount GeneratorList::ClampValueRange(GeneratorType genType, GeneratorAmount value)
	{
		if (zoneType == ZoneType::Preset)
			return value;

		switch (genType)
		{
		case GeneratorType::ModLfoToPitch:
		case GeneratorType::VibLfoToPitch:
		case GeneratorType::ModEnvToPitch:
		case GeneratorType::ModLfoToFilterFc:
		case GeneratorType::ModEnvToFilterFc:
			value.amount = max(value.amount, -12000);
			value.amount = min(value.amount, 12000);
			break;

		case GeneratorType::InitialFilterFc: 
			value.amount = max(value.amount, 1500);
			value.amount = min(value.amount, 13500);
			break;

		case GeneratorType::InitialFilterQ: 
			value.amount = max(value.amount, 0);
			value.amount = min(value.amount, 960);
			break;


		case GeneratorType::ModLfoToVolume:
			value.amount = max(value.amount, -960);
			value.amount = min(value.amount, 960);
			break;

		case GeneratorType::ChorusEffectsSend:	
		case GeneratorType::ReverbEffectsSend:
		case GeneratorType::SustainModEnv:
			value.amount = max(value.amount, 0);
			value.amount = min(value.amount, 1000);
			break;

		case GeneratorType::Pan:	
			value.amount = max(value.amount, -500);
			value.amount = min(value.amount, 500);
			break;

		case GeneratorType::DelayModLFO: 
		case GeneratorType::DelayVibLFO:
		case GeneratorType::DelayModEnv:
		case GeneratorType::HoldModEnv:
		case GeneratorType::DelayVolEnv:
		case GeneratorType::HoldVolEnv:
			value.amount = max(value.amount, -12000);
			value.amount = min(value.amount, 5000);
			break;

		case GeneratorType::FreqModLFO:
		case GeneratorType::FreqVibLFO:
			value.amount = max(value.amount, -16000);
			value.amount = min(value.amount, 4500);
			break;

			//
		case GeneratorType::AttackModEnv: 
		case GeneratorType::DecayModEnv:
		case GeneratorType::ReleaseModEnv:
		case GeneratorType::AttackVolEnv:
		case GeneratorType::DecayVolEnv:
		case GeneratorType::ReleaseVolEnv:
			value.amount = max(value.amount, -12000);
			value.amount = min(value.amount, 8000);
			break;


		case GeneratorType::KeynumToModEnvHold:
		case GeneratorType::KeynumToModEnvDecay:
		case GeneratorType::KeynumToVolEnvHold:
		case GeneratorType::KeynumToVolEnvDecay:
			value.amount = max(value.amount, -1200);
			value.amount = min(value.amount, 1200);
			break;


			//	
		case GeneratorType::SustainVolEnv:
		case GeneratorType::InitialAttenuation:
			value.amount = max(value.amount, 0);
			value.amount = min(value.amount, 1440);
			break;

			//
		case GeneratorType::KeyRange: 	
		case GeneratorType::VelRange:
			value.rangeData.low = max(value.rangeData.low, 0);
			value.rangeData.low = min(value.rangeData.low, 127);
			value.rangeData.high = max(value.rangeData.high, 0);
			value.rangeData.high = min(value.rangeData.high, 127);
			if (value.rangeData.low > value.rangeData.high)
				value.rangeData.low = value.rangeData.high;
			break;


		case GeneratorType::Keynum:
		case GeneratorType::Velocity:
		case GeneratorType::OverridingRootKey:
			value.amount = max(value.amount, 0);
			value.amount = min(value.amount, 127);
			break;


			//
		case GeneratorType::ScaleTuning: 
			value.amount = max(value.amount, 0);
			value.amount = min(value.amount, 1200);
			break;

		case GeneratorType::CoarseTune: 	
			value.amount = max(value.amount, -120);
			value.amount = min(value.amount, 200);
			break;

		case GeneratorType::FineTune:
			value.amount = max(value.amount, -99);
			value.amount = min(value.amount, 99);
			break;

		}

		return value;
	}

}
