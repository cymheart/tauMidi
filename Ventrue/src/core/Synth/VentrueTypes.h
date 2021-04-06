#ifndef _VentrueTypes_h_
#define _VentrueTypes_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include "scutils/Semaphore.h"
#include "task/TaskProcesser.h"
#include "scutils/ObjectPool.h"
#include "scutils/RingBuffer.h"
#include "Effect/EffectList.h"
#include "iir1/iir/Iir1.h"

using namespace task;
using namespace scutils;

namespace ventrue
{

	enum class GeneratorType :int;
	enum class ModInputPreset :int;

	class Ventrue;
	class NoteOnEvent;
	class Lfo;
	class Preset;
	class Region;
	class Instrument;
	class ModInputInfo;
	class Modulator;
	class Sample;
	class RegionSounder;
	class Envelope;
	class KeySounder;
	class MidiPlay;
	class Channel;
	class Track;
	class MidiFile;
	class GeneratorList;
	class RegionSounderThread;
	class RegionSounderThreadData;
	class VirInstrument;
	class SoundFontParser;
	class VentrueEvent;

	struct LfoModInfo;
	struct EnvModInfo;
	struct SamplesLinkToInstRegionInfo;
	struct InstLinkToPresetRegionInfo;
	struct LineEquationInfo;
	struct NoteOnEvOnKeyInfo;
	struct RealtimeKeyEvent;

	using NoteOnEvOnKeyInfoList = vector<NoteOnEvOnKeyInfo>;

	using SampleList = vector<Sample*>;
	using InstrumentList = vector<Instrument*>;
	using PresetList = vector<Preset*>;
	using ModInputInfoList = vector <ModInputInfo*>;
	using ModulatorVec = vector <Modulator*>;
	using SamplesLinkToInstRegionInfoList = vector <SamplesLinkToInstRegionInfo>;
	using InstLinkToPresetRegionInfoList = vector <InstLinkToPresetRegionInfo>;
	using LineEquationInfoList = vector <LineEquationInfo>;
	using LfoModInfoList = vector <LfoModInfo>;
	using EnvModInfoList = vector <EnvModInfo>;
	using RegionSounderList = vector <RegionSounder*>;
	using RegionSounderThreadDataList = vector <RegionSounderThreadData*>;
	using MidiPlayList = vector <MidiPlay*>;
	using MidiFileList = vector <MidiFile*>;
	using GeneratorTypeList = vector <GeneratorType>;
	using VirInstList = vector <VirInstrument*>;
	using ModPresetTypeList = vector<ModInputPreset>;
	using TrackList = vector <Track*>;


	using KeySounderList = list<KeySounder*>;
	using RegionSounderQueue = list<RegionSounder*>;
	using RealtimeKeyEventList = list<RealtimeKeyEvent>;


	using PresetMap = unordered_map<uint32_t, Preset*>;
	using ChannelMap = unordered_map<uint32_t, Channel*>;
	using SoundFontParserMap = unordered_map<string, SoundFontParser*>;

	typedef uint64_t KeySounderID;

	using UnitTransformCallBack = float (*)(float value);
	using ModTransformCallBack = float (*)(float value);
	using RenderTimeCallBack = void (*)(float sec, void* data);

#ifdef _WIN32
	template struct DLL_CLASS atomic<bool>;
#endif

	// LfoEnv调制目标
	enum class LfoEnvTarget
	{
		// 音调调制(单位:频率倍率)
		// 如果为一段采样的原始音调提高3倍，意思为在原始音调频率上*3倍得到频率即为目标音调
		// 频率倍率增加2倍，则音调升高1个八度
		// 1个cent的频率倍率为 2^(1/1200) = 1.00057779  倍
		// cent = 2, 则频率倍率增加或减少 mul = 1.00057779^2  倍
		// cent = n, 则频率倍率增加或减少 mul = 1.00057779^n  倍
		// semit = n,则频率倍率增加或减少 mul = 1.059463^n    倍
		// octave = n, 则频率倍率增加或减少 mul = 2^n         倍
		ModPitch,


		// 音量调制(单位:倍率)
		// dB增益转线性增益倍率: volLinearGainMul = Math.Pow(10, volDbGain / 10f);  
		ModVolume,


