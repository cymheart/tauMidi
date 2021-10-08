#include"MidiEditorSynther.h"
#include"Synth/VirInstrument.h"
#include"Synth/Editor/MidiEditor.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"
#include"Synth/Tau.h"

namespace tau
{
	MidiEditorSynther::MidiEditorSynther(Tau* tau)
		:RealtimeSynther(tau)
	{

	}

	MidiEditorSynther::~MidiEditorSynther()
	{
		DEL(midiEditor);
	}


	// 处理midi事件
	void MidiEditorSynther::ProcessMidiEvents()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Run(tau->unitSampleSec);
	}

	//建立MidiEditor
	MidiEditor* MidiEditorSynther::CreateMidiEditor()
	{
		if (midiEditor != nullptr)
			return midiEditor;

		midiEditor = new MidiEditor(this);
		return midiEditor;
	}


	//播放
	void MidiEditorSynther::Play()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Play();
	}

	//停止
	void MidiEditorSynther::Stop()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Stop();
	}

	//暂停
	void MidiEditorSynther::Pause()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Pause();
	}

	//移除midiEditor
	void MidiEditorSynther::Remove()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Remove();
		DEL(midiEditor);
	}

	void MidiEditorSynther::EnterStepPlayMode()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->EnterStepPlayMode();
	}

	void MidiEditorSynther::LeaveStepPlayMode()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->LeaveStepPlayMode();
	}

	void MidiEditorSynther::Runto(double sec)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Runto(sec);
	}

	//快进到指定位置
	void MidiEditorSynther::Goto(double sec)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Goto(sec);
	}

	//获取播放时间
	float MidiEditorSynther::GetPlaySec()
	{
		return midiEditor->GetPlaySec();
	}

	//获取状态
	EditorState MidiEditorSynther::GetState()
	{
		if (midiEditor == nullptr)
			return EditorState::STOP;

		return midiEditor->GetState();
	}

	//获取结束时间(单位:秒)
	float MidiEditorSynther::GetEndSec()
	{
		if (midiEditor == nullptr)
			return 0;

		return midiEditor->GetEndSec();
	}

	//设置结束时间(单位:秒)
	void MidiEditorSynther::SetEndSec(double sec)
	{
		if (midiEditor == nullptr)
			return;

		return midiEditor->SetEndSec(sec);
	}


	//设置播放速度
	void MidiEditorSynther::SetSpeed(float speed)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetSpeed(speed);
	}

	// 禁止播放指定轨道
	void MidiEditorSynther::DisableTrack(Track* track)
	{
		if (midiEditor == nullptr)
			return;

		if (track == nullptr)
			midiEditor->DisableAllTrack();
		else
			midiEditor->DisableTrack(track);
	}

	// 启用播放指定的轨道
	void MidiEditorSynther::EnableTrack(Track* track)
	{
		if (midiEditor == nullptr)
			return;

		if (track == nullptr)
			midiEditor->EnableAllTrack();
		else
			midiEditor->EnableTrack(track);
	}

	// 禁止播放指定编号通道
	void MidiEditorSynther::DisableChannel(int channelIdx)
	{
		if (midiEditor == nullptr)
			return;

		if (channelIdx == -1)
			midiEditor->DisableAllTrack();
		else
			midiEditor->DisableChannel(channelIdx);
	}

	// 启用播放指定编号通道
	void MidiEditorSynther::EnableChannel(int channelIdx)
	{
		if (midiEditor == nullptr)
			return;

		if (channelIdx == -1)
			midiEditor->EnableAllTrack();
		else
			midiEditor->EnableChannel(channelIdx);
	}


	// 设置对应轨道的乐器
	void MidiEditorSynther::SetVirInstrument(Track* track,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetVirInstrument(
			track, bankSelectMSB, bankSelectLSB, instrumentNum);
	}


	//设置打击乐号
	void MidiEditorSynther::SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetBeatVirInstrument(bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	//设置标记
	void MidiEditorSynther::SetMarkerList(MidiMarkerList* mrklist)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetMarkerList(mrklist);
	}

	//新建轨道
	void MidiEditorSynther::NewTrack()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->NewTrack();
	}

	//删除轨道
	void MidiEditorSynther::DeleteTrack(Track* track)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->DeleteTrack(track);
	}

	//移动乐器片段到目标轨道的指定时间点
	void MidiEditorSynther::MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->MoveInstFragment(instFragment, dstTrack, dstBranchIdx, sec);
	}

	//移除乐器片段
	void MidiEditorSynther::RemoveInstFragment(InstFragment* instFragment)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->RemoveInstFragment(instFragment);
	}


	//计算指定轨道所有事件的实际时间点
	void MidiEditorSynther::ComputeTrackEventsTime(Track* track)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->ComputeTrackEventsTime(track);
	}

	//计算结束时间点
	void MidiEditorSynther::ComputeEndSec()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->ComputeEndSec();
	}

}
