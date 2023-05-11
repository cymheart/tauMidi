#ifndef _Generator_h_
#define _Generator_h_

#include"TauTypes.h"

namespace tau
{
	struct Generator
	{
		GeneratorType type = GeneratorType::None;
		GeneratorAmount genAmount = {(int16_t)0};
	};


	// 生成器列表
	class GeneratorList
	{
	public:
		~GeneratorList();

		void Clear();
		void Remove(GeneratorType type);
		void Copy(GeneratorList& orgGens);

		// 根据生成器类型,判断此类型生成器是否为空值，从未设置过
		bool IsEmpty(GeneratorType type)
		{
			return gens[(int)type].type == GeneratorType::None;
		}

		// 根据生成器类型，获取生成器数据值
		GeneratorAmount GetAmount(GeneratorType type);

		// 根据生成器类型，设置生成器数据值
		void SetAmount(GeneratorType type, GeneratorAmount amount);

		GeneratorAmount GetDefaultValue(GeneratorType genType);

		// 钳位类型值的取值范围   
		GeneratorAmount ClampValueRange(GeneratorType genType, GeneratorAmount value);


	private:
		Generator gens[(int)GeneratorType::EndOper + 1];
	};
}

#endif
