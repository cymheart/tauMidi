#ifndef _SoundFont_h_
#define _SoundFont_h_

#include "TauTypes.h"
#include"SampleGenerator.h"


namespace tau
{
	class DLL_CLASS SoundFont
	{
	public:
		SoundFont();
		~SoundFont();

		//根据格式类型,解析soundfont文件
		void Parse(string formatName, string path);

		//开启物理钢琴
		void EnablePhysicsPiano(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 增加一个样本到样本列表
		Sample* AddSample(string name, short* samples, size_t size, uint8_t* sm24 = nullptr);

		inline vector<Sample*>* GetSampleList()
		{
			return sampleList;
		}

		// 增加一个样本生成器到样本列表
		void AddSampleGen(SampleGenerator* sampleGen);

		// 增加一个乐器到乐器列表
		Instrument* AddInstrument(string name);
		//获取乐器列表
		vector<Instrument*>* GetInstrumentList()
		{
			return instList;
		}

		// 增加一个预设到预设列表
		Preset* AddPreset(string name, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//获取预设列表
		inline vector<Preset*>* GetPresetList()
		{
			return presets;
		}

		// 样本绑定到乐器上
		Zone* SampleBindToInstrument(Sample* sample, Instrument* inst);

		// 样本发生器绑定到乐器上
		Zone* SampleGenBindToInstrument(SampleGenerator* sampleGen, Instrument* inst);

		// 乐器绑定到预设上 
		Zone* InstrumentBindToPreset(Instrument* inst, Preset* preset);

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
		vector<Sample*>* sampleList = nullptr;
		vector<SampleGenerator*>* sampleGenList = nullptr;
		vector<Instrument*>* instList = nullptr;
		vector<Preset*>* presets;
		PresetMap* presetBankDict = nullptr;

		//是否开启物理钢琴
		bool isEnablePhysicsPiano = false;

	
		//音源解析格式
		SoundFontParserMap* sfParserMap;
	};

}

#endif
