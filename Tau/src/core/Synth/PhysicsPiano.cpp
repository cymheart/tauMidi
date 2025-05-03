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
		pianoKey.Init(param[key - A0]);
	}

	//��������
	void PhysicsPianoSampleGen::Trigger(float velocity)
	{
		pianoKey.Trigger(velocity);
	}

	/// <summary>
	/// ���������ֵ
	/// </summary>
	/// <param name="prevSampleIntPos">ǰһ����������λ��</param>
	/// <param name="nextSampleIntPos">��һ����������λ��</param>
	/// <param name="a">����֮��Ĳ�ֵ����</param>
	/// <returns></returns>
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


	ZoneSampleGenerator* PhysicsPiano::CreateZoneSampleGen(int key)
	{
		if (key < A0) key = A0;
		else if (key > C8) key = C8;

		PhysicsPianoSampleGen* gen = new PhysicsPianoSampleGen();
		gen->Init(key);
		return gen;
	}

	void PhysicsPiano::ReleaseZoneSampleGen(ZoneSampleGenerator* sampleGen)
	{
		delete (PhysicsPianoSampleGen*)sampleGen;
	}


	void PhysicsPiano::Create(SoundFont* sf, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		sf->AddSampleGen(this);

		Instrument* inst = sf->AddInstrument("Physics Piano");
		Zone* instGlobalZone = inst->GetGlobalZone();
		sf->SampleGenBindToInstrument(this, inst);
		//instGlobalZone->GetGenList()->SetAmount(GeneratorType::AttackVolEnv, 0.1f);

		GeneratorAmount a;
		instGlobalZone->GetGens().SetAmount(GeneratorType::ReleaseVolEnv, a);
		Preset* preset = sf->AddPreset("Physics Piano", bankSelectMSB, bankSelectLSB, instrumentNum);
		sf->InstrumentBindToPreset(inst, preset);
	}
}