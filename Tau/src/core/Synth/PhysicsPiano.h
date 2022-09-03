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

	class PhysicsPianoSampleGen : public RegionSampleGenerator
	{
	public:
		PhysicsPianoSampleGen();
		~PhysicsPianoSampleGen() {}
		void Init(int key);
		void Trigger(float velocity);
		float Out(int prevSampleIntPos, int nextSampleIntPos, float a);
	private:
		PianoKeyParams* param;
		PianoKey pianoKey;
		int samplePos = 0;
		float out[1024] = { 0 };
		int outCount = 0;
	};


	class PhysicsPiano : public SampleGenerator
	{
	public:
		PhysicsPiano();
		~PhysicsPiano() {}
		RegionSampleGenerator* CreateRegionSampleGen(int key);
		void ReleaseRegionSampleGen(RegionSampleGenerator* sampleGen);

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