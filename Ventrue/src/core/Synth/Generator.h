#ifndef _Generator_h_
#define _Generator_h_

#include"VentrueTypes.h"

namespace ventrue
{   
    union GeneratorAmount
    {
        struct{
            float low;
            float high;
        } rangeData;

        float amount;
    };

    class Generator
    {
    public:  
        Generator(GeneratorType type = GeneratorType::None)
        {
            this->type = type;
            genAmount.rangeData.low = 0;
            genAmount.rangeData.high = 0;
            genAmount.amount = 0;
        }

    public:
        GeneratorType type;
        GeneratorAmount genAmount;
    };

   
    // 生成器列表
    class GeneratorList
    {
    public:
        ~GeneratorList();

        void Clear();

        void Remove(GeneratorType type);
        // type值
        // 0: instrument类型
        // 1: preset类型
        inline void SetType(RegionType type)
        {
            this->type = type;
        }

        // 根据生成器类型,判断此类型生成器是否为空值，从未设置过
        bool IsEmpty(GeneratorType type)
        {
            return (gens[(int)type] == nullptr ? true : false);
        }

        // 根据生成器类型，获取生成器数据值
        float GetAmount(GeneratorType type)
        {
            return (gens[(int)type] == nullptr ?
                    GetDefaultValue(type) : gens[(int)type]->genAmount.amount);
        }

        // 根据生成器类型，获取生成器数据范围值
        RangeFloat GetAmountRange(GeneratorType type);


        // 根据生成器类型，获取生成器数据范围的低值
        float GetAmountLow(GeneratorType type);


        // 根据生成器类型，获取生成器数据范围的高值(int)
        float GetAmountHigh(GeneratorType type);

        void ZeroAmount(GeneratorType type);

        // 根据生成器类型，设置生成器数据值
        void SetAmount(GeneratorType type, float amount);
          
        // 根据生成器类型，设置生成器数据范围值
        void SetAmountRange(GeneratorType type, float low, float high);

        float GetDefaultValue(GeneratorType genType);

        RangeFloat GetDefaultRangeValue(GeneratorType genType);

        // 限制类型值的取值范围   
        float LimitValueRange(GeneratorType genType, float value);

          
        // 限制类型值的取值范围  
        RangeFloat LimitRangeValueRange(GeneratorType genType, float low, float high);
 
    private:
        Generator* gens[64] = {nullptr}; 
        RegionType type = RegionType::Insttrument;

    };
}

#endif