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

	//������ٵ���������������
	class PhysicsPianoSampleGen : public ZoneSampleGenerator
	{
	public:
		PhysicsPianoSampleGen();
		~PhysicsPianoSampleGen() {}
		void Init(int key);
		//��������
		void Trigger(float velocity);

		/// <summary>
	/// ���������ֵ
	/// </summary>
	/// <param name="prevSampleIntPos">ǰһ����������λ��</param>
	/// <param name="nextSampleIntPos">��һ����������λ��</param>
	/// <param name="a">����֮��Ĳ�ֵ����</param>
	/// <returns></returns>
		float Out(int prevSampleIntPos, int nextSampleIntPos, float a);
	private:
		PianoKeyParams* param;
		PianoKey pianoKey;
		int samplePos = 0;
		float out[1024] = { 0 };
		int outCount = 0;
	};

	//������ٵ�����������
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