#ifndef _Region_h_
#define _Region_h_

#include"TauTypes.h"
#include"ModulatorList.h"

namespace tau
{
	/*
	* 区域是一个生成器列表genList 和 一个调制器列表modulatorlist的集合
	* 其中调制器是对生成器调制的动态变换方法，由外部输入值控制调制生成器的新值
	* by cymheart, 2020--2021.
	*/
	class Region
	{
	public:
		Region(RegionType type);
		~Region();


		// 获取生成器数据表
		inline GeneratorList* GetGenList()
		{
			return genList;
		}

		// 获取调制器表
		ModulatorVec* GetModulators()
		{
			return modulatorList.GetModulators();
		}

		//增加调制器
		void AddModulator(Modulator* mod);

		// 获取生成器KeyRange数据值
		RangeFloat GetKeyRange();

		// 获取生成器VelRange数据值
		RangeFloat GetVelRange();

		bool IsHavNoteOnKeyModulator()
		{
			return isHavNoteOnKeyModulator;
		}

	private:

		RegionType type;

		//外部固定设置的生成器数据表
		GeneratorList* genList = nullptr;

		//外部固定设置的调制器列表
		ModulatorList modulatorList;

		bool isHavNoteOnKeyModulator = false;

	};
}

#endif
