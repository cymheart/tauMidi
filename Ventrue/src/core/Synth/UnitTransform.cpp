#include"UnitTransform.h"

namespace ventrue
{
	// 分贝转增益  
	// <param name="db"></param>       
	float UnitTransform::DecibelsToGain(float db)
	{
		return db > -144.0f ? (float)pow(10.0f, db * 0.05f) : 0;
	}

	// 增益转分贝
	// <param name="gain"></param>
	float UnitTransform::GainToDecibels(float gain)
	{
		return gain <= 0.000001f ? -144 : (float)(20.0 * log10(gain));
	}

	// 共振峰Db值转滤波Q值
	// <param name="resonanceDb"></param>
	float UnitTransform::ResonanceDbToFilterQ(float resonanceDb)
	{
		return resonanceDb > -144.0f ? (float)pow(10.0f, resonanceDb * 0.05f) : 0;
	}

	//cB转分贝
	float UnitTransform::CentibelsToDecibels(float cB)
	{
		return cB / 10.0f;
	}

	// 音分转倍率
	// <param name="cents"></param>
	float UnitTransform::CentsToMul(float cents)
	{
		return (float)pow(1.00057779, cents);
	}

	// 音分转赫兹(Hz)
	float UnitTransform::CentsToHertz(float cents)
	{
		//return 8.176f * powf(2, cents / 1200);
		return 8.176f * FastPow2(cents / 1200.0f);
	}

	// timecents转秒
	float UnitTransform::TimecentsToSecsf(float timecents)
	{
		//(float)Math.Pow(2.0f, timecents / 1200.0f);
		return FastPow2(timecents / 1200.0f);
	}

	// 半音转倍率
	// 1半音之间的频率倍率为2^(1/12) = 1.059463f
	// <param name="semitone"></param>
	float UnitTransform::SemitoneToMul(float semitone)
	{
		return (float)pow(1.059463f, semitone);
	}
}
