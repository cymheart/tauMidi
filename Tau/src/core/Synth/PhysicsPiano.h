#ifndef _PhysicsPiano_h_
#define _PhysicsPiano_h_

#include"TauTypes.h"
#include"Midi/MidiTypes.h"
#include"SoundFont.h"
#include"Preset.h"
#include"SampleGenerator.h"
#include"OteyPiano/Piano.h"

using namespace oteypiano;

namespace tau
{

	//物理钢琴的区域样本生成器
	class PhysicsPianoSampleGen : public ZoneSampleGenerator
	{
	public:
		PhysicsPianoSampleGen();
		~PhysicsPianoSampleGen() {}
		void Init(int key);
		//触发按键
		void Trigger(float velocity);

		/// <summary>
	/// 输出采样点值
	/// </summary>
	/// <param name="prevSampleIntPos">前一个采样整数位置</param>
	/// <param name="nextSampleIntPos">后一个采样整数位置</param>
	/// <param name="a">两者之间的插值距离</param>
	/// <returns></returns>
		float Out(int prevSampleIntPos, int nextSampleIntPos, float a);
	private:
		PianoKeyParams* param;
		PianoKey pianoKey;
		int samplePos = 0;
		float out[1024] = { 0 };
		int outCount = 0;
	};

	//物理钢琴的样本生成器
	class PhysicsPiano : public SampleGenerator
	{
	public:
		PhysicsPiano();
		~PhysicsPiano() {}
		ZoneSampleGenerator* CreateZoneSampleGen(int key);
		void ReleaseZoneSampleGen(ZoneSampleGenerator* sampleGen);

		float GetSampleRate()
		{
			return 44100.0f;
		}

		void Create(SoundFont* sf, int bankSelectMSB, int bankSelectLSB, int instrumentNum);


	private:
		PianoKeyParams* param;
	};

}


#endif