#ifndef _TauEffect_h_
#define _TauEffect_h_

#include<Synth/TauTypes.h>
#include "scutils/Utils.h"
#include "task/Task.h"
using namespace scutils;
using namespace tau;
using namespace task;


namespace tauFX
{
	class DLL_CLASS TauEffect
	{
	public:
		TauEffect(Synther* synther = nullptr)
		{
			this->synther = synther;
		}

		virtual ~TauEffect() { }


		virtual void SetSynther(Synther* synther)
		{
			this->synther = synther;
		}

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

	public:
		void SetEnableTask(bool isEnable);
	private:
		static void _SetEnableTask(Task* task);

	protected:
		//是否开启效果
		bool isEnable = true;
		Synther* synther = nullptr;
	};
}

#endif
