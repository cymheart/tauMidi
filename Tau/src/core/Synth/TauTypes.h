#ifndef _TauTypes_h_
#define _TauTypes_h_

#include "scutils/Utils.h"
#include "scutils/MathUtils.h"
#include "scutils/LinkedList.h"
#include <queue>

using namespace scutils;

#define TauLock(tau) lock_guard<mutex> lock(tau->lockMutex);

namespace tau
{

	enum class GeneratorType :int;
	enum class ModInputPreset :int;
	enum class EditorState;
	enum class CacheState;

	class Tau;
	class Editor;
	class MeasureInfo;
	class Synther;
	class SoundFont;
	class NoteOnEvent;
	class Lfo;
	class Preset;
	class Zone;
	class Instrument;
	class ModInputInfo;
	class Modulator;
	class Sample;
	class SampleGenerator;
	class ZoneSampleGenerator;
	class ZoneSounder;
	class InsideModulators;
	class MidiTrackRecord;
	class MeiExporter;

	class Envelope;
	class KeySounder;
	class MidiEditor;
	class Channel;
	class Track;
	class InstFragment;
	class MidiFile;
	class MidiEvent;
	class GeneratorList;
	class ZoneSounderThread;
	class ZoneSounderThreadData;
	class VirInstrument;
	class SoundFontParser;
	class SyntherEvent;
	class PhysicsPiano;

	struct RecordTempo;
	struct LfoModInfo;
	struct EnvModInfo;
	struct SamplesLinkToInstZoneInfo;
	struct InstLinkToPresetZoneInfo;
	struct LineEquationInfo;


	using SampleList = vector<Sample*>;
	using SampleGenList = vector<SampleGenerator*>;
	using InstrumentList = vector<Instrument*>;
	using LineEquationInfoList = vector <LineEquationInfo>;
	using ZoneSounderThreadDataList = vector <ZoneSounderThreadData*>;
	using MidiEditorList = vector <MidiEditor*>;
	using MidiFileList = vector <MidiFile*>;
	using GeneratorTypeList = vector <GeneratorType>;
	using VirInstList = vector <VirInstrument*>;
	using ModPresetTypeList = vector<ModInputPreset>;


	using ZoneSounderQueue = list<ZoneSounder*>;


	using PresetMap = unordered_map<uint32_t, Preset*>;
	using SoundFontParserMap = unordered_map<string, SoundFontParser*>;

	typedef uint64_t KeySounderID;

	using UnitTransformCallBack = float (*)(float value);
	using ModTransformCallBack = float (*)(float value);
	using VirInstStateChangedCallBack = void (*)(VirInstrument* virInst);
	using RenderEndCallBack = void (*)(Synther* synther);

#ifdef _WIN32
	template struct DLL_CLASS atomic<bool>;
	template struct DLL_CLASS atomic<double>;
	template struct DLL_CLASS atomic<EditorState>;
	template struct DLL_CLASS atomic<CacheState>;
#endif

#define A0 21
#define C1 24
#define C8 108
#define KeyWhite 1
#define KeyBlack 0

	//88键钢琴键盘定义
	extern int keyTypes88Std[];

	//获取note类型
	int GetNoteType(int note);

	//获取下一个黑色类型note
	int GetNextBlackNote(int note);

	//获取下一个白色类型note
	int GetNextWhiteNote(int note);


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


	// 类型
	enum class Type
	{
		MidiTrackType,
		DeviceType
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
	enum class ZoneType
	{
		//乐器
		Instrument,
		//预设
		Preset
	};


	// 循环播放模式
	enum class LoopPlayBackMode
	{
		// 无循环播放模式
		// 采样播放一次后停止
		NonLoop = 0,

		// 连续循环播放模式
		// 此时样本播放长度会以音量包络的总时长为准
		// 采样在循环起点和终点间无限重复播放（如弦乐、合成器持续音）
		Loop = 1,

		// 按键按下时循环播放，松开后播放剩余样本
		// 按住 MIDI 键时循环播放，松开后播放剩余部分（如钢琴衰减音）。
		LoopEndContinue = 3,

	};


	// 生成器类型
	enum class GeneratorType :int
	{
		None = -1,


