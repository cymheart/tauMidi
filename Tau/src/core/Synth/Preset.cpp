#include"Preset.h"
#include"Region.h"

namespace tau
{
	Preset::Preset()
	{
		globalRegion = new Region(RegionType::Preset);
		presetRegionLinkInfoList = new InstLinkToPresetRegionInfoList;
	}

	Preset::~Preset()
	{
		DEL(globalRegion);
		DEL(presetRegionLinkInfoList);
	}

	InstLinkToPresetRegionInfoList* Preset::GetPresetRegionLinkInfoList()
	{
		return presetRegionLinkInfoList;
	}

	// 连接一个乐器到一个presetRegion
	Region* Preset::LinkInstrument(Instrument* inst)
	{
		Region* presetRegion = new Region(RegionType::Preset);
		InstLinkToPresetRegionInfo linkInfo;
		linkInfo.region = presetRegion;
		linkInfo.linkInst = inst;
		presetRegionLinkInfoList->push_back(linkInfo);
		return presetRegion;
	}
}
