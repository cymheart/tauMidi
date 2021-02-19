#include"Region.h"
#include"Generator.h"

namespace ventrue
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