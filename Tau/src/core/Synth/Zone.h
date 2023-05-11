#ifndef _Zone_h_
#define _Zone_h_

#include"TauTypes.h"
#include"Generator.h"

namespace tau
{
	/*
	* 区域是一个生成器列表genList 和 一个调制器列表modulatorlist的集合
	* 其中调制器是对生成器调制的动态变换方法，由外部输入值控制调制生成器的新值
	* by cymheart, 2020--2021.
	*/
	class Zone
	{
	public:
		Zone(ZoneType type);
		~Zone();

		// 获取生成器数据表
		inline GeneratorList& GetGens()
		{
			return genList;
		}

		// 获取调制器表
		vector<Modulator*>& GetModulators()
		{
			return modulators;
		}

		//增加调制器
		void AddModulator(Modulator* mod);

		// 获取生成器KeyRange数据值
		GeneratorAmount GetKeyRange();

		// 获取生成器VelRange数据值
		GeneratorAmount GetVelRange();

	private:
		ZoneType type;
		//生成器数据表
		GeneratorList genList;
		//调制器列表
		vector<Modulator*> modulators;
	};
}

#endif
