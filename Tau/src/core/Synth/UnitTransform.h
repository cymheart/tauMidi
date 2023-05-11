#ifndef _UnitTransform_h_
#define _UnitTransform_h_

#include"TauTypes.h"

namespace tau
{
	/*
	  分贝单位转换
	  Nanobel (nB)
      1 Microbel (µB) = 1000 Nanobels (nB)
      1 Millibel (mB) = 1000 Microbels (µB)
      1 Centibel (cB) = 10 Millibels (mB)
      1 Decibel (dB) = 10 Centibels (cB)
      1 Bel (B) = 10 Decibels (dB)
      1 Decabel (daB) = 10 Bels (B)
      1 Hectobel (hB)= 10 Decabels (daB)
      1 Kilobel (kB) = 10 Hectobels (hB)
      1 Megabel (MB) = 1000 Kilobel (kB)
	*/

	class UnitTransform
	{
	public:

		// 分贝转增益   
		// <param name="db"></param>
		static float DecibelsToGain(float db);

		// 增益转分贝
		// <param name="gain"></param>
		static float GainToDecibels(float gain);

		// 共振峰Db值转滤波Q值
		// <param name="resonanceDb"></param>
		static float ResonanceDbToFilterQ(float resonanceDb);

		//cB转分贝
		static float CentibelsToDecibels(float cB);

		// 音分转倍率
		// 1 音分 = 1/100 半音（1 半音 = 100 音分）
		// 1 八度 = 12 半音 = 1200 音分。
		// 频率关系：每 1 音分对应频率比值为 2^(1/1200) =  1.00057779
		// 示例：升高 100 音分（1 半音）对应频率比  2^(100/1200) =  1.00057779
		// 示例：升高 1200 音分（1 个八度或者 12 个半音 ）对应频率比  2^(1200/1200) =  2
		// <param name="cents"></param>
		static float CentsToMul(float cents);

		// 音分转倍率
		// 1 音分 = 1/100 半音（1 半音 = 100 音分）
		// 1 八度 = 12 半音 = 1200 音分。
		// 频率关系：每 1 音分对应频率比值为 2^(1/1200) =  1.00057779
		// 示例：升高 100 音分（1 半音）对应频率比  2^(100/1200) =  1.00057779
		// 示例：升高 1200 音分（1 个八度或者 12 个半音 ）对应频率比  2^(1200/1200) =  2
		static float CentsToMul(GeneratorAmount genAmount);

		// 音分转赫兹(Hz)
		// <param name="cents"></param>
		static float CentsToHertz(float cents);

		// 音分转赫兹(Hz)
		static float CentsToHertz(GeneratorAmount genAmount);

		// timecents转秒   
		// <param name="timecents"></param>     
		static float TimecentsToSecsf(float timecents);

		// timecents转秒
		static float TimecentsToSec(short tiemcents);

		// timecents转秒   
		static float TimecentsToSec(GeneratorAmount genAmount);

		// 半音转倍率
		// 1半音之间的频率倍率为2^(1/12) = 1.059463f    
		// <param name="semitone"></param>   
		static float SemitoneToMul(float semitone);

	private:
		UnitTransform() {}
		~UnitTransform() {}

	};
}

#endif