		// 控制采样起始偏移的核心参数
		// 从采样头参数 Start 到实际播放起始点的偏移量（以 采样点数量 为单位）
		// 在采样数据点中，从起始采样头到该乐器要播放的第一个采样数据点的偏移。
		// 例如，如果Start为7，startAddrOffset为2，则播放的第一个采样数据点将为采样数据点9。
		// 默认值：0 → 无偏移，直接使用 Start 参数定义的起始点。
		// 范围：无理论限制，但受采样数据实际长度约束。
		// 单位：采样点（smpls），每个采样点对应音频数据的一个离散点。
		StartAddrsOffset = 0,


		// 结束地址偏移
		// 定义 样本数据的结束播放点偏移，即在样本头参数 End 的基础上，额外增加或减少的采样点数。
		// 例如，如果End为17，endAddrOffset为-2，则最后播放的采样数据点将为采样数据点15。
		// 默认值：0 → 无偏移，直接使用 End 指定的原始结束点。
		// 单位：样本数据点（Sample Data Points），即音频文件中每个采样点的索引。
		// 范围：无严格限制，但需确保偏移后不超出样本数据范围或导致负值。
		EndAddrsOffset = 1,


		// 循环起始地址偏移
		// 定义 样本循环起始点的偏移量，即在样本头参数 Startloop 的基础上，增加或减少的采样点数，以确定循环播放的起始位置
		// 例如，如果Startloop为10，startloopAddrsOffset为-1，则第一个重复循环采样数据点将为采样数据点9。
		//默认值：0 → 无偏移，直接使用 Startloop 指定的原始循环起始点。
		//单位：样本数据点（Sample Data Points），即音频文件中每个采样点的索引。
		//范围：无严格限制，但需确保偏移后不超出样本数据范围或导致循环逻辑错误（例如起始点晚于结束点）。
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


		// 调制LFO对音调的影响
		// 调制LFO的全幅振荡对音调的影响程度
		// 单位为 音分（Cents）
		// 默认值：0 → 无调制效果，音高不受LFO影响。
		// 范围：-12000（降低10个八度）至 12000（升高10个八度）。
		// 正负值逻辑：
		// 正值：LFO正向振荡（峰值）时升高音调，负向振荡（谷值）时降低音调。
		// 负值：LFO正向振荡时降低音调，负向振荡时升高音调。
		ModLfoToPitch = 5,


		// 颤音LFO对音调的影响
		// 用于控制颤音低频振荡器（Vibrato LFO）对音高的调制强度，颤音LFO全幅度偏移对音高的影响程度
		// 单位为 音分（Cents） 1 音分 = 1/100 半音，1 半音 = 100 音分，1 八度 = 1200 音分。
		// 正值：LFO的正向偏移（如波形上升）会升高音调。
		// 负值：LFO的正向偏移会降低音调。
		// 范围: -12000（-10 个八度）至 12000（+10 个八度）
		// 默认值：0 → 无音调调制效果。
		// LFO 速率（freqVibLfo）：
		// 慢速（15 Hz）适合自然颤音，快速（1020 Hz）适合机械 / 电子效果。
		// LFO 波形（WaveformVibLfo）：
		// 正弦波 → 平滑音高波动；方波 → 突变音高切换。
		VibLfoToPitch = 6,


		// 调制包络对音调的影响
		// 用于控制调制包络（Modulation Envelope）对音高的动态调制强度
		// 单位为 音分（Cents）
		// 范围:  -12000 至 +12000 音分（对应 -10 至 +10 个八度）。
		// 默认值：0 → 无音调调制效果。
		ModEnvToPitch = 7,


		// 截至频率(单位:cents(音分))
		// 默认值:13500 cents
		// 对应实际频率：20 kHz（人耳可听范围的极限，此时滤波器不生效）。
		// 存储格式：使用16位有符号整数（SHORT）表示，但实际有效范围为 1500~13500 cent。
		// 默认值13500:   8.176*2^(13500/1200) = 20000Hz
		// 最小值1500:  8.176*2^(1500/1200) = 20Hz
		InitialFilterFc = 8,


