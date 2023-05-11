#ifndef _SF2Types_h_
#define _SF2Types_h_

#include "scutils/Utils.h"
#include "scutils/ByteStream.h"

using namespace scutils;

namespace tau
{

	//生成器枚举值
	enum class SF2Generator
	{
		StartAddrsOffset = 0,
		EndAddrsOffset = 1,
		StartloopAddrsOffset = 2,
		EndloopAddrsOffset = 3,
		StartAddrsCoarseOffset = 4,
		ModLfoToPitch = 5,
		VibLfoToPitch = 6,
		ModEnvToPitch = 7,
		InitialFilterFc = 8,
		InitialFilterQ = 9,
		ModLfoToFilterFc = 10,
		ModEnvToFilterFc = 11,
		EndAddrsCoarseOffset = 12,
		ModLfoToVolume = 13,
		ChorusEffectsSend = 15,
		ReverbEffectsSend = 16,
		Pan = 17,
		DelayModLFO = 21,
		FreqModLFO = 22,
		DelayVibLFO = 23,
		FreqVibLFO = 24,
		DelayModEnv = 25,
		AttackModEnv = 26,
		HoldModEnv = 27,
		DecayModEnv = 28,
		SustainModEnv = 29,
		ReleaseModEnv = 30,
		KeynumToModEnvHold = 31,
		KeynumToModEnvDecay = 32,
		DelayVolEnv = 33,
		AttackVolEnv = 34,
		HoldVolEnv = 35,
		DecayVolEnv = 36,
		SustainVolEnv = 37,
		ReleaseVolEnv = 38,
		KeynumToVolEnvHold = 39,
		KeynumToVolEnvDecay = 40,
		Instrument = 41,
		KeyRange = 43,
		VelRange = 44,
		StartloopAddrsCoarseOffset = 45,
		Keynum = 46,
		Velocity = 47,
		InitialAttenuation = 48,
		EndloopAddrsCoarseOffset = 50,
		CoarseTune = 51,
		FineTune = 52,

		// SampleID是Instrument Generator列表的终止生成器，
		// 只能在Instrument Generator列表子块中出现，并且必须在除全局区域外的所有区域中作为最后一个生成器出现
		// 通过该参数，乐器区域能够明确指定使用哪一个采样作为音源
		SampleID = 53,

		SampleModes = 54,
		Reserved3 = 55,
		ScaleTuning = 56,
		ExclusiveClass = 57,
		OverridingRootKey = 58,
		Dummy = 59,
		EndOper = 60
	};

	enum class SF2SampleLink
	{
		MonoSample = 1,
		RightSample = 2,
		LeftSample = 4,
		LinkedSample = 8,
		RomMonoSample = 0x8001,
		RomRightSample = 0x8002,
		RomLeftSample = 0x8004,
		RomLinkedSample = 0x8008
	};

	enum class SF2Modulator
	{
		None = 0,
		NoteOnVelocity = 1,
		NoteOnKey = 2,
		PolyPressure = 10,
		ChannelPressure = 13,
		PitchWheel = 14,
		PitchWheelSensivity = 16
	};

	enum class SF2ModulatorSourceType
	{
		Linear = 0,
		Concave = 1,
		Convex = 2,
		Switch = 3
	};

	enum class SF2Transform
	{
		Linear = 0,
		Abs = 2
	};

	struct RangesType
	{
		byte byLo;
		byte byHi;
	};

	union SF2GeneratorAmount
	{
		RangesType ranges;
		int16_t Amount;
		uint16_t UAmount;
	};

	struct SF2VersionTag
	{
		uint32_t Size = 4;
		uint16_t Major;
		uint16_t Minor;
	};

	//SFModulatorSource 的 16 位值按功能划分为以下字段（从高位到低位）
	struct SF2ModulatorSource
	{
		uint16_t type : 6;                   //枚举字段的 高位 10-15（共 6 位） 被定义为控制器类型（Type），但 当前仅支持一种源类型（线性或凹型）
		uint16_t polarities : 1;             //P（极性） 信号极性： 0（单极）：值范围 0 到 1（如 MIDI 颤音轮）。1（双极）：值范围 - 1 到 1（如 MIDI 弯音轮）。
		uint16_t direction : 1;              //D（方向） 信号方向：0 表示增加，1表示减少
		uint16_t midiControllerFlag : 1;     //CC标志  1表示信号源为 MIDI 连续控制器（CC），否则为其他类型（如 LFO、触后等）。
		uint16_t index : 7;                  //Index（索引） 定义具体信号源（如 CC 编号、触后类型等）。
	};
}

#endif
