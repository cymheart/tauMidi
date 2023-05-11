#ifndef _SampleGenerator_h_
#define _SampleGenerator_h_

#include "TauTypes.h"

namespace tau
{
	//区域样本生成器
	//本来区域样本是通过连接到采样数据生成的，
	//而这个接口可以自定义乐器区域通过区域样本生成器动态生成
	//by cymheart, 2022.
	class ZoneSampleGenerator
	{
	public:
		virtual ~ZoneSampleGenerator() {}
		virtual void Init(int key) = 0;
		virtual void Trigger(float velocity) = 0;
		virtual float Out(int prevSampleIntPos, int nextSampleIntPos, float a) = 0;


	};

	//样本生成器,管理生成所有区域动态样本生成器
	class SampleGenerator
	{
	public:
		virtual ~SampleGenerator() {}
		virtual ZoneSampleGenerator* CreateZoneSampleGen(int key) = 0;
		virtual void ReleaseZoneSampleGen(ZoneSampleGenerator* sampleGen) = 0;
		virtual float GetSampleRate() = 0;
	};
}

#endif
