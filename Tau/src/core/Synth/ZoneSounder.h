#ifndef _ZoneSounder_h_
#define _ZoneSounder_h_

#include"TauTypes.h"
#include"Envelope.h"
#include"Generator.h"
#include"Sample.h"
#include"Midi/MidiTypes.h"
#include"InsideModulators.h"
#include"DaisySP/daisysp.h"
#include <stk/Chorus.h>
#include"iir/Iir1.h"

namespace tau
{
	/*
	* 区域发声器
	* 区域发声器将会根据区域(Zone)中生成器，以及调制器共同作用来控制样本的每帧的发声方式
	* 并输出处理后的每帧样本供后续的声音渲染程序使用
	* by cymheart, 2020--2025.
	*/
	class ZoneSounder
	{
	public:
		ZoneSounder();
		~ZoneSounder();


		//清理
		void Clear();

		// 初始化
		void Init();

		static ZoneSounder* New();

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

		inline void SetSec(float sec)
		{
			this->sec = sec;
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

		inline KeySounder* GetKeySounder()
		{
			return keySounder;
		}

		//是否保持按键状态
		//当设置延音踏板SustainPedalOnOff时设置
		//SustainPedalOnOff< 64 : isHoldDownKey = false;
		//SustainPedalOnOff>=64 : isHoldDownKey = true;
		inline bool IsHoldDownKey()
		{
			return isHoldDownKey;
		}

		//设置是否保持按键状态
		inline void SetHoldDownKey(bool isHold)
		{
			isHoldDownKey = isHold;
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

		//设置静态样本
		void SetSample(Sample* sample)
		{
			this->sample = sample;
		}

		//设置动态样本生成器
		void SetSampleGen(SampleGenerator* sampleGen)
		{
			this->sampleGen = sampleGen;
		}

		// 获取最终修改后的生成器的独占发音归类的数据
		GeneratorAmount GetGenExclusiveClass()
		{
			return resultGens.GetAmount(GeneratorType::ExclusiveClass);
		}

		// 获取混音深度
		GeneratorAmount GetGenReverbEffectsSend()
		{
			return resultGens.GetAmount(GeneratorType::ReverbEffectsSend);
		}

		// 获取和声深度
		GeneratorAmount GetGenChorusEffectsSend()
		{
			return resultGens.GetAmount(GeneratorType::ChorusEffectsSend);
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

		// 设置力度
		void SetVelocity(float vel);

		// 按下对应的键
		void OnKey(int key, float vel);


		// 松开按键    
	    // releaseSec : 重设按键释放时长， -1为不设置
		void OffKey(float velocity, float releaseSec = -1);

		void SetFrameBuffer(float* leftChannelBuf, float* rightChannelBuf);

		// 时序序列采样块处理
		void Render();

		//设置内部调制器列表
		void SetInsideModulators(InsideModulators* mods) {
			insideModulators = mods;
		}

		//调制
		void Modulation();

	private:

		//计算区域的生成器列表
		//复合同一区域的乐器生成列表(instZone)和乐器全局生成器列表(instGlobalZone)值到modifyedGenList中
	    //复合方法: 优先使用instZone中的值，如果没有将使用instGlobalZone的值，
	    //然后叠加preset的值
	    //这个复合处理在ZoneSounder建立后只需要处理一次，将会直到ZoneSounder发声结束一直缓存这组值
		void ComputeZoneGens();

		//计算区域的调制器列表
		void ComputeZoneMods();
		//计算区域的结果调制器列表
		void ComputeZoneResultMods();
		//调制ResultGens
		void ModResultGens();
	
		//渲染的声道buffer是否为0值
		bool IsZeroValueRenderChannelBuffer();

		//检测由效果器带来的尾音是否结束
		bool IsEffectSoundStop();

		//增加和通道对应的内部控制器相关调制器
		void AddInsideModulators();

		//设置lfo或者包络的调制信息
		void SetLfoEnvModInfo(
			vector<LfoModInfo>& infos, GeneratorType genType, 
			UnitTransformCallBack unitTransform, float amountMul = 1);

		//设置采样模式
		void SetSampleModes();
		//设置基音调
		void SetBasePitchMul();
		//设置音量衰减值
		void SetAttenuation();
		//设置声向
		void SetPan();
		//设置和声深度
		void SetChorusDepth();
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
		// 设置压力值
		void SetPressure();

		// 使用滑音
		void UsePortamento();

		// 使用连音
		void UseLegato();

		// 计算按键key相对于根音符的频率偏移倍率
		// <param name="key"></param>
		float CalBasePitchMulByKey(int key);

		// 滑音处理
		float PortamentoProcess(float sec);

		float NextAdjustPitchSample(float sampleSpeed);

		// 重设低通滤波器
		void ResetLowPassFilter(float computedSec);

		// lfos, envs调制音调
		float LfosAndEnvsPitch(float sec);

		// lfos, envs调制截至频率(返回值单位:cents)
		short LfosAndEnvsFc(float sec);

		// lfos, envs调制声音
		float LfosAndEnvsVolume(float sec);

		//过渡参数值
		void FadeParams();
		float FadeValue(float orgValue, ValueFadeInfo fadeInfo);

		void ChorusProcessSample(int idx);

	public:

		//是否实时控制发声区域
		bool isRealtimeControl = true;

		Synther* synther = nullptr;
		Tau* tau = nullptr;
		VirInstrument* virInst = nullptr;
		KeySounder* keySounder = nullptr;

		// 乐器区域
		Zone* instZone = nullptr;

		// 乐器全局区域
		Zone* instGlobalZone = nullptr;

		// 预设区域
		Zone* presetZone = nullptr;

		// 预设全局区域
		Zone* presetGlobalZone = nullptr;

		// 左通道已处理采样点
		float* leftChannelSamples = nullptr;

		// 右通道已处理采样点
		float* rightChannelSamples = nullptr;

	private:

		unordered_set<int> modifyedGenTypes;

		//发声样本
		Sample* sample = nullptr;

		//动态样本生成器
		SampleGenerator* sampleGen = nullptr;
		//动态区域样本生成器
		ZoneSampleGenerator* zoneSampleGen = nullptr;
		//
		daisysp::Chorus* chorus[2];
		Iir::RBJ::LowPass* biquad = nullptr;
		double biquadSampleRate = 48000;
		short biquadFcCents = 0;
		double biquadQ = ONESQRT2;

		//是否保持按键状态
		//当设置延音踏板SustainPedalOnOff时设置
		//SustainPedalOnOff< 64 : isHoldDownKey = false;
		//SustainPedalOnOff>=64 : isHoldDownKey = true;
		bool isHoldDownKey = false;

		// 是否激活低通滤波
		bool isActiveLowPass = false;

		// 已处理的采样点计数
		int processedSampleCount = 0;

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

		//是否是循环样本
		bool isLoopSample = true;

		// 采样处理是否结束
		bool isSampleProcessEnd = false;

		// 发音是否结束
		// 包括了采样处理结束，和效果音残余处理结束
		bool isSoundEnd = false;

		// 当前处理采样的时间点
		float sec = 0;

		//是否需要松开按键
		bool isNeedOffKey = false;

		//按键是否是按下状态
		bool isDownNoteKey = false;
		int downNoteKey = 60;

		// 按键力度，1为最大力度(音量最大)， 0为无力度(音量最小)
		float velocity = 1;
		// 按键力度过渡信息
		ValueFadeInfo velocityFadeInfo;

		// 音量衰减
		float attenuation = -1;
		// 音量衰减过渡信息
		ValueFadeInfo attenFadeInfo;

		// 左右通道音量增益
		//0:left
		//1:right
		float channelGain[2] = { -1 };
		// 左右通道音量增益过渡信息
		ValueFadeInfo channelGainFadeInfo[2];


		// 和声深度衰减
		float chorusDepth = -1;
		// 和声深度衰减过渡信息
		ValueFadeInfo chorusDepthFadeInfo;

		// 低频滤波截至频率(计算值)
		short fcCompute = -1;

		// 低频滤波截至频率
		short fcCents = 0;

		// 低频滤波Q值
		float Q = 0;

		//
		LoopPlayBackMode loopPlayBack = LoopPlayBackMode::NonLoop;

		//最后在原始源中定位的采样位置
		//即已采样处理到此处
		//注意，此处为保证累加精度，必须使用double类型，不然变调声音会不平滑
		double lastSamplePos = -1;

		// 样本起始点位置
		int sampleStartIdx = 0;

		// 样本结束点位置
		int sampleEndIdx = 0;

		// 样本循环起始点位置
		int sampleStartLoopIdx = 0;

		// 样本循环结束点位置
		int sampleEndLoopIdx = 0;

		// 样本处理块尺寸
		//sampleProcessBlockSize将控制调制的精度，sampleProcessBlockSize = 1，将会对每个采样都计算调制参数,
		//sampleProcessBlockSize = 64,则每64个采样点计算一次调制参数
		//注意此参数过大，会导致卡顿不流畅
		int sampleProcessBlockSize = 64;
		float invSampleProcessBlockSize = 1 / 64.0f;


		// 计算的生成器列表
		GeneratorList computedGens;
		// 结果生成器列表
		GeneratorList resultGens;


		// 计算的调制器列表
		vector<Modulator*> computedMods;
		// 计算的调制器对应数量列表
		float computedModAmounts[128];

		// 结果调制器列表
		vector<Modulator*> resultMods;
		// 结果调制器对应数量列表
		float resultModAmounts[128];


		// 颤音音调调制LFO
		Lfo* vibLfo = nullptr;

		// mod调制LFO
		Lfo* modLfo = nullptr;

		// 音量包络线调制
		Envelope* volEnv = nullptr;

		// mod包络线调制
		Envelope* modEnv = nullptr;


		vector<LfoModInfo> vibLfoInfos;
		vector<LfoModInfo> modLfoInfos;
		vector<LfoModInfo> volEnvInfos;
		vector<LfoModInfo> modEnvInfos;

		//内部调制器列表
		InsideModulators* insideModulators = nullptr;

	};
}



#endif
