#include"EffectList.h"

namespace ventrue
{
	EffectList::EffectList()
	{

	}

	EffectList::~EffectList()
	{
		for (int i = 0; i < effects.size(); i++)
		{
			DEL(effects[i]);
		}
	}

	//添加效果器
	void EffectList::AppendEffect(VentrueEffect* effect)
	{
		effects.push_back(effect);
	}

	//效果处理
	void EffectList::Process()
	{
		for (int i = 0; i < effects.size(); i++)
		{
			if (!effects[i]->IsEnable())
				continue;

			effects[i]->EffectProcess(leftChannelSamples, rightChannelSamples, channelSampleCount);
		}
	}

}
