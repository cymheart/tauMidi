#include"SoundFont.h"
#include"Instrument.h"
#include"Preset.h"
#include"SoundFontParser.h"
#include"SoundFontFormat/SF2/SF2Parser.h"
#include"SoundFontFormat/TauFont/TauFont.h"
#include"PhysicsPiano.h"

namespace tau
{
	SoundFont::SoundFont()
	{
		sampleList = new SampleList;
		sampleGenList = new SampleGenList;
		instList = new InstrumentList;
		presetList = new PresetList;
		presetBankDict = new PresetMap;
		sfParserMap = new SoundFontParserMap();

		//
		AddParsers();
	}

	SoundFont::~SoundFont()
	{
		DEL_OBJS_VECTOR(sampleList);
		DEL_OBJS_VECTOR(sampleGenList);
		DEL_OBJS_VECTOR(instList);
		DEL_OBJS_VECTOR(presetList);
		DEL(presetBankDict);

		//
		SoundFontParserMap::iterator it = sfParserMap->begin();
		for (; it != sfParserMap->end(); it++)
			DEL(it->second);
		DEL(sfParserMap);
	}

	//增加所有soundfont解析器
	void SoundFont::AddParsers()
	{
		//
		AddParser("TauFont", new TauFont(this));

		//
		SF2Parser* sf2Parser = new SF2Parser(this);
		sf2Parser->SetParseModulator(false);
		AddParser("SF2", sf2Parser);
	}

	//增加一个解析格式类型
	void SoundFont::AddParser(string formatName, SoundFontParser* sfParser)
	{
		(*sfParserMap)[formatName] = sfParser;
	}

	//根据格式类型,解析soundfont文件
	void SoundFont::Parse(string formatName, string path)
	{
		SoundFontParser* sfParser = nullptr;
		auto it = sfParserMap->find(formatName);
		if (it != sfParserMap->end())
			sfParser = it->second;

		if (sfParser)
			sfParser->Parse(path);
	}

	//开启物理钢琴
	void SoundFont::EnablePhysicsPiano(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (isEnablePhysicsPiano)
			return;

		isEnablePhysicsPiano = true;
		PhysicsPiano* physicsPiano = new PhysicsPiano();
		physicsPiano->Create(this, bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	// 增加一个样本到样本列表
	Sample* SoundFont::AddSample(string name, short* samples, size_t size, uint8_t* sm24)
	{
		Sample* sample = new Sample();
		sample->name = name;
		sample->SetSamples(samples, (uint32_t)size, sm24);
		sampleList->push_back(sample);
		return sample;
	}

	// 增加一个样本生成器到样本列表
	void SoundFont::AddSampleGen(SampleGenerator* sampleGen)
	{
		sampleGenList->push_back(sampleGen);
	}

	// 增加一个乐器到乐器列表
	Instrument* SoundFont::AddInstrument(string name)
	{
		Instrument* inst = new Instrument();
		inst->name = name;
		instList->push_back(inst);
		return inst;
	}

	// 增加一个预设到预设列表
	Preset* SoundFont::AddPreset(string name, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		Preset* preset = new Preset();
		preset->name = name;
		preset->SetBankNum(bankSelectMSB, bankSelectLSB, instrumentNum);
		presetList->push_back(preset);
		(*presetBankDict)[preset->GetBankKey()] = preset;
		return preset;
	}

	// 乐器绑定到预设上
	Region* SoundFont::InstrumentBindToPreset(Instrument* inst, Preset* preset)
	{
		return preset->LinkInstrument(inst);
	}

	// 样本绑定到乐器上
	Region* SoundFont::SampleBindToInstrument(Sample* sample, Instrument* inst)
	{
		return inst->LinkSamples(sample);
	}

	// 样本发生器绑定到乐器上
	Region* SoundFont::SampleGenBindToInstrument(SampleGenerator* sampleGen, Instrument* inst)
	{
		return inst->LinkSamplesGen(sampleGen);
	}

	// 获取乐器预设
	Preset* SoundFont::GetInstrumentPreset(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		int key = bankSelectMSB << 16 | bankSelectLSB << 8 | instrumentNum;
		return GetInstrumentPreset(key);
	}

	// 获取乐器预设
	Preset* SoundFont::GetInstrumentPreset(int key)
	{
		auto it = presetBankDict->find(key);
		if (it != presetBankDict->end()) {
			//cout << "使用乐器:" << instrumentNum << " " << it->second->name << endl;
			return it->second;
		}

		//cout << "未找到对应乐器!" << endl;

		//
		key &= 0xff00ff;  //bankSelectMSB << 16 | instrumentNum;
		it = presetBankDict->find(key);
		if (it != presetBankDict->end()) {
			//cout << "使用乐器:" << instrumentNum << " " << it->second->name << endl;
			return it->second;
		}

		return nullptr;
	}
}
