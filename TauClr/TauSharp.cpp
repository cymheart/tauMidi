
#include "TauSharp.h"

namespace TauClr
{
	TauSharp::TauSharp()
	{
		tau = new Tau();
		tau->SetFrameSampleCount(512);
		tau->SetSampleProcessRate(44100);
		tau->SetChildFrameSampleCount(64);
		tau->SetLimitRegionSounderCount(500);
		tau->SetLimitOnKeySpeed(600);
		tau->SetPerSyntherLimitTrackCount(20);
		tau->SetLimitSyntherCount(12);
		tau->SetOnlyUseOneMainSynther(true);
		tau->SetUseRegionInnerChorusEffect(true);
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

	void TauSharp::OnKey(int key, float velocity, InstrumentSharp^ inst, int delayMS)
	{
		tau->OnKey(key, velocity, inst->GetVirInstrument(), delayMS);
	}

	void TauSharp::OffKey(int key, float velocity, InstrumentSharp^ inst, int delayMS)
	{
		tau->OffKey(key, velocity, inst->GetVirInstrument(), delayMS);
	}

	void TauSharp::Load(String^ midiFilePath)
	{
		string stdMidiFilePath = marshal_as<string>(midiFilePath->ToString());
		tau->Load(stdMidiFilePath);
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
		return (TauClr::EditorState)tau->GetEditorState();
	}

	void TauSharp::SetSoundFont(SoundFontSharp^ sf)
	{
		tau->SetSoundFont(sf->GetSoundFont());
	}
}