		//截止处的共振峰resonance peak(共振峰单位cB)
		//注意:InitialFilterQ在sf中是共振峰resonance而不是音频中的品质Q参数
		//默认值：存储值为0，对应实际cB值0（无共振）。
		//范围：0 ~ 960，对应（0dB ~96 dB)的共振峰值。
		//当InitialFilterQ = 0时，滤波器表现为平坦响应，无共振峰。
		//共振峰resonance处的频率的增益明显高于其他频率。resonance可在直流增益上以dB为单位测量
		//正值增加共振峰高度，负值理论上无效（但规范未明确禁止，可能被钳制到0）
		//此参数表示滤波器在截止频率处共振峰相对于直流增益（DC Gain）的高度，单位为百分贝（cB，1 cB = 0.1 dB）。
		//当值为零或负时，表示滤波器无共振；即使指定值为零，截止频率处的增益（极点角度）仍可能小于零。
		//此参数还会影响直流增益，使直流增益降低指定值的一半。
		//例如，若参数值为 100（对应 10 dB），则直流增益会比统一增益低 5 dB，而共振峰高度将比直流增益高 10 dB（即比统一增益高 5 dB）。
		//此外，若 InitialFilterQ 设为零或负且截止频率超过 20 kHz，则滤波器响应为平坦且增益为 1。
	    //换算: Q = Pow(10.0, ((resonanceDB - 3) / 20.0))
		//曲线平坦时，Q = 0.707, 即resonanceDB = 20*Log10(0.707) + 3, resonance = 0，即为截至频率的增益
		InitialFilterQ = 9,


		// 调制LFO对滤波截止频率的影响
		// 用于控制调制低频振荡器（Modulation LFO）对滤波器截止频率的动态调整幅度
		// 单位:音分（cents）
		// 范围:-12000（降低 10 个八度）至 +12000（升高 10 个八度）。
		// 默认值：0
		ModLfoToFilterFc = 10,


		// mod包络调制截至频率
		// 单位:音分（cents）
		// 范围:-12000（降低 10 个八度）至 +12000（升高 10 个八度）。
		// 默认值：0
		ModEnvToFilterFc = 11,



		// 以32768个采样数据点为单位的偏移量,在采样数据点中，超出Endloop采样头的偏移，将被认为等效于此乐器循环的Startloop样本数据点。
		// 例如，如果Endloop为15而endloopAddrsOffset为2，EndAddrsCoarseOffset = 2 ,则采样数据点 15 + 2 + 2*32768将被视为等同于Startloop采样数据点，
		// 因此采样数据点16将在循环期间有效地位于Startloop之前。
		//  15 + 2 + 2*32768
		EndAddrsCoarseOffset = 12,


		// 调制LFO对音量的影响
		// 用于控制调制低频振荡器（Modulation LFO）对音量的动态调制强度
		// 单位为 centibels
		// 范围：-960 至 +960 cB（对应 -96 dB 至 +96 dB）。
		// 默认值：0 → 无音量调制效果。
		// 调制LFO的周期性波形（如正弦波、三角波）会驱动音量波动，幅度由 ModLfoToVolume 决定。
		// 示例：
		// 若 ModLfoToVolume = 100 cB，LFO全幅度偏移时，音量将先上升 10 dB，随后下降 10 dB，形成周期性起伏。
		// 若设为 -300 cB，音量将在LFO正向偏移时降低 30 dB（适用于渐隐效果）。
		// LFO速率（FrequencyModLfo）：
		// 低频（0.1~5 Hz）：模拟自然音量波动（如弦乐颤音）。
		// 高频（5~20 Hz）：创造机械感或电子化效果（如颤振音效）。
		// LFO波形（WaveformModLfo）：
		// 正弦波：平滑音量过渡（常用）。
		// 方波：瞬时音量切换（特殊效果）。
		ModLfoToVolume = 13,


		// 合唱效果发送量
		// 用于控制音频信号发送至合唱效果处理器的强度
		// 范围: 0~1000（即 0%~100%）
		// 默认值： 0（无效果发送）
		ChorusEffectsSend = 15,


		// 混响效果发送量
		// 用于控制音频信号发送至混响效果处理器的强度
		// 范围：0~1000（0%~100%）
		// 默认：0
		ReverbEffectsSend = 16,


