#ifndef _SF2Structs_h_
#define _SF2Structs_h_

#include"SF2Types.h"

namespace ventrue
{
    /// <summary>
    /// 预设
    /// </summary>
    class SF2PresetHeader
    {

    public:
        SF2PresetHeader(ByteStream& br);

    public:
        static uint32_t Size;
        byte presetName[20];
        uint16_t Preset = 0, Bank = 0, PresetBagIndex = 0;
        uint32_t library = 0, genre = 0, morphology = 0;
    };


    /// <summary>
    /// 区域
    /// </summary>
    class SF2Bag
    {
    public:
        SF2Bag(ByteStream& br);
    public:
        static uint32_t Size;
        uint16_t GeneratorIndex = 0; // 生成器列表标号
        uint16_t ModulatorIndex = 0; // 调制器列表标号
    };


    /// <summary>
    /// 调制器
    /// </summary>
    class SF2ModulatorList
    {
    public:
        SF2ModulatorList(ByteStream& br);

    public:
        static uint32_t Size;

        uint16_t ModulatorSource;
        SF2Generator ModulatorDestination;
        uint16_t ModulatorAmount;
        uint16_t ModulatorAmountSource;
        SF2Transform ModulatorTransform;     
    };


    /// <summary>
    /// 生成器
    /// </summary>
    class SF2GeneratorList
    {
    public:
        SF2GeneratorList(ByteStream& br);
    public:
        static uint32_t Size;

        //生成器类型
        SF2Generator Generator;
        //生成器数值
        SF2GeneratorAmount GeneratorAmount;
    };

    /// <summary>
    /// 乐器
    /// </summary>
    class SF2Instrument
    {
    public:
        SF2Instrument(ByteStream& br);
    public:
        static uint32_t Size;

        //乐器名称
        byte instrumentName[20];

        //关联到乐器在区域中的起始标号
        uint16_t InstrumentBagIndex;
    };

    /// <summary>
    /// 样本
    /// </summary>
    class SF2SampleHeader
    {
     
    public:
        SF2SampleHeader(ByteStream& br);

    public:
        static uint32_t Size;
        byte sampleName[20];
        uint32_t Start;
        uint32_t End;
        uint32_t LoopStart;
        uint32_t LoopEnd;
        uint32_t SampleRate;
        byte OriginalKey;
        char PitchCorrection;
        uint16_t SampleLink;
        SF2SampleLink SampleType = SF2SampleLink::MonoSample;

    };
}

#endif