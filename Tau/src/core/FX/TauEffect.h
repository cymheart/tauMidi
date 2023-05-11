#ifndef _TauEffect_h_
#define _TauEffect_h_

#include<Synth/TauTypes.h>
#include "scutils/Utils.h"
#include "task/Task.h"
#include "Synth/Tau.h"


using namespace scutils;
using namespace tau;
using namespace task;


namespace tauFX
{
	class DLL_CLASS TauEffect
	{
	public:
		TauEffect()
		{
		}

		virtual ~TauEffect() { }

		virtual void Clear() {}

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

		void LockData();
		void UnLockData();

		void SetTau(Tau* tau) {
			this->tau = tau;
		}

		//
		virtual void EffectProcess(float* leftChannelSamples, float* rightChannelSamples, int channelSampleCount) {

		}

		virtual void EffectProcess(float* synthStream, int numChannels, int channelSampleCount) {

		}

	public:
		void SetEnableTask(bool isEnable);
	private:
		static void _SetEnableTask(Task* task);

	protected:
		//是否开启效果
		bool isEnable = true;
		Tau* tau = nullptr;
	};
}

#endif