		// 声像定位,用于控制音频信号的左右声道分布比例
		// 默认值: 0 信号居中（左右声道均衡）
		// 范围：无硬性限制，但实际应用中通常为 -500 至 +500（-50%~+50%）。
		// 当pan = 0时,左右声道音量一样大都增益为0.5倍
		// 如果pan>=500, 那右声道增益为1倍，左声道增益为0倍
		// 如果pan 小于等于 -500，那左声道增益为1倍，右声道增益为0倍
		Pan = 17,


		// 调制LFO的延迟时间
		// 用于定义从触发音符（Key On）到调制低频振荡器（Modulation LFO）开始工作的延迟时间
		// 单位为 时间音分（timecents)
		// 范围为 -12000（约 1 毫秒）至 5000（约 20 秒）
		// 默认值: -12000
		DelayModLFO = 21,


		// 调制LFO的频率
		// 用于控制调制低频振荡器（Modulation LFO）的振荡频率，直接影响调制效果的快慢与周期性
		// 绝对音分（Absolute Cents）
		// 范围: -16000 至 4500（对应频率范围：1 mHz ~ 100 Hz）
        // 默认值: 0 Cents  
		// 0 Cents:  8.176 * 2^(0/1200) = 8.176 Hz
		FreqModLFO = 22,


		// 颤音LFO的延迟时间
		// 用于控制颤音低频振荡器（Vibrato LFO）在音符触发后的启动延迟时间
		// 定义从 音符触发（Key On）到颤音LFO开始启动 的时间间隔
		// 单位：绝对时间音分（Timecents）
		// 范围: -12000 至 5000（对应范围：1 ms ~ 20s）
		// 默认值: -12000 Timecents
		DelayVibLFO = 23,


		// 颤音LFO的频率
		// 用于控制颤音低频振荡器（Vibrato LFO）的三角波频率，直接影响颤音效果的周期性速度与波动幅度
		// 绝对音分（Absolute Cents）
		// 范围: -16000 至 4500（对应频率范围：1 mHz ~100 Hz）
		// 默认值: 0 Cents  
		// 0 Cents:  8.176 * 2^(0/1200) = 8.176 Hz
		FreqVibLFO = 24,


		// 调制包络（Modulation Envelope）中控制延迟阶段（Delay Phase）时间
		// 定义从 音符触发（Key On） 到调制包络 起音阶段（Attack Phase） 开始前的延迟时间
		// 单位：绝对时间厘（Absolute Timecents）
		// 最小值：-12000 → 对应约 1 毫秒, 最大值：5000 → 对应约 20 秒
		// 默认值：-12000 → 对应 1 毫秒
		DelayModEnv = 25,


		// 调制包络的起音时长（Modulation Envelope Attack Time）
		// 定义调制包络从延迟阶段（Delay）结束到达到峰值（Attack Peak）所需的时间，
		// 影响音色动态（如滤波器截止频率或音高的调制强度）
		// 单位：时间分（absolute timecents）
		// 范围：-12000（约1毫秒）~ 8000（约100秒）
		// 默认值：-12000 timecents（约 1 毫秒，即近乎瞬时起音）
		// -12000:  2^(-12000/1200) = 0.00096秒
		AttackModEnv = 26,


		// 调制包络保持时间
		// 定义调制包络在 起音阶段（Attack Phase）结束后、衰减阶段（Decay Phase）开始前 的保持时间
		// 在此期间，调制包络值维持在 峰值电平（Peak Level） 不变。
		// 单位：绝对时间厘（Absolute Timecents）
		// 最小值：-12000 → 对应约 1 毫秒,最大值：5000 → 对应约 20 秒。
		// 默认值：-12000 timecents（约 1 毫秒，即近乎瞬时起音）
		// -12000:  2^(-12000/1200) = 0.00096秒
		HoldModEnv = 27,


		// 调制包络衰减时间
		// 控制调制包络（Modulation Envelope） 在 衰减阶段（Decay Phase） 的持续时间，
		// 即从 峰值电平 线性衰减到 持续电平（Sustain Level） 所需的时间
		// 单位：绝对时间厘（Absolute Timecents）
		// 范围：-12000（约1毫秒）~ 8000（约100秒）
		// 默认值：-12000 timecents（约 1 毫秒，即近乎瞬时起音）
		// -12000:  2^(-12000/1200) = 0.00096秒
		DecayModEnv = 28,


