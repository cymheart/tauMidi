#include"SF2Structs.h"

namespace tau
{
	uint32_t SF2PresetHeader::Size = 38;
	SF2PresetHeader::SF2PresetHeader(ByteStream& br)
	{
		br.read(presetName, 0, 20);
		Preset = br.read<uint16_t>();
		Bank = br.read<uint16_t>();
		PresetBagIndex = br.read<uint16_t>();
		library = br.read<uint32_t>();
		genre = br.read<uint32_t>();
		morphology = br.read<uint32_t>();
	}

	uint32_t SF2Bag::Size = 4;
	SF2Bag::SF2Bag(ByteStream& br)
	{
		GeneratorIndex = br.read<uint16_t>();
		ModulatorIndex = br.read<uint16_t>();
	}

	uint32_t SF2ModulatorList::Size = 10;
	SF2ModulatorList::SF2ModulatorList(ByteStream& br)
	{
		ModulatorSource = br.read<uint16_t>();
		ModulatorDestination = (SF2Generator)br.read<uint16_t>();
		ModulatorAmount = br.read<uint16_t>();
		ModulatorAmountSource = br.read<uint16_t>();
		ModulatorTransform = (SF2Transform)br.read<uint16_t>();
	}

	uint32_t SF2GeneratorList::Size = 4;
	SF2GeneratorList::SF2GeneratorList(ByteStream& br)
	{
		Generator = (SF2Generator)br.read<uint16_t>();
		GeneratorAmount.UAmount = br.read<uint16_t>();
	}

	uint32_t SF2Instrument::Size = 22;
	SF2Instrument::SF2Instrument(ByteStream& br)
	{
		br.read(instrumentName, 0, 20);
		InstrumentBagIndex = br.read<uint16_t>();
	}

	uint32_t SF2SampleHeader::Size = 46;
	SF2SampleHeader::SF2SampleHeader(ByteStream& br)
	{
		br.read(sampleName, 0, 20);
		Start = br.read<uint32_t>();
		End = br.read<uint32_t>();
		LoopStart = br.read<uint32_t>();
		LoopEnd = br.read<uint32_t>();
		SampleRate = br.read<uint32_t>();
		OriginalKey = br.read<byte>();
		PitchCorrection = br.read<int8_t>();
		SampleLink = br.read<uint16_t>();
		SampleType = (SF2SampleLink)br.read<uint16_t>();
	}
}
