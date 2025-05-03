
#include "TauSharp.h"

namespace TauClr
{
	TauSharp::TauSharp()
	{
		tau = new Tau();
		tau->SetFrameSampleCount(512);
		tau->SetSampleProcessRate(44100);
		tau->SetChildFrameSampleCount(64);
		tau->SetLimitZoneSounderCount(500);
		tau->SetLimitOnKeySpeed(600);

		tau->SetUseZoneInnerChorusEffect(true);
		tau->SetEnableAllVirInstEffects(true);
	}

	TauSharp::~TauSharp()
	{
		delete tau;
	}

	void TauSharp::Open()
	{
		tau->Open();
	}

	InstrumentSharp^ TauSharp::EnableInstrument(
		int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		VirInstrument* inst = tau->EnableVirInstrument(deviceChannelNum, bankSelectMSB, bankSelectLSB, instrumentNum);
		InstrumentSharp^ instSharp = gcnew InstrumentSharp();
		instSharp->SetVirInstrument(inst);
		return instSharp;
	}

	void TauSharp::OnKey(int key, float velocity, InstrumentSharp^ inst)
	{
		tau->OnKey(key, velocity, inst->GetVirInstrument());
	}

	void TauSharp::OffKey(int key, float velocity, InstrumentSharp^ inst)
	{
		tau->OffKey(key, velocity, inst->GetVirInstrument());
	}

	bool TauSharp::IsLoadCompleted()
	{
		return tau->IsLoadCompleted();
	}

	void TauSharp::Load(String^ midiFilePath, bool isWaitLoadCompleted)
	{
		string stdMidiFilePath = marshal_as<string>(midiFilePath->ToString());
		tau->Load(stdMidiFilePath, isWaitLoadCompleted);
	}

	void TauSharp::Load(String^ midiFilePath)
	{
		Load(midiFilePath, true);
	}

	void TauSharp::Play()
	{
		tau->Play();
	}

	void TauSharp::Stop()
	{
		tau->Stop();
	}

	void TauSharp::Pause()
	{
		tau->Pause();
	}

	void TauSharp::Close()
	{
		tau->Close();
	}

	void TauSharp::Goto(float sec)
	{
		tau->Goto(sec);
	}

	TauClr::EditorState TauSharp::GetEditorState()
	{
		return (TauClr::EditorState)tau->GetPlayState();
	}

	//��ȡ��ǰ����ʱ���
	double TauSharp::GetPlaySec()
	{
		return tau->GetPlaySec();
	}

	//��ȡ����ʱ���
	double TauSharp::GetEndSec()
	{
		return tau->GetEndSec();
	}

	void TauSharp::SetSoundFont(SoundFontSharp^ sf)
	{
		tau->SetSoundFont(sf->GetSoundFont());
	}

	// �趨�����ٶ�
	void TauSharp::SetSpeed(float speed)
	{
		tau->SetSpeed(speed);
	}
}