		// 调制包络持续电平,调制包络延音分贝降低倍率
		// 设定调制包络在衰减阶段结束后需维持的 目标电平（以 峰值电平的百分比 表示）。
		// 范围：0（0% 衰减，全峰值）至 1000（100% 衰减，完全静音）。
		// 默认值：0 → 维持 100% 峰值电平（无衰减）。
		SustainModEnv = 29,


		// 调制包络的释放时间（Modulation Envelope Release Time）
		// 控制调制包络在释放阶段（Release Phase）从当前值衰减至零所需的时间，
		// ReleaseModEnv 仅在释放阶段生效，需与其他包络阶段（延迟、起音、衰减、持续）结合使用
		// 影响音色动态的消退速度（如滤波器或音高调制的平滑结束）
		// 单位：绝对时间分（absolute timecents）
		// 最小值：-12000,最大值：8000（约 100 秒）
		// 默认值：-12000 timecents
		// 若需立即结束调制效果（无释放时间），可设为极小值（如 -12000 → 约 1 毫秒）。
		// -12000:  2^(-12000/1200) = 0.00096秒
		ReleaseModEnv = 30,


		// 键位对调制包络保持时间的调节
		// 用于通过 MIDI 键号动态调整调制包络（Modulation Envelope）的保持时间（Hold Phase Time）
		// 单位：时间厘/键位（Timecents per KeyNumber）
		// 范围： -1200 至 1200 Timecents per KeyNumber
		// 默认值：0 → 保持时间不随键号变化，始终采用 HoldModEnv的设定值。
		// 修正后的HoldModEnv时间(timecents) = HoldModEnv + (KeynumToModEnvHold * (OnKeyNumber - 60))
		// 键号 60（中央 C） 的保持时间始终不变（由 HoldModEnv 直接控制）。
		// 设计意图：
		// 模拟真实乐器特性：例如钢琴高音区衰减更快，可设为正值；低音区衰减更慢，可设为负值。
		// 动态音色控制：结合 HoldVolEnv 和 KeynumToVolEnvDecay，实现音色随音高变化的复杂动态响应。
		KeynumToModEnvHold = 31,


		// 键位对调制包络衰减时间的调节
		// 通过 MIDI 键号动态调整调制包络（Modulation Envelope）的衰减阶段时间（Decay Phase Time）
		// 单位：时间厘/键位（Timecents per KeyNumber）
		// 范围：-1200 至 1200
		// 默认值：0 → 衰减时间不随键号变化，始终由 DecayModEnv（参数 28）独立控制。
		// 修正后的DecayModEnv时间(timecents) = DecayModEnv + (KeynumToModEnvDecay * (OnKeyNumber - 60))
		// 键号 60（中央 C） 的衰减时间由 DecayModEnv 直接设定，不受此参数影响。
		KeynumToModEnvDecay = 32,


		// 音量包络延迟时间
		// 定义从 音符触发（Key On） 到音量包络 起音阶段（Attack Phase） 开始前的延迟时间
		// 单位：绝对时间厘（Absolute Timecents）
		// 最小值：-12000 → 对应约 1 毫秒, 最大值：5000 → 对应约 20 秒。
		// 默认值：-12000 timecents
		// -12000:  2^(-12000/1200) = 0.00096秒
		DelayVolEnv = 33,


		// 音量包络起音时长(Volume Envelope Attack Time)
		// 控制音量包络从延迟阶段结束到达到峰值的时间。
		// 单位：绝对时间分（absolute timecents）
		// 范围：-12000 timecents（1 毫秒）至 8000 timecents（100 秒）
		// 默认值：-12000 timecents（对应约 1 毫秒 的起音时间）
		// -12000：  2^(-12000/1200) = 0.0009766秒
		AttackVolEnv = 34,


