#ifndef _Sample_h_
#define _Sample_h_

#include "TauTypes.h"

namespace tau
{
	//by cymheart, 2020--2021.
	class Sample
	{
	public:
		~Sample();


		//获取样本指定位置值
		inline float GetValue(uint32_t pos) {
			if (sm24 == nullptr) return pcm[pos] / 32768.0f * 0.95f; //16位样本值
			return (pcm[pos] << 8 | sm24[pos]) / 8388608.0f * 0.95f; //24位样本值
		}

		// 设置样本
		void SetSamples(short* samples, uint32_t size, uint8_t* sm24 = nullptr);

		// 设置原始音调
		inline void SetOriginalPitch(float pitch)
		{
			originalPitch = pitch;
		}

		// 获取原始音调
		inline float GetOriginalPitch()
		{
			return originalPitch;
		}


		// 设置音调按音分偏移校正
		inline void SetCentPitchCorrection(float cent)
		{
			centPitchCorrection = cent;
		}

		// 设置样本的采样力度
		inline void SetVelocity(float vel)
		{
			velocity = vel;
		}

		// 获取音调按音分偏移校正
		inline  float GetCentPitchCorrection()
		{
			return centPitchCorrection;
		}

		// 设置样本连接
		inline void SetSampleLink(Sample* sample)
		{
			sampleLink = sample;
		}

		//设置样本类型
		inline void SetSampleType(SampleType sampleType)
		{
			this->sampleType = sampleType;
		}

	public:

		string name;

		// PCM流
		short* pcm = nullptr;
		uint8_t* sm24 = nullptr; //24位样本低8位值
		size_t size = 0;


		// 样本采样率
		float sampleRate = 44100;

		// 原始音调
		float originalPitch = 60;

		// 音调修正(单位:音分, 100cents 为一个半音)
		float centPitchCorrection = 0;

		//样本采样力度
		float velocity = 127;

		// 样本类型
		SampleType sampleType = SampleType::MonoSample;

		// 连接的样本
		Sample* sampleLink = nullptr;

		// PCM流采样点起始点位置
		int startIdx = 0;

		// PCM流采样点结束点位置
		int endIdx = 0;

		// PCM流采样点循环起始点位置
		int startloopIdx = 0;

		// PCM流采样点循环结束点位置
		int endloopIdx = 0;

	};
}

#endif
