#ifndef _SF2Types_h_
#define _SF2Types_h_

#include "scutils/Utils.h"
#include "scutils/ByteStream.h"

using namespace scutils;

namespace ventrue
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
        SampleID = 53,
        SampleModes = 54,
        ScaleTuning = 56,
        ExclusiveClass = 57,
        OverridingRootKey = 58,
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

    struct SF2ModulatorSource
    {
        uint16_t type : 6;
        uint16_t polarities :1;
        uint16_t direction : 1;
        uint16_t midiControllerFlag:1;
        uint16_t index:7;
    };
}

#endif