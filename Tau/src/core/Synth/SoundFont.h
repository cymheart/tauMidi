#ifndef _SoundFont_h_
#define _SoundFont_h_

#include "TauTypes.h"

namespace tau
{
	class DLL_CLASS SoundFont
	{
	public:
		SoundFont();
		~SoundFont();

		//根据格式类型,解析soundfont文件
		void Parse(string formatName, string path);

		// 增加一个样本到样本列表
		Sample* AddSample(string name, short* samples, size_t size, uint8_t* sm24 = nullptr);

		inline SampleList* GetSampleList()
		{
			return sampleList;
		}

		// 增加一个乐器到乐器列表
		Instrument* AddInstrument(string name);
		//获取乐器列表
		InstrumentList* GetInstrumentList()
		{
			return instList;
		}

		// 增加一个预设到预设列表
		Preset* AddPreset(string name, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//获取预设列表
		inline PresetList* GetPresetList()
		{
			return presetList;
		}

		// 样本绑定到乐器上
		Region* SampleBindToInstrument(Sample* sample, Instrument* inst);

		// 乐器绑定到预设上 
		Region* InstrumentBindToPreset(Instrument* inst, Preset* preset);

		// 获取乐器预设
		Preset* GetInstrumentPreset(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 获取乐器预设
		Preset* GetInstrumentPreset(int key);

	private:

		//增加所有soundfont解析器
		void AddParsers();

		//增加一个解析格式类型
		void AddParser(string formatName, SoundFontParser* sfParser);

	private:
		//
		SampleList* sampleList = nullptr;
		InstrumentList* instList = nullptr;
		PresetList* presetList = nullptr;
		PresetMap* presetBankDict = nullptr;

		//音源解析格式
		SoundFontParserMap* sfParserMap;
	};

}

#endif
