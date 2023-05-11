#include"Zone.h"


namespace tau
{
	Zone::Zone(ZoneType type)
	{
		this->type = type;
	}

	Zone::~Zone()
	{
	}

	//增加调制器
	void Zone::AddModulator(Modulator* mod)
	{
		modulators.push_back(mod);
	}

	// 获取生成器KeyRange数据值
	GeneratorAmount Zone::GetKeyRange()
	{
		return genList.GetAmount(GeneratorType::KeyRange);
	}

	// 获取生成器VelRange数据值
	GeneratorAmount Zone::GetVelRange()
	{
		return genList.GetAmount(GeneratorType::VelRange);
	}
}
