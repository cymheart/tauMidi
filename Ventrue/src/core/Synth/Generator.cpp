#include"Generator.h"
namespace ventrue
{
	GeneratorList::~GeneratorList()
	{
		Clear();
	}

	void GeneratorList::Clear()
	{
		for (int i = 0; i < 64; i++)
		{
			DEL(gens[i]);
		}
	}

	void GeneratorList::Remove(GeneratorType type)
	{
		DEL(gens[(int)type]);
	}

	// 根据生成器类型，获取生成器数据范围值
	RangeFloat GeneratorList::GetAmountRange(GeneratorType type)
	{
		if (gens[(int)type] == nullptr)
			return GetDefaultRangeValue(type);

		return RangeFloat(
			gens[(int)type]->genAmount.rangeData.low,
			gens[(int)type]->genAmount.rangeData.high);
	}

	// 根据生成器类型，获取生成器数据范围的低值
	float GeneratorList::GetAmountLow(GeneratorType type)
	{
		if (gens[(int)type] == nullptr)
		{
			RangeFloat rangeValue = GetDefaultRangeValue(type);
			return rangeValue.min;
		}

		return gens[(int)type]->genAmount.rangeData.low;
	}

	// 根据生成器类型，获取生成器数据范围的高值(int)>
	float GeneratorList::GetAmountHigh(GeneratorType type)
	{
		if (gens[(int)type] == nullptr)
		{
			RangeFloat rangeValue = GetDefaultRangeValue(type);
			return rangeValue.max;
		}

		return gens[(int)type]->genAmount.rangeData.high;
	}

	void GeneratorList::ZeroAmount(GeneratorType type)
	{
		if (gens[(int)type] == nullptr)
			gens[(int)type] = new Generator(type);

		gens[(int)type]->genAmount.amount = 0;
	}

	// 根据生成器类型，设置生成器数据值
	void GeneratorList::SetAmount(GeneratorType type, float amount)
	{
		if (gens[(int)type] == nullptr)
			gens[(int)type] = new Generator(type);

		gens[(int)type]->genAmount.amount = LimitValueRange(type, amount);
	}

	// 根据生成器类型，设置生成器数据范围值
	void GeneratorList::SetAmountRange(GeneratorType type, float low, float high)
	{
		if (gens[(int)type] == nullptr)
			gens[(int)type] = new Generator(type);

		RangeFloat rangeValue = LimitRangeValueRange(type, low, high);
		gens[(int)type]->genAmount.rangeData.low = rangeValue.min;
		gens[(int)type]->genAmount.rangeData.high = rangeValue.max;
	}


	//获取默认值
	float GeneratorList::GetDefaultValue(GeneratorType genType)
	{
		switch (genType)
		{
		case GeneratorType::InitialFilterFc: return 19912;
		case GeneratorType::InitialFilterQ: return -3;
		case GeneratorType::ScaleTuning: return 100;
		case GeneratorType::InitialAttenuation: return 0;

		case GeneratorType::AttackVolEnv:
			return 0.001f;
			break;

		case GeneratorType::ReleaseVolEnv:
			return 0.001f;
			break;

		case GeneratorType::DecayVolEnv:
		case GeneratorType::HoldVolEnv:
		case GeneratorType::AttackModEnv:
		case GeneratorType::DecayModEnv:
		case GeneratorType::HoldModEnv:
		case GeneratorType::ReleaseModEnv:
			return 0.001f;

		case GeneratorType::SustainVolEnv:
		case GeneratorType::SustainModEnv:
			return 1;


		case GeneratorType::Keynum:
			return -1;

		case GeneratorType::OverridingRootKey:
			return -1;

		case GeneratorType::FreqModLFO:
		case GeneratorType::FreqVibLFO:
			return 20;

		default:
			return 0;
		}
	}

	RangeFloat GeneratorList::GetDefaultRangeValue(GeneratorType genType)
	{
		RangeFloat rangeValue;

		switch (genType)
		{
		case GeneratorType::VelRange:
		case GeneratorType::KeyRange:
			rangeValue.min = 0;
			rangeValue.max = 127;
			return rangeValue;

		default:
			rangeValue.min = 0;
			rangeValue.max = 127;
			return rangeValue;
		}
	}

	// 限制类型值的取值范围
	float GeneratorList::LimitValueRange(GeneratorType genType, float value)
	{
		switch (genType)
		{
		case GeneratorType::InitialFilterFc:
			if (type == RegionType::Insttrument)
			{
				if (value > 19912) value = 19912;
				else if (value < 19) value = 19;
			}
			else
			{
				if (value < 0) value = 0;
			}
			return  value;

		case GeneratorType::InitialAttenuation:
			if (type == RegionType::Insttrument)
			{
				if (value > 0) value = 0;
				else if (value < -144) value = -144;
			}
			else
			{
				if (value < -60) value = -60;
				else if (value > 60) value = 60;
			}
			return value;

		case GeneratorType::Pan:
			if (type == RegionType::Insttrument)
			{
				if (value > 50) value = 50;
				else if (value < -50) value = -50;
			}
			else
			{
				if (value > 100) value = 100;
				else if (value < -100) value = -100;
			}
			return value;

		case GeneratorType::SampleModes:
			if (value < 0) value = 0;
			else if (value > 2) value = 2;
			return value;

		case GeneratorType::CoarseTune:
			if (value < -120) value = -120;
			else if (value > 120) value = 120;
			return value;

		case GeneratorType::FineTune:
			if (value < -99) value = -99;
			else if (value > 99) value = 99;
			return value;

		case GeneratorType::ScaleTuning:
			if (value < 0) value = 0;
			else if (value > 1200) value = 1200;
			return value;

		case GeneratorType::VibLfoToPitch:
		case GeneratorType::ModLfoToPitch:
		case GeneratorType::ModLfoToFilterFc:
			if (value > 12000) value = 12000;
			else if (value < -12000) value = 12000;
			return value;

		case GeneratorType::AttackVolEnv:
			return value < 0.001f ? 0.001f : value;
			break;

		case GeneratorType::ReleaseVolEnv:
			return value < 0.001f ? 0.001f : value;
			break;

		case GeneratorType::DecayVolEnv:
		case GeneratorType::HoldVolEnv:
		case GeneratorType::AttackModEnv:
		case GeneratorType::DecayModEnv:
		case GeneratorType::HoldModEnv:
		case GeneratorType::ReleaseModEnv:
			return value < 0.001f ? 0.001f : value;

		case GeneratorType::SustainModEnv:
		case GeneratorType::SustainVolEnv:
			if (value < 0) value = 0;
			else if (value > 1) value = 1;
			return value;

		case GeneratorType::KeynumToVolEnvHold:
		case GeneratorType::KeynumToVolEnvDecay:
		case GeneratorType::KeynumToModEnvHold:
		case GeneratorType::KeynumToModEnvDecay:
			if (value < -12) value = -12;
			else if (value > 12) value = 12;
			return value;

		case GeneratorType::Keynum:
			if (value < 0) value = 0;
			else if (value > 127) value = 127;
			return value;


		default:
			return value;
		}
	}

	// 限制类型值的取值范围
	RangeFloat GeneratorList::LimitRangeValueRange(GeneratorType genType, float low, float high)
	{
		switch (genType)
		{
		case GeneratorType::KeyRange:
		case GeneratorType::VelRange:
			if (low < 0) low = 0;
			else if (low > 127) low = 127;

			if (high < 0) high = 0;
			else if (high > 127) high = 127;

			if (low > high)
				low = high;
			break;
		}

		RangeFloat rangeValue(low, high);
		return rangeValue;
	}
}
