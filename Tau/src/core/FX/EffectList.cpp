#include"EffectList.h"

namespace tauFX
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
	void EffectList::AppendEffect(TauEffect* effect)
	{
		effects.push_back(effect);
	}

	void EffectList::Clear()
	{
		for (int i = 0; i < effects.size(); i++)
		{
			if (!effects[i]->IsEnable())
				continue;

			effects[i]->Clear();
		}
	}

	//效果处理
	void EffectList::Process()
	{
		for (int i = 0; i < effects.size(); i++)
		{
			if (!effects[i]->IsEnable())
				continue;

			if (leftChannelSamples == nullptr && rightChannelSamples == nullptr) {
				effects[i]->EffectProcess(synthStream, numChannels, channelSampleCount);
			}
			else {
				effects[i]->EffectProcess(leftChannelSamples, rightChannelSamples, channelSampleCount);
			}
		}
	}

}