		// 音量左右声道平衡调制(单位:数量值)
		// 范围: -50 ~ 50
		ModPan,


		// 截至频率调制(单位:倍率)
		// cent转倍率： 1.00057779^cent,  cent = 0, 则mul = 1
		// 此cent为相对值
		ModFilterCutoff,

		ModCount
	};

	//声音渲染品质
	enum class RenderQuality
	{
		//快速
		Fast,
		//好质量,这个选项效率与质量平衡，推荐选择
		Good,
		//高品质
		High,
		//超高品质
		SuperHigh,
	};

	//声道输出模式
	enum class ChannelOutputMode
	{
		//单声道
		Mono = 1,

		//立体声
		Stereo = 2
	};


	// 样本类型
	enum class SampleType
	{
		// 单声道
		MonoSample = 1,

		// 右声道
		RightSample = 2,

		// 左声道
		LeftSample = 4,

		// 连接
		LinkedSample = 8,

		RomMonoSample = 0x8001,
		RomRightSample = 0x8002,
		RomLeftSample = 0x8004,
		RomLinkedSample = 0x8008
	};

	// 区域类型
	enum class RegionType
	{
		//乐器
		Insttrument,
		//预设
		Preset
	};


	// 循环播放模式
	enum class LoopPlayBackMode
	{
		// 无循环播放模式
		NonLoop = 0,

		// 连续循环播放模式
		// 此时样本播放长度会以音量包络的总时长为准
		Loop = 1,

		// 按键按下时循环播放，松开后播放剩余样本
		LoopEndContinue = 3,

	};


	// 生成器类型
	enum class GeneratorType :int
	{
		None = -1,


		// 在采样数据点中，从起始采样头到该乐器要播放的第一个采样数据点的偏移。
		// 例如，如果Start为7，startAddrOffset为2，则播放的第一个采样数据点将为采样数据点9。
		//
		StartAddrsOffset = 0,


		// 在采样数据点中，从结尾采样头到要为该乐器播放的最后一个采样数据点的偏移。
		// 例如，如果End为17，endAddrOffset为-2，则最后播放的采样数据点将为采样数据点15。
		//
		EndAddrsOffset = 1,


		// 在采样数据点中，超出Startloop采样头的偏移，该偏移量将在此乐器的播放循环中重复到第一个采样数据点。
		// 例如，如果Startloop为10，startloopAddrsOffset为-1，则第一个重复循环采样数据点将为采样数据点9。
		//
		StartloopAddrsOffset = 2,



		// 在采样数据点中，超出Endloop采样头的偏移，将被认为等效于此乐器循环的Startloop样本数据点。
		// 例如，如果Endloop为15而endloopAddrsOffset为2，则采样数据点17将被视为等同于Startloop采样数据点，
		// 因此采样数据点16将在循环期间有效地位于Startloop之前。
		//
		EndloopAddrsOffset = 3,


		// 以32768个采样数据点为单位的偏移量，超出了起始采样头和该乐器要播放的第一个采样数据点。
		// 此参数添加到startAddrsOffset参数。例如，如果“start”为5，且“ startAddrsOffset”为3，
		// “ startAddrsCoarseOffset”为2，则播放的第一个采样数据点将是样本数据点65543。
		//  5 + 2 + 2*32768
		//
		StartAddrsCoarseOffset = 4,


		// modLfo的调制音调
		//
		ModLfoToPitch = 5,


		// vibLfo的调制音调
		//
		VibLfoToPitch = 6,


		// mod包络调制音调
		//
		ModEnvToPitch = 7,


		// <para>截至频率(单位:Hz)</para>
		// <para>默认值:8.176Hz</para>
		// <para>cent转Hz : 8.176 * 2^(cent/1200)</para>
		// <para>定义以8.176Hz为cent的绝对0值, cent=0时，hz=8.176;  cent = 1200时，Hz = 8.176*2,正好是2倍</para>
		// <para>Hz转cent: 1200Log2[Hz/8.176]</para>
		//
		InitialFilterFc = 8,


