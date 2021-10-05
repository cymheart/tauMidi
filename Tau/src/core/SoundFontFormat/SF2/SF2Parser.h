#ifndef _SF2Parser_h_
#define _SF2Parser_h_

#include"SF2.h"
#include"Synth/TauTypes.h"
#include"Synth/SoundFontParser.h"

namespace tau
{
	class SF2Parser :public SoundFontParser
	{
	public:
		SF2Parser(SoundFont* sf)
			:SoundFontParser(sf)
		{
		}

		//设置是否解析调制器
		void SetParseModulator(bool isParse)
		{
			isParseModulator = isParse;
		}

		//解析
		void Parse(string filePath);


	private:

		//解析样本列表    
		void ParseSampleList();

		//解析乐器列表    
		void ParseInstrumentList();
		//解析乐器区域的生成器列表
		Region* ParseInstRegionGeneratorList(int bagIdx, Instrument* oreInst);
		//解析乐器区域的调制器列表
		void ParseInstRegionModulatorList(Region* region, int bagIdx, Instrument* oreInst);

		//解析预设列表
		void ParsePresetList();
		//解析预设区域的生成器列表
		Region* ParsePresetRegionGeneratorList(int bagIdx, Preset* orePreset);
		//解析预设区域的调制器列表
		void ParsePresetRegionModulatorList(Region* region, int bagIdx, Preset* orePreset);

		//设置生成器列表值
		void SetGenList(GeneratorList& genList, vector<SF2GeneratorList*>& sf2Gens, int start, int end, RegionType regionType);
		//生成调制器
		void CreateModulator(vector<SF2ModulatorList*>& mods, int i, Region* region);
		//替换区域中前一个相似调制器
		void ReplaceRegionPrevSameModulator(Modulator* modulator, Region* region);

	private:
		bool isParseModulator = true;
		SF2* sf2 = nullptr;
		Modulator** modulators = nullptr;
		size_t modulatorCount = 0;
	};
}

#endif
