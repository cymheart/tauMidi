#include"Instrument.h"
namespace tau
{
	Instrument::Instrument()
	{
		globalZone = new Zone(ZoneType::Instrument);
	}

	Instrument::~Instrument()
	{
		DEL(globalZone);
		instZoneLinkInfos.clear();
	}

	//连接一个样本到一个新的乐器区域
	//一般sf文件都通过这个来关联样本和乐器区域
	Zone* Instrument::LinkSamples(Sample* sample)
	{
		Zone* instZone = new Zone(ZoneType::Instrument);
		SamplesLinkToInstZoneInfo linkInfo;
		linkInfo.Zone = instZone;
		linkInfo.linkSampleGen = nullptr;
		linkInfo.linkSample = sample;
		instZoneLinkInfos.push_back(linkInfo);
		return instZone;
	}

	//连接一个样本生成器到一个新的乐器区域
	//这个接口可以用来关联一些动态生成的样本(比如物理钢琴算法动态生成的样本)和乐器区域绑定
	Zone* Instrument::LinkSamplesGen(SampleGenerator* sampleGen)
	{
		Zone* instZone = new Zone(ZoneType::Instrument);
		SamplesLinkToInstZoneInfo linkInfo;
		linkInfo.Zone = instZone;
		linkInfo.linkSample = nullptr;
		linkInfo.linkSampleGen = sampleGen;
		instZoneLinkInfos.push_back(linkInfo);
		return instZone;
	}

}