		// <para>Q增益倍率因子决定了截止处的共振峰resonance peak(共振峰单位dB)</para>
		// <para>默认值:-3dB</para>
		// <para>共振峰resonance处的频率的增益明显高于其他频率。resonance可在直流增益上以dB为单位测量</para>
		// <para>换算: resonance = 20Log(10,Q)</para>
		// <para>换算: Q = Pow(10.0, (resonance / 20.0))</para>
		// <para>曲线平坦时，Q = 0.707, 即resonance = 20*Lg(0.707), resonance = -3dB，即为截至频率的增益</para>
		//
		InitialFilterQ = 9,


		// modLfo的调制截至频率
		// 单位:音分
		//
		ModLfoToFilterFc = 10,


		// mod包络调制截至频率
		//
		ModEnvToFilterFc = 11,



		//  以32768个采样数据点为单位的偏移量,在采样数据点中，超出Endloop采样头的偏移，将被认为等效于此乐器循环的Startloop样本数据点。
		// 例如，如果Endloop为15而endloopAddrsOffset为2，EndAddrsCoarseOffset = 2 ,则采样数据点 15 + 2 + 2*32768将被视为等同于Startloop采样数据点，
		// 因此采样数据点16将在循环期间有效地位于Startloop之前。
		//  15 + 2 + 2*32768
		//
		EndAddrsCoarseOffset = 12,


		// modLfo的调制音量
		//
		ModLfoToVolume = 13,


		// 和声
		//
		ChorusEffectsSend = 15,


		// 混响
		//
		ReverbEffectsSend = 16,


		// 左右声道音量增益平衡设置
		// 默认值: 0
		// 当pan = 0时,左右声道音量一样大都增益为0.5倍
		// 如果pan>=50, 那右声道增益为1倍，左声道增益为0倍
		// 如果pan 小于等于 -50，那左声道增益为1倍，右声道增益为0倍
		//
		Pan = 17,


		// modLfo的延迟时长  
		//delay最小值如果没有设置必须默认为0，不然会有断音现象
		// 默认值: 0
		DelayModLFO = 21,


		// modLfo的频率
		// 默认值: 1
		//
		FreqModLFO = 22,


		// vibLfo的延迟时长
		// 默认值: 0
		//
		DelayVibLFO = 23,


		// vibLfo的频率
		// 默认值: 1
		//
		FreqVibLFO = 24,


		// mod包络延迟时长(s)
		//
		DelayModEnv = 25,


		// mod包络起音时长(s)
		//
		AttackModEnv = 26,


		// mod包络保持时长(s)
		//
		HoldModEnv = 27,


		// mod包络衰减时长(s)
		//
		DecayModEnv = 28,


		// mod包络延音分贝降低倍率
		// sustainMul值域:[0, 1]
		// 可以从[0, 100]线性转换为[0,1]，mul/100 
		//
		SustainModEnv = 29,


		// mod包络释音时长(s)
		//
		ReleaseModEnv = 30,


		// mod包络的keyToHold
		//
		KeynumToModEnvHold = 31,


		// mod包络的keyToDecay
		//
		KeynumToModEnvDecay = 32,


		// 音量包络延迟时长(s)
		//
		DelayVolEnv = 33,


		// 音量包络起音时长(s)
		//
		AttackVolEnv = 34,


		// 音量包络保持时长(s)
		//
		HoldVolEnv = 35,


		// 音量包络衰减时长(s)
		//
		DecayVolEnv = 36,


		// 音量包络延音分贝降低倍率
		// 设置音量包络延音分贝降低倍率
		// sustainMul值域:[0, 1]
		// 可以从[0, 144]线性转换为[0,1]，dB/144 
		//
		SustainVolEnv = 37,


		// 音量包络释音时长(s)
		//
		ReleaseVolEnv = 38,


		// 音量包络的keyToHold
		//
		KeynumToVolEnvHold = 39,


		// 音量包络的keyToDecay
		//
		KeynumToVolEnvDecay = 40,


		Instrument = 41,


		// 发音音符范围
		// 0~127
		//
		KeyRange = 43,


		// 按键力度范围
		// 力度的大小影响音量的大小,超出力度影响范围的按键力度将不发音
		// 1为最大力度(音量最大)， 0为无力度(音量最小)
		//
		VelRange = 44,

		StartloopAddrsCoarseOffset = 45,


		// 固定按键音符
		// 将范围内的任何按键发音用指定的KeyNum的发音替换
		//
		Keynum = 46,


