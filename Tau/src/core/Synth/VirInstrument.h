﻿#ifndef _VirInstrument_h_
#define _VirInstrument_h_

#include"TauTypes.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include"MidiTrackRecord.h"
#include"FX/TauEffect.h"
#include"FX/EffectList.h"
using namespace tauFX;

namespace tau
{

	// 虚拟乐器状态
	enum class VirInstrumentState
	{
		//打开中
		ONING,
		//已经打开
		ONED,
		//关闭中
		OFFING,
		//已经关闭
		OFFED,
	};

	// 虚拟乐器状态操作类型
	enum class VirInstrumentStateOpType
	{
		ON,
		OFF,
		REMOVE
	};

	// 虚拟乐器状态操作
	struct VirInstrumentStateOp
	{
		VirInstrumentStateOpType opType;
		bool isFade;
	};


	/*
	* 虚拟乐器类
	* 通过指定通道和预设来确认当前乐器所在的演奏空间
	* 这个类主要用来处理同为一个空间的一种乐器的一组按键发音器(KeySounder)的方法结合
	* by cymheart, 2020--2021.
	*/
	class DLL_CLASS VirInstrument
	{
	public:
		VirInstrument(Synther* synther, Channel* channel, Preset* preset);
		~VirInstrument();

		//获取Tau
		Tau* GetTau()
		{
			return tau;
		}

		//获取乐器所在通道
		Channel* GetChannel()
		{
			return channel;
		}

		//获取状态
		VirInstrumentState GetState()
		{
			return state;
		}

	private:

		inline void SetRealtime(bool isRealtime)
		{
			this->isRealtime = isRealtime;
		}

		inline bool IsRealtime()
		{
			return isRealtime;
		}

		//获取乐器的预设
		inline Preset* GetPreset()
		{
			return preset;
		}

		//清除发音数据
		void ClearSoundDatas();

		//设置是否开启录制功能
		void SetEnableRecordFunction(bool isEnable)
		{
			isEnableRecordFunction = isEnable;
		}

		//打开乐器
		void On(bool isFade = true);

		//关闭乐器
		void Off(bool isFade = true);

		//移除乐器
		void Remove(bool isFade);

		inline bool IsRemove()
		{
			return isRemove;
		}

		//状态操作
		void StateOp();

		//执行打开乐器
		void OnExecute(bool isFade = true);

		//执行关闭乐器
		void OffExecute(bool isFade = true);

		//增加效果器
		void AddEffect(TauEffect* effect);

		// 获取左声道已处理采样数据
		float* GetLeftChannelSamples()
		{
			return leftChannelSamples;
		}

		// 获取右声道已处理采样数据
		float* GetRightChannelSamples()
		{
			return rightChannelSamples;
		}

		//所有keySounder发音是否结束
		bool IsAllKeySoundEnd();

		//乐器是否发声结束,包括效果器作用带来的尾音是否结束
		inline bool IsSoundEnd()
		{
			return isSoundEnd;
		}

		//获取区域发声数量
		inline int GetRegionSounderCount()
		{
			return regionSounderCount;
		}

		//获取区域发声数组
		inline RegionSounder** GetRegionSounders()
		{
			return regionSounders;
		}

		//最后一个保留按键
		inline KeySounder* GetLastKeySounder()
		{
			return lastKeySounder;
		}

		//移除指定按下状态的按键发音器
		bool _RemoveOnKeyStateSounder(KeySounder* keySounder);

		//移除指定按下状态的按键发音器
		void RemoveOnKeyStateSounder(KeySounder* keySounder);

		// 获取最后在按状态的按键发音器(不包括最后松开的按键)
		KeySounder* GetLastOnKeyStateSounder();

		// 在Mono模式下重新发音最后一个按键
		void MonoModeReSoundLastOnKey();

		//从正在发音的KeySounders中查找KeySounder
		bool FindKeySounderFromKeySounders(KeySounder* keySounder);

		// 找寻最后一个按键发声区域中具有同样乐器区域的regionSounder
		RegionSounder* FindLastSameRegion(Region* region);