		// 音量包络的保持时间（Volume Envelope Hold Time）
		// 控制音量包络在起音阶段（Attack Phase）结束后、衰减阶段（Decay Phase）开始前的保持时间
		// 在此期间，音量维持在起音峰值（Attack Peak）不变化。
		// 单位：绝对时间分（absolute timecents）
		// 最小值：-12000（约 1 毫秒）,最大值：5000（约 20 秒）
		// 默认值：-12000 timecents（约 1 毫秒，几乎无保持阶段）
		// -12000：  2^(-12000/1200) = 0.0009766秒
		HoldVolEnv = 35,


		// 音量包络的衰减时间(Volume Envelope Decay Time）
		// 控制音量包络从起音阶段（Attack Peak）结束到进入持续电平（Sustain Level）所需的时间，
		// 影响音量的自然衰减速度
		// 单位：绝对时间分（absolute timecents）
		// 最小值：-12000（约 1 毫秒）最大值：8000（约 100 秒）
		// 默认值：-12000 timecents
		// 1秒： 2^(0/1200) = 1
		DecayVolEnv = 36,


		// 音量包络的持续电平控制参数（Volume Envelope Sustain Level）
		// 音量包络延音分贝降低倍率
		// 定义音量包络在衰减阶段（Decay Phase）结束后、释放阶段（Release Phase）开始前的持续电平，即音符持续期间的音量衰减幅度
		// 范围： 0 ~ 1440 centibels
		// 默认值：0 centibels（对应 0 dB 衰减，即持续电平等于起音峰值电平）。
		SustainVolEnv = 37,


		// 音量包络（Volume Envelope）的释放时间参数
		// 定义音量包络在释放阶段从当前电平（通常为 SustainVolEnv 设定的持续电平）衰减至完全静音所需的时间
		// 单位：绝对时间厘（absolute timecents）
		// 最小值：-12000 timecents（1 毫秒）至 8000 timecents（100 秒）
		// 默认值：-12000 timecents → 对应实际时间约 1 毫秒 
		ReleaseVolEnv = 38,


		// 音量包络的keyToHold
		// 用于控制 音量包络保持时间（HoldVolEnv） 随 MIDI 键号变化的动态调整逻辑
		// 定义 MIDI 键号（Key Number） 对音量包络保持时间（HoldVolEnv）的 调节强度
		// 单位是 时间厘/键号（timecents/key）
		// 范围：-1200（每键号减少 1 个八度时间）至 1200（每键号增加 1 个八度时间）。
		// 默认值：0 → 表示键号变化不影响保持时间。
		// 修正后的HoldVolEnv时间(timecents) = HoldVolEnv + (KeynumToVolEnvHold * (OnKeyNumber - 60))
		// 键号 60（中央 C）：保持时间不变。
		// 键号 > 60：保持时间随键号升高按比例缩短（若 KeynumToVolEnvHold 为正值）。
		// 键号 < 60：保持时间随键号降低按比例延长（若 KeynumToVolEnvHold 为正值）。
		//设计意图：
		//模拟真实乐器特性：例如钢琴高音区衰减更快,低音区衰减更慢。
		//动态音色控制：结合 HoldVolEnv 和 KeynumToVolEnvDecay，实现音色随音高变化的复杂动态响应。
		KeynumToVolEnvHold = 39,


		// 键位对音量包络衰减时间的调节
		// MIDI 键号升高时，音量包络衰减时间缩短的速率（以 时间厘/键位单位 为单位）
		// 单位：时间厘/键位（Timecents per KeyNumber），每单位键号变化对应的时间厘调整量
		// 范围：-1200 至 1200，负值表示键号升高时衰减时间延长，正值表示缩短。
		// 默认值：0 → 衰减时间不随键号变化，始终采用 DecayVolEnv（参数 36）的设定值。
		// 键号 60（中央 C） 的衰减时间始终不变（由 DecayVolEnv 直接控制）。
		// 修正后的DecayVolEnv时间(timecents) = DecayVolEnv + (KeynumToVolEnvDecay * (OnKeyNumber - 60))
		// 结合 SustainVolEnv（持续电平）和 ReleaseVolEnv（释放时间），可模拟钢琴、吉他等乐器的自然衰减特性。
		// 例如，高音区快速衰减至 50% 电平，低音区缓慢衰减至 20% 电平。
		KeynumToVolEnvDecay = 40,


