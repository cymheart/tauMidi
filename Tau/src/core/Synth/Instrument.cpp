#include"Instrument.h"
namespace tau
{
	Instrument::Instrument()
	{
		globalRegion = new Region(RegionType::Instrument);
		instRegionLinkInfoList = new vector<SamplesLinkToInstRegionInfo>;
	}

	Instrument::~Instrument()
	{
		DEL(globalRegion);
	}

	// 连接一个样本到一个instRegion
	Region* Instrument::LinkSamples(Sample* sample)
	{
		Region* instRegion = new Region(RegionType::Instrument);
		SamplesLinkToInstRegionInfo linkInfo;
		linkInfo.region = instRegion;
		linkInfo.linkSampleGen = nullptr;
		linkInfo.linkSample = sample;
		instRegionLinkInfoList->push_back(linkInfo);
		return instRegion;
	}

	// 连接一个样本发生器到一个instRegion
	Region* Instrument::LinkSamplesGen(SampleGenerator* sampleGen)
	{
		Region* instRegion = new Region(RegionType::Instrument);
		SamplesLinkToInstRegionInfo linkInfo;
		linkInfo.region = instRegion;
		linkInfo.linkSample = nullptr;
		linkInfo.linkSampleGen = sampleGen;
		instRegionLinkInfoList->push_back(linkInfo);
		return instRegion;
	}

	// 获取KeyNum在指定范围内的乐器区域组
	int Instrument::GetHavKeyInstRegionLinkInfos(int keyNum, float velocity, SamplesLinkToInstRegionInfo* activeInstRegionLinkInfos)
	{
		RangeFloat keyRange;
		RangeFloat velRange;
		int pos = 0;
		for (int i = 0; i < instRegionLinkInfoList->size(); i++)
		{
			keyRange = (*instRegionLinkInfoList)[i].region->GetKeyRange();
			velRange = (*instRegionLinkInfoList)[i].region->GetVelRange();

			if (keyNum >= keyRange.min && keyNum <= keyRange.max &&
				velocity >= velRange.min && velocity <= velRange.max)
			{
				activeInstRegionLinkInfos[pos++] = (*instRegionLinkInfoList)[i];
			}
		}

		SamplesLinkToInstRegionInfo info;
		info.linkSample = nullptr;
		info.region = nullptr;
		activeInstRegionLinkInfos[pos] = info;

		return pos;

	}
}
