﻿#ifndef _RegionSounder_h_
#define _RegionSounder_h_

#include"VentrueTypes.h"
#include"Envelope.h"
#include"Generator.h"
#include"Sample.h"
#include"Midi/MidiTypes.h"
#include"ModulatorList.h"
#include"RegionModulation.h"
#include"dsignal/Biquad.h"

using namespace stk;

namespace ventrue
{
	/*
	* 区域发声器
	* 区域发声器将会根据区域(Region)中生成器，以及调制器共同作用来控制样本的每帧的发声方式
	* 并输出处理后的每帧样本供后续的声音渲染程序使用
	* by cymheart, 2020--2021.
	*/
	class RegionSounder
	{
	public:
		RegionSounder();
		~RegionSounder();


		//清理
		void Clear();

		// 初始化
		void Init();

		static RegionSounder* New();

		// 释放
		void Release();

		// 发声是否结束
		//包括了采样处理结束，和效果音残余处理结束
		bool IsSoundEnd()
		{
			return isSoundEnd;
		}

		//处理不能听到发声的滞留区域，使其结束发音
		void EndBlockSound();

		// 直接结束发音
		inline void EndSound()
		{
			isDownNoteKey = false;
			isSampleProcessEnd = true;
			isSoundEnd = true;
		}

		//处理不能听到发声的滞留区域，使其结束发音
		void EndSoundLevel(int endLevel);

		// 需要松开按键
		inline void NeedOffKey()
		{
			isNeedOffKey = true;
		}

		inline VirInstrument* GetVirInstrument()
		{
			return virInst;
		}

		//是否保持按键状态
		//当设置延音踏板SustainPedalOnOff时设置
		//SustainPedalOnOff< 64 : isHoldDownKey = false;
		//SustainPedalOnOff>=64 : isHoldDownKey = true;
		inline bool IsHoldDownKey()
		{
			return isHoldDownKey;
		}

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

		void SetSample(Sample* sample)
		{
			this->sample = sample;
			input = sample->pcm;
		}


		// 获取最终修改后的生成器数据表
		inline GeneratorList* GetModifyedGenList()
		{
			return modifyedGenList;
		}

		// 获取最终修改后的生成器的独占发音归类的数据
		inline  float GetGenExclusiveClass()
		{
			return modifyedGenList->GetAmount(GeneratorType::ExclusiveClass);
		}

		// 获取混音深度
		inline  float GetGenReverbEffectsSend()
		{
			return modifyedGenList->GetAmount(GeneratorType::ReverbEffectsSend);
		}

		// 获取和声深度
		inline  float GetGenChorusEffectsSend()
		{
			return modifyedGenList->GetAmount(GeneratorType::ChorusEffectsSend);
		}

		// 获取当前采样处理的计算后的音符的频率偏移倍率
		inline float GetCurtCalBasePitchMul()
		{
			return curtCalBasePitchMul;
		}



		// 获取按键key相对于根音符的频率偏移倍率
		inline float GetBasePitchMul()
		{
			return basePitchMul;
		}

		// 获取音量包络的当前状态
		inline EnvStage GetVolEnvStage()
		{
			return volEnv->GetStage();
		}

		// 获取音量包络的当前值
		inline float GetVolEnvCurtValue()
		{
			return volEnv->GetCurtValue();
		}

		// 设置力度
		void SetVelocity(float velocity);

		// 按下对应的键
		void OnKey(int key, float velocity);


		// 松开按键    
		void OffKey(float velocity);

		void SetFrameBuffer(float* leftChannelBuf, float* rightChannelBuf);

		//渲染的声道buffer是否为0值
		bool IsZeroValueRenderChannelBuffer();

		// 时序序列采样块处理
		void Render();


		//调制生成器参数
		void ModulationParams();



	private:

		//设置不需要调制器调制的参数值
		void SetNotModParams(int key, float velocity);

		//为RegionModulation设置初始值
		void InitRegionModulation();
		//增加和通道对应的内部控制器相关调制器
		void AddInsideControllerModulators();

