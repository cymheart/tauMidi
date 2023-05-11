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

		virtual ~SF2Parser();

		//设置是否解析调制器
		void SetParseModulator(bool isParse)
		{
			isParseModulator = isParse;
			if (!isParseModulator) {
				printf("关闭解析sf调制器!");
			}
		}

		//解析
		void Parse(string filePath);


	protected:
		void ParseCore();

		//解析样本列表    
		void ParseSampleList();

		//解析乐器列表    
		void ParseInstrumentList();
		//解析乐器区域的生成器列表
		Zone* ParseInstZoneGeneratorList(int bagIdx, Instrument* tauInst);
		//解析乐器区域的调制器列表
		void ParseInstZoneModulatorList(Zone* zone, int bagIdx, Instrument* tauInst);

		//解析预设列表
		void ParsePresetList();
		//解析预设区域的生成器列表
		Zone* ParsePresetZoneGeneratorList(int bagIdx, Preset* tauPreset);
		//解析预设区域的调制器列表
		void ParsePresetZoneModulatorList(Zone* zone, int bagIdx, Preset* tauPreset);
		//生成调制器
		void CreateModulator(vector<SF2ModulatorList*>& mods, int start, int idx, Zone* zone);
		//重设输出目标调制器
		void ResetOutTargetModulator(vector<SF2ModulatorList*>& mods, int start, int idx);
		//获取区域中相同调制器
		Modulator* GetZoneSameModulator(Modulator* modulator, Zone* zone);

	protected:
		bool isParseModulator = true;
		SF2* sf2 = nullptr;
		Modulator** modulators = nullptr;
		size_t modulatorBufSize = 0;
		int modulatorCount = 0;
	};
}

#endif
