#include"Region.h"
#include"Generator.h"

namespace tau
{
	Region::Region(RegionType type)
	{
		this->type = type;
		genList = new GeneratorList();
		genList->SetType(type);

	}

	Region::~Region()
	{
		DEL(genList);
	}

	//增加调制器
	void Region::AddModulator(Modulator* mod)
	{
		modulatorList.AddModulator(mod);


		auto type = mod->GetInputType(0);
		auto presetType = mod->GetInputPresetType(0);
		if (type == ModInputType::Preset &&
			(presetType == ModInputPreset::NoteOnKey ||
				presetType == ModInputPreset::NoteOnVelocity))
		{
			isHavNoteOnKeyModulator = true;
		}

	}

	// 获取生成器KeyRange数据值
	RangeFloat Region::GetKeyRange()
	{
		return genList->GetAmountRange(GeneratorType::KeyRange);
	}

	// 获取生成器VelRange数据值
	RangeFloat Region::GetVelRange()
	{
		return genList->GetAmountRange(GeneratorType::VelRange);
	}
}
