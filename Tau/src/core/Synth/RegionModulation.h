#ifndef _ModulationAndComplex_h_
#define _ModulationAndComplex_h_

#include"TauTypes.h"
#include"Region.h"
#include"Midi/MidiTypes.h"
#include"Generator.h"

namespace tau
{
	/*
	* 区域调制处理器
	* by cymheart, 2020--2021.
	*/
	class RegionModulation
	{
	public:
		RegionModulation();
		~RegionModulation();

		void Clear();

		//调制
		void Modulation();

		//设置Gen修改项保存变量
		inline void SetModifyedGenTypes(unordered_set<int>* modifyedGenTypes)
		{
			this->modifyedGenTypes = modifyedGenTypes;
		}


		//设置内部控制器调制器列表
		inline void SetInsideCtrlModulatorList(ModulatorList* insideCtrlModulatorList)
		{
			this->insideCtrlModulatorList = insideCtrlModulatorList;
		}


		//设置通道
		inline  void SetChannel(Channel* channel)
		{
			this->channel = channel;
		}

		//设置是否使用外部普通调制器
		inline void SetUseCommonModulator(bool use)
		{
			useCommonModulator = use;
		}

		/// <summary>
		/// 设置Gen值最终修改后的存储
		/// </summary>
		/// <param name="modifyedGenList"></param>
		inline void SetDestModifyedGenList(GeneratorList* modifyedGenList)
		{
			this->modifyedGenList = modifyedGenList;
		}

		//设置区域
		inline void SetRegions(
			Region* instRegion, Region* instGlobalRegion,
			Region* presetRegion, Region* presetGlobalRegion)
		{
			this->instRegion = instRegion;
			this->instGlobalRegion = instGlobalRegion;
			this->presetRegion = presetRegion;
			this->presetGlobalRegion = presetGlobalRegion;
		}



	private:

		// 首次生成乐器调制后的GeneratorList
		void InitGenList();

		//调制发声区域的所有生成器值,生成最终的修改值
		void ModGenList();

		//复合同一乐器中的乐器生成列表(instRegion)和乐器全局生成器列表(instGlobalRegion)值
		//复合方法: 优先使用instRegion中的值，如果没有将使用instGlobalRegion的值，
		//如果此项在两个列表中都查不到，那将删除instCombGenList中对应项
		//VelRange, KeyRange,这两项不参与复合处理，直接写入最终的modifyedGenList对应项中
		void CombInstGenList();

		//复合同一乐器中的预设生成列表(presetRegion)和预设全局生成器列表(presetGlobalRegion)值
		//复合方法: 优先使用presetRegion中的值，如果没有将使用presetGlobalRegion的值，
		//如果此项在两个列表中都查不到，那将删除instCombGenList中对应项
		//VelRange, KeyRange,这两项不参与复合处理
		void CombPresetGenList();

		//复合算得的乐器生成器列表(instCombGenList)和算的预设生成器列表(presetCombGenList)
		void CombInstAndPresetGenList(bool isTestModFlag = true);

		//根据最终复合计算出的生成器列表,设置需要修改的参数类型
		void InitNeedModifyGenParamTypes();

		/// <summary>
		/// 使用调制器调制区域的生成器列表项
		/// </summary>
		/// <param name="mods">用于调制输入生成器的调制器</param>
		/// <param name="orgGenlist">输入的生成器列表</param>
		/// <param name="modGenlist">调制后的对应输出的生成器Genlist</param>
		/// <param name="isAlreadyModedGenTypes">标志对应生成器是否被调制过的flag值</param>
		void SetGenListMods(ModulatorVec* mods, GeneratorList* inGenlist, GeneratorList* outModGenlist, bool* isAlreadyModedGenTypes = nullptr);
		void ExecuteGenListMods(ModulatorVec* mods, GeneratorList* modGenlist, bool* isAlreadyModedGenTypes);

		/// <summary>
		/// 设置调制器输入
		/// </summary>
		/// <param name="mods">用于调制输入生成器的调制器</param>
		void SetModsInputs(ModulatorVec* mods);

		//执行最终修改区域的Gen项mods调制
		void InsideModifyedGenListMods();

		//设置调制器的输入
		//当targetGenType == GeneratorType::None时，此时的调制器调制目标是另一调制器
		//此时也是可以通过测试，使用下面的处理来处理数据的链式结果
		void SetModulatorInput(Modulator& mod);


		void CombGenValue(GeneratorType genType, bool isTestModFlag);

		bool Process(GeneratorType targetGenType, float modValue, ModulationType modType);

		inline void SetProcessGenList(GeneratorList* modGenList)
		{
			processGenList = modGenList;
		}

