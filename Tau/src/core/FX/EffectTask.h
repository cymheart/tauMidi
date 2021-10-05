#ifndef _EffectTask_h_
#define _EffectTask_h_

#include"TauEffect.h"
#include"task/Task.h"
using namespace task;


namespace tauFX
{
	class EffectTask : public Task
	{
	public:
		EffectTask(TauEffect* effect)
		{
			this->effect = effect;
		}

		TauEffect* effect;
		float valuef[10] = { 0 };
		int valuei[10] = { 0 };
		bool valuebl[10] = { false };
	};
}

#endif