		//设置控制发声参数
		void SetParams();
		//设置采样模式
		void SetSampleModes();
		//设置基音调
		void SetBasePitchMul();
		//设置音量衰减值
		void SetAttenuationl();
		//设置声向
		void SetPan();
		//设置样本起始位置
		void SetSampleStartIdx();
		//设置样本结束位置
		void SetSampleEndIdx();
		//设置样本循环起始位置
		void SetSampleStartLoopIdx();
		//设置样本循环结束位置
		void SetSampleEndLoopIdx();
		//设置颤音lfo调制的音调
		void SetVibLfoToPitch();
		//设置颤音lfo的延迟
		void SetDelayVibLFO();
		//设置颤音lfo的频率
		void SetFreqVibLFO();
		//设置modlfo调制截止频率
		void SetModLfoToFilterFc();
		//设置modlfo调制音调
		void SetModLfoToPitch();
		//设置modlfo调制音量
		void SetModLfoToVolume();
		//设置modlfo的延迟
		void SetDelayModLFO();
		//设置modlof的频率
		void SetFreqModLFO();
		//设置音量包络线控制参数
		void SetVolEnv();
		//设置调制包络线控制参数
		void SetModEnv();
		//设置调制包络线调制音调
		void SetModEnvToPitch();
		//设置调制包络线调制截止频率
		void SetModEnvToFilterFc();
		// 设置双二阶Biquad滤波器参数
		void SetBiquadParams();
		// 设置延音踏板开关值
		void SetSustainPedalOnOff();


		// 使用滑音
		void UsePortamento();

		// 使用连音
		void UseLegato();


		// 计算按键key相对于根音符的频率偏移倍率
		// <param name="key"></param>
		float CalBasePitchMulByKey(int key);

		void OffKeyEnvs();



		// 启动所有LFO调制
		void OpenLfos();

		// 启动所有包络控制
		void OpenEnvs();


		// 增加一个Lfo调制到目标
		// <param name="lfo"></param>
		// <param name="modValue">调制最大值</param>
		// <param name="target">调制目标</param>
		int AddLfoModTarget(Lfo* lfo, float modValue, GeneratorType genType, UnitTransformCallBack unitTrans, LfoEnvTarget target);
		int GetLfoModTargetIdx(LfoEnvTarget target, GeneratorType genType);


		// 增加一个包络线调制到目标 
		// <param name="env">包络线</param>
		// <param name="modValue">调制最大值</param>
		// <param name="target">调制目标</param>
		int AddEnvModTarget(Envelope* env, float modValue, GeneratorType genType, UnitTransformCallBack unitTrans, LfoEnvTarget target);

		int GetEnvModTargetIdx(LfoEnvTarget target, GeneratorType genType);

		// 滑音处理
		float PortamentoProcess(float sec);

		float NextAdjustPitchSample(float sampleSpeed);

		// 重设低通滤波器
		void ResetLowPassFilter(float computedSec);

		// lfos, envs调制
		float LfosAndEnvsModulation(LfoEnvTarget modTarget, float computedSec);



	public:

		//是否实时控制发声区域
		bool isRealtimeControl = true;

		Ventrue* ventrue = nullptr;
		VirInstrument* virInst = nullptr;

		// 乐器区域
		Region* instRegion = nullptr;

		// 乐器全局区域
		Region* instGlobalRegion = nullptr;

		// 预设区域
		Region* presetRegion = nullptr;

		// 预设全局区域
		Region* presetGlobalRegion = nullptr;

		// 左通道已处理采样点
		float* leftChannelSamples = nullptr;

		// 右通道已处理采样点
		float* rightChannelSamples = nullptr;

	private:

		GeneratorType modifyedGenTypes[64];

		Sample* sample = nullptr;
		float* input = nullptr;

		Biquad* biquad = nullptr;

		//是否保持按键状态
		//当设置延音踏板SustainPedalOnOff时设置
		//SustainPedalOnOff< 64 : isHoldDownKey = false;
		//SustainPedalOnOff>=64 : isHoldDownKey = true;
		bool isHoldDownKey = false;

