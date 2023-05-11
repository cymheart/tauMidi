#ifndef _Preset_h_
#define _Preset_h_

#include "TauTypes.h"


namespace tau
{
	class Preset
	{
	public:
		Preset();
		~Preset();

		Zone* GetGlobalZone()
		{
			return globalZone;
		}

		inline void SetBankNum(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
		{
			this->bankSelectMSB = bankSelectMSB;
			this->bankSelectLSB = bankSelectLSB;
			this->instrumentNum = instrumentNum;
		}

		inline int GetBankKey()
		{
			return (bankSelectMSB << 16 | bankSelectLSB << 8 | instrumentNum);
		}

		vector<InstLinkToPresetZoneInfo>& GetPresetZoneLinkInfos()
		{
			return presetZoneLinkInfos;
		}


		// 连接一个乐器到一个presetZone
		Zone* LinkInstrument(Instrument* inst);

	public:
		string name;
		int bankSelectMSB = 0;
		int bankSelectLSB = 0;
		int instrumentNum = 0;

		Zone* globalZone;
		vector<InstLinkToPresetZoneInfo> presetZoneLinkInfos;
	};
}

#endif
