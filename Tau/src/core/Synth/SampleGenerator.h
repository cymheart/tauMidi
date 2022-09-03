#ifndef _SampleGenerator_h_
#define _SampleGenerator_h_

#include "TauTypes.h"

namespace tau
{
	//by cymheart, 2022.
	class RegionSampleGenerator
	{
	public:
		virtual ~RegionSampleGenerator() {}
		virtual void Init(int key) = 0;
		virtual void Trigger(float velocity) = 0;
		virtual float Out(int prevSampleIntPos, int nextSampleIntPos, float a) = 0;


	};

	class SampleGenerator
	{
	public:
		virtual ~SampleGenerator() {}
		virtual RegionSampleGenerator* CreateRegionSampleGen(int key) = 0;
		virtual void ReleaseRegionSampleGen(RegionSampleGenerator* sampleGen) = 0;
		virtual float GetSampleRate() = 0;
	};
}

#endif
