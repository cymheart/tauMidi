#ifndef _SF2Structs_h_
#define _SF2Structs_h_

#include"SF2Types.h"

namespace tau
{
	/// <summary>
	/// 预设
	/// </summary>
	class SF2PresetHeader
	{

	public:
		SF2PresetHeader() {};
		SF2PresetHeader(ByteStream& br);

	public:
		static uint32_t Size;
		byte presetName[20];
		uint16_t Preset = 0;          //预设编号
		uint16_t Bank = 0;            //预设音色库
		uint16_t PresetBagIndex = 0;  //指向预设关联的Bag列表起始索引
		uint32_t library = 0;         //音色版权标识码（32位）
		uint32_t genre = 0;           //乐器分类（如0 = 未分类，1 = 钢琴，2 = 弦乐）
		uint32_t morphology = 0;      //发音特征编码（如0 = 单音，1 = 复音）
	};


	/// <summary>
	/// 区域
	/// </summary>
	class SF2Bag
	{
	public:
		SF2Bag() {};
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
		SF2ModulatorList() {};
		SF2ModulatorList(ByteStream& br);

	public:
		static uint32_t Size;

		uint16_t ModulatorSource;           // 调制源（2字节枚举，如LFO、包络）
		SF2Generator ModulatorDestination;  // 调制目标（2字节枚举或链接其他调制器）
		SHORT ModulatorAmount;              // 调制量（有符号短整型，范围-32768~32767）
		uint16_t ModulatorAmountSource;     // 调制量的动态源（2字节枚举）
		SF2Transform ModulatorTransform;    // 调制转换操作（2字节枚举，如线性、指数）
	};


	/// <summary>
	/// 生成器
	/// </summary>
	class SF2GeneratorList
	{
	public:
		SF2GeneratorList() {};
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
		SF2Instrument() {};
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
		SF2SampleHeader() {};
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
		int8_t PitchCorrection;
		uint16_t SampleLink;
		SF2SampleLink SampleType = SF2SampleLink::MonoSample;

	};
}

#endif