		//是否使用单音模式
		inline bool UseMonoMode()
		{
			return useMonoMode;
		}

		// 获取是否总是使用滑音   
		inline bool AlwaysUsePortamento()
		{
			return alwaysUsePortamento;
		}

		// 获取是使用滑音   
		inline bool UsePortamento()
		{
			return usePortamento;
		}

		//获取滑音过渡时间
		inline float GetPortaTime()
		{
			return portaTime;
		}

		//是否使用连音
		inline bool UseLegato()
		{
			return useLegato;
		}

		//调制生成器参数
		void ModulationParams();

		//调制生成器参数
		void ModulationParams(int key);

		//调制输入按键生成器参数
		void ModulationInputKeyParams();

		// 移除已完成所有区域发声处理(采样处理)的KeySounder   
		void RemoveProcessEndedKeySounder();

		//清除通道样本缓存
		void ClearChannelSamples();

		//应用效果器到乐器的声道buffer
		void ApplyEffectsToChannelBuffer();


		inline void SetPreset(Preset* preset)
		{
			this->preset = preset;
		}

		//设置滑音过渡时间
		inline void SetPortaTime(float tm)
		{
			portaTime = tm;
		}

		//设置是否使用连音
		inline void SetUseLegato(bool isUse)
		{
			useLegato = isUse;
		}

		//设置是否使用单音模式  
		inline void SetUseMonoMode(bool isUse)
		{
			useMonoMode = isUse;
		}

		// 设置是否总是使用滑音    
		inline void SetAlwaysUsePortamento(bool isAlwaysUse)
		{
			alwaysUsePortamento = isAlwaysUse;
		}

		// 设置是否使用滑音    
		inline void SetUsePortamento(bool isUse)
		{
			usePortamento = isUse;
		}

		//设置是否开启效果器
		void SetEnableEffects(bool isEnable)
		{
			isEnableEffect = isEnable;
			isEnableInnerEffects = isEnable;
		}

		//获取当前混音深度
		inline float GetCurtReverbDepth()
		{
			return curtReverbDepth;
		}

		//设置过渡效果深度信息
		void SettingFadeEffectDepthInfo(float maxEffectDepth, FadeEffectDepthInfo& fadeEffectDepthInfo);

		//过渡混音深度
		void FadeReverbDepth();

		//设置区域混音深度
		void SetRegionReverbDepth(float value);



		//bend
		void SetPitchBend(int value);

		//PolyPressure
		void SetPolyPressure(int key, int pressure);

