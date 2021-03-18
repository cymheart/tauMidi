
#ifndef _ModulatorList_h_
#define _ModulatorList_h_

#include"VentrueTypes.h"
#include"Midi/MidiTypes.h"
#include"Modulator.h"

namespace ventrue
{
	/*
	* 调制器列表
	* 包含了内部内置控制器调制器，和外部添加调制器
	* by cymheart, 2020--2021.
	*/
	class ModulatorList
	{
	public:
		ModulatorList();
		~ModulatorList();

		//获取调制器列表
		ModulatorVec* GetModulators()
		{
			return modulators;
		}

		// 新增一个调制器 
	   // <returns>返回新增的调制器</returns>
		void AddModulator(Modulator* mod);

		//根据指定类型启用内部控制器调制器
		void OpenInsideCtrlModulator(MidiControllerType ctrlType);

		//根据指定类型启用内部预设调制器
		void OpenInsidePresetModulator(ModInputPreset modPresetType);

		//根据指定类型关闭内部控制器调制器
		void CloseInsideCtrlModulator(MidiControllerType ctrlType);

		//根据指定类型关闭内部预设调制器
		void CloseInsidePresetModulator(ModInputPreset modPresetType);

		//生成内部颤音调制器
		void CreateInsideVibModulator();

		//关闭所有内部调制器
		void CloseAllInsideModulator();


	private:
		void CreateInsideModulators();

		//生成内部pan调制器
		void CreateInsidePanModulator();

		//生成内部CoarseTune调制器(以半音为单位校正音调)
		void CreateInsideCoarseTuneModulator();

		//生成内部FineTune调制器(以音分为单位校正音调)
		void CreateInsideFineTuneModulator();

		//生成内部Volume调制器
		void CreateInsideVolumeModulator();

		//生成内部SustainPedalOnOff调制器
		void CreateInsideSustainPedalOnOffModulator();

		//生成内部滑音调制器
		void CreateInsidePitchBendModulator();

		static float VolGainTans(float gain);

	private:

		//调制器列表
		ModulatorVec* modulators = nullptr;
		Modulator* insideCtrlMod[128] = { nullptr };
		bool isUsedInsideCtrlMod[128] = { 0 };

		Modulator* insidePresetMod[20] = { nullptr };
		bool isUsedInsidePresetMod[20] = { 0 };
	};
}

#endif
