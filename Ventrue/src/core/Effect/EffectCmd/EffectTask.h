#ifndef _EffectTask_h_
#define _EffectTask_h_

#include"task/Task.h"
#include"Effect/VentrueEffect.h"
using namespace task;

namespace ventrue
{
	class EffectTask : public Task
	{
	public:
		EffectTask(VentrueEffect* effect)
		{
			this->effect = effect;
		}

		VentrueEffect* effect;
		float valuef[10] = { 0 };
		int valuei[10] = { 0 };
		bool valuebl[10] = { false };
	};
}

#endif