		Instrument = 41,


		// 发音音符范围
		// 范围:0~127
		// 默认范围:0~127
		KeyRange = 43,


		// 按键力度范围
		// 力度的大小影响音量的大小,超出力度影响范围的按键力度将不发音
		// 范围:0~127
		// 默认范围:0~127
		//
		VelRange = 44,

		StartloopAddrsCoarseOffset = 45,


		// 固定按键音符
		// 将范围内的任何按键发音用指定的KeyNum的发音替换
		// 范围: 0~127
		// 默认值:-1
		Keynum = 46,


		// 固定按键力度
		// 将按键力度强制设置为给定的值，此时外界传入的力度值将会失效
		// 范围: 0~127
		// 默认值:-1
		Velocity = 47,


		// 采样音量衰减
		// 范围：0（无衰减）至 1440 cB （144 dB 衰减）
		// 默认值: 0(单位:cB)
		// dB衰减转线性衰减倍率: volLinearAttenMul = Math.Pow(10, -volDbAtten / 20f);  
		// 0dB ~ 144dB 对应(1, 0)
		InitialAttenuation = 48,

		//
		EndloopAddrsCoarseOffset = 50,


		// 以半音为单位校正音调,影响样本的音调(粗调音调)
		// 用于对音调进行大范围调整的生成器参数，直接影响采样音调的整体偏移
		// 以 半音（Semitone） 为单位，对乐器或预设的原始采样音高进行整体调整，偏移范围为 -24 至 +24 半音（即 ±2 个八度）
	    // 范围：-120（-10oct）至 120 semitone （10oct）
		// 默认值: 0
		CoarseTune = 51,


		// 以音分为单位校正音调，所调值将加到coarseTune上，继而一起影响样本的音调
		// 1 音分 = 1/100 半音，1 半音 = 100 音分，1 八度 = 1200 音分
		// 范围：-99 至 99 音分 
		// 默认值: 0
		FineTune = 52,

		// SampleID是Instrument Generator列表的终止生成器，
		// 只能在Instrument Generator列表子块中出现，并且必须在除全局区域外的所有区域中作为最后一个生成器出现
		// 通过该参数，乐器区域能够明确指定使用哪一个采样作为音源
		SampleID = 53,


		// 循环播放模式
		// SampleModes 是用于控制乐器区域（Instrument Zone）中采样循环模式的关键参数
		// 用于定义当前乐器区域关联采样的循环播放行为，仅出现在 IGEN 子块（Instrument Generator 列表）中
		// 可参考: LoopPlayBackMode
		// 默认值:0
		SampleModes = 54,


		//Reserved3 是规范中明确标注的保留字段，未分配实际功能。
		//若在 SoundFont 文件中遇到此参数，应直接忽略，其值不会对音色生成或播放产生任何影响。
		Reserved3 = 55,

		// 音阶调校，以overridingRootKey为参考音，即overridingRootKey处的音调不受scaleTuning影响，其它音将依比例改变，
		// 更改两个连续按键音符之间的音调差异程度， 100表示两个连续音符键间音调相差1个半音， 
		// 50表示每键之间相差为0.5个半音， 0表示两个连续音符间音调无差异
		// 最大值设为1200，表示每键之间相差12个半音,数值范围: 0 ~ 1200  cent/key（每键音分）
		// 默认值: 100 (单位: cent/key（每键音分）)
		// ScaleTuning = 100 → 每键升高 1 半音（标准十二平均律）
		// ScaleTuning = 200 → 每键升高 2 半音（全音阶）
		// ScaleTuning = 50 → 每键升高 0.5 半音（四分之一音阶）
		ScaleTuning = 56,

		// 独占发音归类
		// 在乐器层级上设置除0以外的值， 
		// 在同一个预设中的此项所有具有相同值乐器，互相之间不会同时发音，
		// 具有相同区域的排它设置值时，一个区域的发音被激活，则另一个区域的发音将会立即停止发音
		ExclusiveClass = 57,


