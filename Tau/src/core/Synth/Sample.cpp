#include"Sample.h"

namespace tau
{
	Sample::~Sample()
	{
		free(pcm);
	}

	// 设置样本
	void Sample::SetSamples(short* samples, uint32_t size, uint8_t* sm24)
	{
		this->size = size;
		pcm = (float*)malloc(size * sizeof(float));

		//是否为24位样本值
		if (sm24 == nullptr)
		{
			for (uint32_t i = 0; i < size; i++)
				pcm[i] = samples[i] / 32767.0f * 0.9f;
		}
		else
		{
			for (uint32_t i = 0; i < size; i++)
			{
				pcm[i] = (samples[i] << 8 | sm24[i]) / 8388607.0f * 0.9f;
			}
		}
	}
}