		// 是否激活低通滤波
		bool isActiveLowPass = false;

		// 已处理的采样点计数
		int processedSampleCount = 0;

		// 左通道音量增益
		float leftChannelGain = 0.5f;

		// 右通道音量增益
		float rightChannelGain = 0.5f;

		// 按键key相对于根音符的频率偏移倍率
		float basePitchMul = 1;

		// 当前采样处理的计算后的音符的频率偏移倍率
		float curtCalBasePitchMul = 1;

		// 最后一个按键的基础频率偏移倍率
		float lastKeyBasePitchMul = 1;

		// 是否激活滑音处理
		bool isActivePortamento = false;

		// 是否激活连音
		bool isActiveLegato = false;

		bool isLoopSample = true;

		// 采样处理是否结束
		bool isSampleProcessEnd = false;

		// 发音是否结束
		// 包括了采样处理结束，和效果音残余处理结束
		bool isSoundEnd = false;

		// 当前处理采样的时间点
		float sec = 0;

		bool isNeedOffKey = false;
		bool isDownNoteKey = false;
		int downNoteKey = 60;

		// 按键力度，1为最大力度(音量最大)， 0为无力度(音量最小)
		float velocity = 1;

		// 音量衰减
		float attenuation = -1;
		// 目标音量衰减
		float dstAttenuation = -1;
		//衰减过渡起始值
		float orgAttenuation = -1;
		//总衰减过渡值
		float totalAttenFadeValue = 0;
		//开始衰减过渡时间点
		float startAttenFadeSec = 0;
		//总衰减过渡时间
		float totalAttenFadeTime = 0.03f;
		//衰减过渡计算值
		float attenFadeComputedValue = 0;


		// 低频滤波截至频率
		float fc = 0;

		// 低频滤波Q值
		float Q = 0;

		//
		LoopPlayBackMode loopPlayBack = LoopPlayBackMode::NonLoop;

		//最后在原始源中定位的采样位置
		//即已采样处理到此处
		float lastSamplePos = -1;

		// 样本起始点位置
		uint32_t sampleStartIdx = 0;

		// 样本结束点位置
		uint32_t sampleEndIdx = 0;

		// 样本循环起始点位置
		uint32_t sampleStartLoopIdx = 0;

		// 样本循环结束点位置
		uint32_t sampleEndLoopIdx = 0;

		// 样本处理块尺寸
		//sampleProcessBlockSize将控制调制的精度，sampleProcessBlockSize = 1，将会对每个采样都计算调制参数,
		//sampleProcessBlockSize = 64,则每64个采样点计算一次调制参数
		//注意此参数过大，会导致卡顿不流畅
		int sampleProcessBlockSize = 64;
		float invSampleProcessBlockSize = 1 / 64.0f;

		float sampleProcessRate = 44100;

		//子帧细分采样数量
		int childFrameSampleCount = 64;

		float invSampleProcessRate = 1.0f / 44100.0f;

		// 最终修改后的生成器数据表
		GeneratorList* modifyedGenList = nullptr;

		// lfo列表
		LfoModInfoList* lfoInfoLists[(int)LfoEnvTarget::ModCount];

		// 包络线列表
		EnvModInfoList* envInfoLists[(int)LfoEnvTarget::ModCount];

		// 当前处理区域的被调制后的生成器数据表
		GeneratorList* genList = nullptr;

		// 当前处理区域外部设置的生成器数据表
		GeneratorList* settingGenList = nullptr;

		// 当前处理区域的全局外部设置的生成器数据表
		GeneratorList* globalSettingGenList = nullptr;

		// 颤音音调调制LFO
		Lfo* vibLfo = nullptr;

		// mod调制LFO
		Lfo* modLfo = nullptr;

		// 音量包络线调制
		Envelope* volEnv = nullptr;

		// mod包络线调制
		Envelope* modEnv = nullptr;

		//区域调制处理器
		RegionModulation* regionModulation = nullptr;

		//内部控制器调制器列表
		//这个控制器列表仅设置了供内部使用的控制器
		ModulatorList insideCtrlModulatorList;
	};
}



#endif