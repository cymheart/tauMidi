#ifndef _VentrueEffect_h_
#define _VentrueEffect_h_

#include <stk\Effect.h>
#include "scutils/Utils.h"
using namespace scutils;

namespace ventrue
{
	class DLL_CLASS VentrueEffect
	{
	public:

		//设置是否开启效果
		void SetEnable(bool isEnable)
		{
			this->isEnable = isEnable;
		}

		//是否开启效果
		bool IsEnable()
		{
			return isEnable;
		}

		virtual void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount) = 0;

	protected:
		//是否开启效果
		bool isEnable = true;
	};
}

#endif
