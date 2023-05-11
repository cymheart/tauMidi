#include"Sample.h"

namespace tau
{
	Sample::~Sample()
	{
		if (pcm != nullptr) {
			free(pcm);
			pcm = nullptr;
		}

		if (sm24 != nullptr) {
			free(sm24);
			sm24 = nullptr;
		}
	}


	// 设置样本
	void Sample::SetSamples(short* samples, uint32_t size, uint8_t* sm24)
	{
		if (pcm != nullptr) {
			free(pcm);
			pcm = nullptr;
		}

		if (this->sm24 != nullptr) {
			free(this->sm24);
			this->sm24 = nullptr;
		}

		//
		this->size = size;
		pcm = (short*)malloc(size * sizeof(short));
		if (pcm == nullptr) {
			this->size = 0;
			throw string("不能为样本分配足够的空间!");	
			return;
		}else {
			memcpy(pcm, samples, size * sizeof(short));
		}

		if (sm24 != nullptr) {
			this->sm24 = (uint8_t*)malloc(size * sizeof(uint8_t));
			if (this->sm24 != nullptr) 
				memcpy(this->sm24, sm24, size * sizeof(uint8_t));		
		}
	}
}