		inline bool ModulationGenType(GeneratorType genType, float modValue, ModulationType modType)
		{
			float val = processGenList->GetAmount(genType);
			if (modType == ModulationType::Replace) { val = modValue; }
			else if (modType == ModulationType::Add) { val += modValue; }
			else { val *= modValue; }

			processGenList->SetAmount(genType, val);


			return true;
		}

		// 调制按键压力
		bool ModulationPressure(float modValue, ModulationType modType);

		// 调制器调制校音(半音)
		bool ModulationCoarseTune(float modValue, ModulationType modType);

		// 调制器调制校音(音分)
		bool ModulationFineTune(float modValue, ModulationType modType);

		// 调制器调制音阶
		bool ModulationScaleTuning(float modValue, ModulationType modType);

		// 调制器调制根音符
		bool ModulationOverridingRootKey(float modValue, ModulationType modType);

		// 调制器调制衰减
		// 单位:dB
		bool ModulationAttenuation(float volGain, ModulationType modType);

		// 调制器调制滤波截至频率
		// 单位：cents
		bool ModulationFc(float cents, ModulationType modType);

		// 调制器调制滤波共振峰
		// 单位：dB
		// Q = 10^(resonanceDB/20)
		bool ModulationResonance(float resonanceDB, ModulationType modType);

		// 调制器调制音量平衡
		bool ModulationPan(float modValue, ModulationType modType);

		// 调制器调制和声深度
		bool ModulationChorusDepth(float modValue, ModulationType modType);

		// 调制器调制循环播放模式
		bool ModulationLoopPlayback(float modValue, ModulationType modType);

		// 调制器调制独占发音归类
		bool ModulationExclusiveClass(float value, ModulationType modType);

		// 调制器调制vibLfo的延迟时长
		bool ModulationVibLfoDelay(float cents, ModulationType modType);

		// 调制器调制vibLfo的频率
		// cent转HzMul : 8.176 * 2^(cent/1200)
		bool ModulationVibLfoFreq(float cents, ModulationType modType);

		// 调制器调制vibLfo的pitch
		bool ModulationVibLfoModPitch(float cents, ModulationType modType);

		// 调制器调制ModLfo的延迟时长
		bool ModulationModLfoDelay(float cents, ModulationType modType);


		// 调制器调制ModLfo的频率
		// cent转HzMul : 8.176 * 2^(cent/1200)
		bool ModulationModLfoFreq(float cents, ModulationType modType);

		// 调制器调制ModLfo的pitch
		bool ModulationModLfoModPitch(float cents, ModulationType modType);

		// 调制器调制ModLfo的fc
		bool ModulationModLfoModFc(float cents, ModulationType modType);

		// 调制器调制ModLfo的volume
		// dB增益转线性增益倍率: volLinearGainMul = Math.Pow(10, volDbGain / 10f);  
		bool ModulationModLfoModVolume(float volDbGain, ModulationType modType);

		// 调制器调制音量包络的延迟时长 
		// <param name="cents">以音分为单位</param>
		bool ModulationVolumeEnvDelaySec(float cents, ModulationType modType);

		// 调制器调制音量包络的起音时长
		// <param name="cents">以音分为单位</param>
		bool ModulationVolumeEnvAttackSec(float cents, ModulationType modType);

		// 调制器调制音量包络的保持时长
		// <param name="cents">以音分为单位</param>
		bool ModulationVolumeEnvHoldSec(float cents, ModulationType modType);

		// 调制器调制音量包络的衰减时长
		// <param name="cents">以音分为单位</param>
		bool ModulationVolumeEnvDecaySec(float cents, ModulationType modType);

		// 调制器调制音量包络的释音时长
		// <param name="cents">以音分为单位</param>
		bool ModulationVolumeEnvReleaseSec(float cents, ModulationType modType);

		// 调制器调制音量包络的延音分贝
		// sustainMul值域:[0, 1]
		// 可以从db[0, 144]线性转换为[0,1]，db/144 
		bool ModulationVolumeEnvSustainMul(float db, ModulationType modType);

		// 调制器调制音量包络的keyToHold
		bool ModulationVolumeEnvKeyToHold(float cents, ModulationType modType);

		// 调制器调制音量包络的keyToDecay
		bool ModulationVolumeEnvKeyToDecay(float cents, ModulationType modType);

		// 调制器调制Mod包络的延迟时长
		// <param name="cents">以音分为单位</param>
		bool ModulationModEnvDelaySec(float cents, ModulationType modType);

		// 调制器调制mod包络的起音时长
		// <param name="cents">以音分为单位</param>
		bool ModulationModEnvAttackSec(float cents, ModulationType modType);