		//改变乐器
		void ChangeProgram(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置乐器号
		void SetProgramNum(int num);

		//设置控制器
		void SetController(MidiControllerType ctrlType, int value);

		//按键
		void OnKey(int key, float velocity, int tickCount = 0);

		//松开按键
		void OffKey(int key, float velocity = 127.0f);

		//松开所有按键
		void OffAllKeys();

		//结束所有按键发音
		void EndOnKeySounds();

		//执行按键
		KeySounder* OnKeyExecute(int key, float velocity);

		//执行松开按键
		void OffKeyExecute(int key, float velocity = 127.0f);


		// 松开指定发音按键
		void _OffKey(KeySounder* keySounder, float velocity);

		//按键执行
		void _OnKey(KeySounder* keySounder, int key, float velocity);

		// 设置具有相同独占类的区域将不再处理样本
		// exclusiveClasses数组以一个小于等于0的值结尾
		void StopExclusiveClassRegionSounderProcess(int* exclusiveClasses);

		// 录制指定乐器弹奏为midi
		void RecordMidi();

		//停止录制midi
		void StopRecordMidi();

		// 获取录制的midi轨道
		MidiTrack* TakeMidiTrack(float baseTickForQuarterNote, vector<RecordTempo>* tempos);

		//合并区域已处理发音样本
		void CombineRegionSounderSamples(RegionSounder* regionSounder);

		//生成发声keySounders
		void CreateKeySounders();

		//为渲染准备所有正在发声的区域
		int CreateRegionSounderForRender(RegionSounder** totalRegionSounder, int startSaveIdx);

		void PrintOnKeyInfo(int key, float velocity, bool isRealTime);

		//是否可以忽略按键
		bool CanIgroneOnKey(int key, float velocity, int tickCount, bool isRealTime);
		//计算按键速度
		void ComputeOnKeySpeed();


	private:
		VirInstStateChangedCallBack virInstStateChangedCB = nullptr;

	private:

		//是否实时
		bool isRealtime = true;

		//发音结束,包括效果器作用带来的尾音是否结束
		bool isSoundEnd = true;

		//效果器
		EffectList* effects;
		//是否启用效果器
		bool isEnableEffect = false;

		//内部效果器
		EffectList* innerEffects;
		//是否启用效果器
		bool isEnableInnerEffects = false;

		//过渡区域混音深度信息
		FadeEffectDepthInfo fadeReverbDepthInfo;

		// 区域混音处理
		Reverb* regionReverb = nullptr;

		//当前混音深度
		float curtReverbDepth = 0;


		// 使用单音模式  
		bool useMonoMode = false;

		// 使用连音  
		//如果前一个按键没有松开，即触发第二个按键，将触发连音
		//此时第二个按键发音将没有attact的过程
		bool useLegato = false;

		// 是否总是使用滑音
		bool alwaysUsePortamento = false;
		// 是否使用滑音
		bool usePortamento = false;

		//滑音过渡时间(单位:秒)
		float portaTime = 0.015f;

		//
		Synther* synther = nullptr;
		Tau* tau = nullptr;

		//乐器所在的通道
		Channel* channel = nullptr;

		//乐器所在预设
		Preset* preset = nullptr;

		// 发音中的按键发声器
		KeySounderList* keySounders = nullptr;

		//按键中的keySounders
		vector<KeySounder*>* onKeySounders = nullptr;
		vector<KeyEvent>* keyEvents = nullptr;

		//按键时间点
		list<float>* onKeySecHistorys = nullptr;
		//按键速率
		float onKeySpeed = 0;

		//
		bool isRemove = false;
		//状态
		VirInstrumentState state = VirInstrumentState::OFFED;
		//状态操作
		queue<VirInstrumentStateOp>* stateOps;
		//是否可以执行状态操作
		bool canExecuteStateOp = false;


		//gain
		float gain = 1;
		float startGain = 0;
		float dstGain = 0;
		float startGainFadeSec = 0;
		float totalGainFadeTime = 0.1f;

		// 左通道已处理采样点
		float* leftChannelSamples;

		// 右通道已处理采样点
		float* rightChannelSamples;

		//最后一个松开按键的键
		//最后一个保持按键状态的keySounder,总是不被直接从内存池中移除，
		//而是放入lastKeySounder，供总是保持滑音状态使用，因为“保持滑音状态”
		//需要最后一个发音作参考，来生成滑音,正常状态的滑音是靠两个同时处于按键状态的keySounder先后来生成的
		//当开起一直保持功能后，生成滑音只需要最近一个按键的发音信息即可，而无所谓是否按下还是已经按下过
		KeySounder* lastKeySounder = nullptr;

		int exclusiveClasses[1024 * 10] = { 0 };
		KeySounder* offKeySounder[1024 * 200];

		//当前乐器中所有正在发声的区域
		RegionSounder* regionSounders[100000] = { nullptr };
		//当前乐器中所有正在发声的区域数量
		int regionSounderCount = 0;

		int realRegionSounderCount = 0;

		//midiTrack录制
		MidiTrackRecord* midiTrackRecord;
		//是否启用录制功能
		bool isEnableRecordFunction = true;


		//
		friend class Synther;
		friend class RegionSounderThread;
		friend class MidiEditor;
		friend class KeySounder;
		friend class RegionSounder;
		friend class Synther;
		friend class RealtimeSynther;
		friend class MidiRender;
	};
}

#endif