		// 修改原始采样发音对应琴键号
		// 如果原始采样音键号对应60，此时修改为69，那么将在69号琴键上发出60号琴键的声音
		// 此时声音样本整体上进行了9个半音的偏移
		// 范围: 0~127
		// 默认值: -1
		OverridingRootKey = 58,

		//Dummy 是规范中明确标注的无效参数，仅用于占位或兼容性目的，无实际功能。
		//无论其值如何设置，合成器在处理时会完全忽略，不会影响音色生成逻辑
		Dummy = 59,

		//以下为自扩展参数
		// 
		// 
		//(扩展)声音包络调制
		VolEnvToVolume = 60,

		//(扩展)延音踏板开关
		//默认值：-1
		SustainPedalOnOff = 61,

		//(扩展)压力值
		//默认值：-1
		Pressure = 62,

		//
		EndOper = 63
	};



	// 乐器连接到PresetZone信息
	struct InstLinkToPresetZoneInfo
	{
		Zone* Zone = nullptr;
		Instrument* linkInst = nullptr;
	};

	// 样本连接到InstZone信息
	struct SamplesLinkToInstZoneInfo
	{
		Zone* Zone = nullptr;
		//静态样本
		Sample* linkSample = nullptr; 
		//动态样本(比如由物理钢琴算法动态生成的样本)
		SampleGenerator* linkSampleGen = nullptr; 

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

	//生成器数值
	union GeneratorAmount
	{
		struct {
			byte low;
			byte high;
		} rangeData;

		int16_t amount = 0;  //有符号16位整数
		uint16_t uAmount;    //无符号16位整数
	};

	//lfo调制信息
	struct LfoModInfo
	{
		Lfo* lfo = nullptr;  //lfo调制器
		float modValue = 0;  //调制量
		GeneratorType genType = GeneratorType::None;  //调制目标
		UnitTransformCallBack unitTransform = nullptr;  //单位变换

		LfoModInfo() {
		}

		/// <summary>
		/// lfo调制信息
		/// </summary>
		/// <param name="lfo">lfo</param>
		/// <param name="modValue">调制量</param>
		/// <param name="genType">调制目标</param>
		/// <param name="unitTransform">单位变换</param>
		LfoModInfo(Lfo* lfo, float modValue, GeneratorType genType, UnitTransformCallBack unitTransform = nullptr)
		{
			this->lfo = lfo;
			this->modValue = modValue;
			this->genType = genType;
			this->unitTransform = unitTransform;
		}
	};

	//包络调制信息
	struct EnvModInfo
	{
		Envelope* env = nullptr; //包络调制器
		float modValue = 0;      //调制量
		GeneratorType genType = GeneratorType::None;  //调制目标
		UnitTransformCallBack unitTransform = nullptr;  //单位变换

		/// <summary>
		/// 包络调制信息
		/// </summary>
		/// <param name="env">包络调制器</param>
		/// <param name="modValue">调制量</param>
		/// <param name="genType">调制目标</param>
		/// <param name="unitTransform">单位变换</param>
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
		float xmin = 0, xmax = 0;

		// 直线斜率
		float a = 0;

		// y轴偏移
		float b = 0;

	};

	struct KeyEvent
	{
		int id = 0;
		bool isOnKey = true;
		int key = 60;
		float velocity = 127;
		bool isRealTime = false;
	};

	//过渡效果深度信息
	struct FadeEffectDepthInfo
	{
		bool isFadeDepth = false;
		float startFadeSec = 0;

		//当前深度
		float curtDepth = 0;
		//目标深度
		float dstDepth = 0;
		//开始深度
		float startDepth = 0;
	};

	//值过渡信息 
	struct ValueFadeInfo
	{
		//目标值
		float dstValue = -1;
		//原始值
		float orgValue = -1;
		//总过渡值
		float totalFadeValue = 0;
		//开始过渡时间点
		float startFadeSec = 0;
		//总过渡时间
		float totalFadeTime = 0.05f;
		//过渡计算值
		float fadeComputedValue = 0;
	};

	//速度设置
	struct TempoSetting
	{
		//一个四分音符的微秒数
		float microTempo = 0;
		//一个四分音符的tick数
		float tickForQuarterNote = 0;
		//开始设置速度的tick数
		int startTickCount = 0;
	};
}

#endif
