#include"Preset.h"
#include"Zone.h"

namespace tau
{
	Preset::Preset()
	{
		globalZone = new Zone(ZoneType::Preset);
	}

	Preset::~Preset()
	{
		DEL(globalZone);
		presetZoneLinkInfos.clear();
	}

	// 连接一个乐器到一个presetZone
	Zone* Preset::LinkInstrument(Instrument* inst)
	{
		Zone* presetZone = new Zone(ZoneType::Preset);
		InstLinkToPresetZoneInfo linkInfo;
		linkInfo.Zone = presetZone;
		linkInfo.linkInst = inst;
		presetZoneLinkInfos.push_back(linkInfo);
		return presetZone;
	}
}