		// 调制器调制mod包络的保持时长
		// <param name="cents">以音分为单位</param>
		bool ModulationModEnvHoldSec(float cents, ModulationType modType);

		// 调制器调制mod包络的衰减时长
		// <param name="cents">以音分为单位</param>
		bool ModulationModEnvDecaySec(float cents, ModulationType modType);

		// 调制器调制mod包络的释音时长
		// <param name="cents">以音分为单位</param>
		bool ModulationModEnvReleaseSec(float cents, ModulationType modType);

		// 调制器调制mod包络的延音分贝
		// sustainMul值域:[0, 1]
		// 可以从[0, 100]线性转换为[0,1]，db/100
		bool ModulationModEnvSustainMul(float value, ModulationType modType);

		// 调制器调制mod包络的keyToHold
		bool ModulationModEnvKeyToHold(float cents, ModulationType modType);

		// 调制器调制mod包络的keyToDecay  
		bool ModulationModEnvKeyToDecay(float cents, ModulationType modType);

		// 调制器调制音mod包络调制Pitch,单位:音分
		bool ModulationModEnvModPitch(float cents, ModulationType modType);

		// 调制器调制音mod包络调制截至频率,单位:音分
		//  cents转倍率: pitchMul = 1.00057779^cents 倍
		bool ModulationModEnvModFc(float cents, ModulationType modType);

		// 调制器调制StartAddrsOffset 
		bool ModulationStartAddrsOffset(float offset, ModulationType modType);

		// 调制器调制StartAddrsCoarseOffset
		// 粗粒度调制
		bool ModulationStartAddrsCoarseOffset(float offset, ModulationType modType);

		// 调制器调制EndAddrsOffset
		bool ModulationEndAddrsOffset(float offset, ModulationType modType);

		// 调制器调制EndAddrsCoarseOffset
		// 粗粒度调制
		bool ModulationEndAddrsCoarseOffset(float offset, ModulationType modType);

		// 调制器调制StartloopAddrsOffset
		bool ModulationStartloopAddrsOffset(float offset, ModulationType modType);

		// 调制器调制StartloopAddrsCoarseOffset
		// 粗粒度调制
		bool ModulationStartloopAddrsCoarseOffset(float offset, ModulationType modType);

		// 调制器调制EndloopAddrsOffset
		bool ModulationEndloopAddrsOffset(float offset, ModulationType modType);

		// 调制器调制EndloopAddrsCoarseOffset
		// 粗粒度调制
		bool ModulationEndloopAddrsCoarseOffset(float offset, ModulationType modType);


		// 调制器调制SustainPedalOnOff
		bool ModulationSustainPedalOnOff(float onoff, ModulationType modType);

	private:

		//是否使用外部普通调制器
		bool useCommonModulator = false;

		//是否初始化过
		bool isInited = false;

		//这个控制器列表仅设置了供内部使用的控制器
		ModulatorList* insideCtrlModulatorList = nullptr;

		GeneratorList* genList = nullptr;
		GeneratorList* processGenList = nullptr;

		unordered_set<int>* modifyedGenTypes = nullptr;

		//对每个已调制的乐器生成器项作bool标记
		bool isModedInstGenTypes[64] = { false };

		//对每个已调制的预设生成器项作bool标记
		bool isModedPresetGenTypes[64] = { false };

		Channel* channel = nullptr;

		// 乐器区域
		Region* instRegion = nullptr;

		// 乐器全局区域
		Region* instGlobalRegion = nullptr;

		// 预设区域
		Region* presetRegion = nullptr;

		// 预设全局区域
		Region* presetGlobalRegion = nullptr;

		// 乐器合并生成器数据表
		// 这个值是复合同一区域的乐器生成列表(instRegion)
		// 和乐器全局生成器列表(instGlobalRegion)值的结果
		GeneratorList* instCombGenList = nullptr;

		// 乐器区域修改后的生成器数据表
		GeneratorList* instRegionModGenList = nullptr;

		// 乐器全局区域修改后的生成器数据表
		GeneratorList* instGlobalRegionModGenList = nullptr;

		// 预设合并生成器数据表
		GeneratorList* presetCombGenList = nullptr;

		// 预设区域调制后的生成器数据表
		GeneratorList* presetRegionModGenList = nullptr;

		// 预设全局区域调制后的生成器数据表
		GeneratorList* presetGlobalRegionModGenList = nullptr;

		// 调制器调制后的生成器数据表
		GeneratorList* modsModGenList = nullptr;

		//复合区域的乐器生成器列表和乐器全局生成器列表，预设生成器列表和预设全局生成器列表
		//获得最终的修改后生成器列表
		//最终修改后的生成器数据表
		GeneratorList* modifyedGenList = nullptr;
	};
}
#endif
