#include"PhysicsPiano.h"
#include"Instrument.h"
#include <OteyPiano/Piano.h>
#include"OteyPiano/PianoParams.h"
#include"Generator.h"
using namespace oteypiano;

namespace tau
{
	PhysicsPianoSampleGen::PhysicsPianoSampleGen()
	{
		param = (PianoKeyParams*)piano_params_default;
	}


	void PhysicsPianoSampleGen::Init(int key)
	{
		pianoKey.Init(param[key - 21]);
	}

	void PhysicsPianoSampleGen::Trigger(float velocity)
	{
		pianoKey.Trigger(velocity);
	}


	float PhysicsPianoSampleGen::Out(int prevSampleIntPos, int nextSampleIntPos, float a)
	{
		if (nextSampleIntPos == samplePos) {
			if (prevSampleIntPos == nextSampleIntPos)
				return out[0];
			return (out[0] * (1.0f - a) + out[1] * a);
		}

		//
		int count;
		if (prevSampleIntPos == samplePos)
		{
			out[0] = out[1];
			outCount = 1;
			count = 1;
		}
		else {
			outCount = 0;
			count = nextSampleIntPos - samplePos;
		}

		pianoKey.Go(out + outCount, count);
		samplePos = nextSampleIntPos;
		return (out[0] * (1.0f - a) + out[1] * a);
	}


	PhysicsPiano::PhysicsPiano()
	{
		param = (PianoKeyParams*)piano_params_default;
	}


	RegionSampleGenerator* PhysicsPiano::CreateRegionSampleGen(int key)
	{
		if (key < 21) key = 21;
		else if (key > 108) key = 108;

		PhysicsPianoSampleGen* gen = new PhysicsPianoSampleGen();
		gen->Init(key);
		return gen;
	}

	void PhysicsPiano::ReleaseRegionSampleGen(RegionSampleGenerator* sampleGen)
	{
		delete (PhysicsPianoSampleGen*)sampleGen;
	}


	void PhysicsPiano::Create(SoundFont* sf, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		sf->AddSampleGen(this);

		Instrument* inst = sf->AddInstrument("Physics Piano");
		Region* instGlobalRegion = inst->GetGlobalRegion();
		//instGlobalRegion->GetGenList()->SetAmount(GeneratorType::AttackVolEnv, 0.1f);
		instGlobalRegion->GetGenList()->SetAmount(GeneratorType::ReleaseVolEnv, 0.5f);

		sf->SampleGenBindToInstrument(this, inst);

		Preset* preset = sf->AddPreset("Physics Piano", 0, 1, 0);
		sf->InstrumentBindToPreset(inst, preset);
	}
}