#ifndef _InsideModulators_h_
#define _InsideModulators_h_

#include"TauTypes.h"
#include"Midi/MidiTypes.h"
#include"Modulator.h"

namespace tau
{
	/*
	* 内部调制器列表
	* by cymheart, 2020--2025.
	*/
	class InsideModulators
	{
	public:
		InsideModulators();
		~InsideModulators();

		//获取调制器列表
		vector<Modulator*>& GetModulators()
		{
			return modulators;
		}

		//根据指定类型启用内部控制器调制器
		void OpenInsideCtrlModulator(MidiControllerType ctrlType);

		//根据指定类型启用内部预设调制器
		void OpenInsidePresetModulator(ModInputPreset modPresetType);

		//根据指定类型关闭内部控制器调制器
		void CloseInsideCtrlModulator(MidiControllerType ctrlType);

		//根据指定类型关闭内部预设调制器
		void CloseInsidePresetModulator(ModInputPreset modPresetType);

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

		//生成内部ModulationWheel调制器
		void CreateInsideModulationWheelModulator();

		//生成内部滑音调制器
		void CreateInsidePitchBendModulator();

		//生成内部PloyPressure调制器
		void CreateInsidePloyPressureModulator();

	private:

		//调制器列表
		vector<Modulator*> modulators;
		Modulator* insideCtrlMod[128] = { nullptr };
		bool isUsedInsideCtrlMod[128] = { 0 };

		Modulator* insidePresetMod[20] = { nullptr };
		bool isUsedInsidePresetMod[20] = { 0 };
	};
}

#endif