		// 固定按键力度
		// 将按键力度强制设置为给定的值，此时外界传入的力度值将会失效
		//
		Velocity = 47,


		// 采样音量衰减，非负值(单位:dB)
		// dB衰减转线性衰减倍率: volLinearAttenMul = Math.Pow(10, -volDbAtten / 20f);  
		// 0dB ~ -144dB 对应(1, 0)
		// 1为最大音， 0.5为衰减一半
		// 默认值: 1
		//
		InitialAttenuation = 48,
		EndloopAddrsCoarseOffset = 50,


		// 以半音为单位校正音调,影响样本的音调
		// 默认值: 0
		//
		CoarseTune = 51,


		// 以音分为单位校正音调，所调值将加到coarseTune上，继而一起影响样本的音调
		// 默认值: 0
		//
		FineTune = 52,

		SampleID = 53,


		// 循环播放模式
		// 默认值:0
		//
		SampleModes = 54,


		// 音阶调校，以overridingRootKey为参考音，即overridingRootKey处的音调不受scaleTuning影响，其它音将依比例改变，
		// 更改两个连续按键音符之间的音调差异程度， 100表示两个连续音符键间音调相差1个半音， 50表示每键之间相差为0.5个半音， 0表示两个连续音符间音调无差异
		// 最大值设为1200，表示每键之间相差12个半音
		// 默认值: 100
		//
		ScaleTuning = 56,



		// 独占发音归类
		// 在乐器层级上设置除0以外的值， 
		// 在同一个预设中的此项所有具有相同值乐器，互相之间不会同时发音，
		// 具有相同区域的排它设置值时，一个区域的发音被激活，则另一个区域的发音将会立即停止发音
		//
		ExclusiveClass = 57,


		// 修改原始采样发音对应琴键号
		// 如果原始采样音键号对应60，此时修改为69，那么将在69号琴键上发出60号琴键的声音
		// 此时声音样本整体上进行了9个半音的偏移
		//
		OverridingRootKey = 58,

		VolEnvToVolume = 59,

		//延音踏板开关
		SustainPedalOnOff = 60,

		EndOper = 61
	};


	// 乐器连接到PresetRegion信息
	struct InstLinkToPresetRegionInfo
	{
		Region* region = nullptr;
		Instrument* linkInst = nullptr;
	};

	// 样本连接到InstRegion信息
	struct SamplesLinkToInstRegionInfo
	{
		Region* region = nullptr;
		Sample* linkSample = nullptr;
	};

	struct RangeFloat
	{
		float min = 0;
		float max = 0;

		RangeFloat(float min = 0, float max = 0)
		{
			this->min = min;
			this->max = max;
		}
	};


	struct LfoModInfo
	{
		Lfo* lfo = nullptr;
		float modValue = 0;
		GeneratorType genType = GeneratorType::None;
		UnitTransformCallBack unitTransform = nullptr;

		LfoModInfo(Lfo* lfo, float modValue, GeneratorType genType, UnitTransformCallBack unitTransform = nullptr)
		{
			this->lfo = lfo;
			this->modValue = modValue;
			this->genType = genType;
			this->unitTransform = unitTransform;
		}
	};


	struct EnvModInfo
	{
		Envelope* env = nullptr;
		float modValue = 0;
		GeneratorType genType = GeneratorType::None;
		UnitTransformCallBack unitTransform = nullptr;

		EnvModInfo(Envelope* env, float modValue, GeneratorType genType, UnitTransformCallBack unitTransform = nullptr)
		{
			this->env = env;
			this->modValue = modValue;
			this->genType = genType;
			this->unitTransform = unitTransform;
		}
	};

	// 直线方程信息
	struct LineEquationInfo
	{
		// x轴范围
		float xmin, xmax;

		// 直线斜率
		float a;

		// y轴偏移
		float b;

	};


	struct RealtimeKeyEvent
	{
		bool isOnKey;
		int key;
		float velocity;
		VirInstrument* virInst;
		//实际时间点
		float timeSec;
		//采样时间点
		float sampleSec;
	};


	struct KeyEvent
	{
		bool isOnKey;
		int key;
		float velocity;
		bool isRealTime;
	};

}

#endif
