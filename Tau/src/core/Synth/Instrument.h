#ifndef _Instrument_h_
#define _Instrument_h_

#include "TauTypes.h"
#include "Sample.h"
#include "Zone.h"
#include <Synth/SampleGenerator.h>

namespace tau
{
	class Instrument
	{
	public:
		Instrument();
		~Instrument();

		Zone* GetGlobalZone()
		{
			return globalZone;
		}

		//连接一个样本到一个新的乐器区域
		//一般sf文件都通过这个来关联样本和乐器区域
		Zone* LinkSamples(Sample* sample);

		//连接一个样本生成器到一个新的乐器区域
		//这个接口可以用来关联一些动态生成的样本(比如物理钢琴算法动态生成的样本)和乐器区域绑定
		Zone* LinkSamplesGen(SampleGenerator* sampleGen);


		vector<SamplesLinkToInstZoneInfo>& GetInstZoneLinkInfos()
		{
			return instZoneLinkInfos;
		}



	public:
		string name;

	private:

		Zone* globalZone;
		vector<SamplesLinkToInstZoneInfo> instZoneLinkInfos;

	};
}

#endif
