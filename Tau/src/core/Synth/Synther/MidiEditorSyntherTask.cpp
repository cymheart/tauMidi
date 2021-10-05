#include"MidiEditorSynther.h"
#include"Synth/Tau.h"
#include"Synth/VirInstrument.h"
#include"Synth/Editor/MidiEditor.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"

namespace tau
{

	// 播放
	void MidiEditorSynther::PlayTask(Semaphore* waitSem)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _PlayTask;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_PlayTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.Play();
		se->sem->set();
	}

	// 停止
	void MidiEditorSynther::StopTask(Semaphore* waitSem)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _StopTask;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_StopTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.Stop();
		se->sem->set();
	}

	//暂停
	void MidiEditorSynther::PauseTask(Semaphore* waitSem)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _PauseTask;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_PauseTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.Pause();
		se->sem->set();
	}

	// 移除
	void MidiEditorSynther::RemoveTask(Semaphore* waitSem)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _RemoveTask;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_RemoveTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.Remove();
		se->sem->set();
	}

	// 指定播放的起始时间点
	void MidiEditorSynther::GotoTask(Semaphore* waitSem, float sec)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _GotoTask;
		ev->exValue[0] = sec;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_GotoTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.Goto(se->exValue[0]);
		se->sem->set();
	}

	//获取状态
	EditorState MidiEditorSynther::GetStateTask()
	{
		EditorState midiState;
		Semaphore waitSem;

		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _GetStateTask;
		ev->ptr = (void*)&midiState;
		ev->sem = &waitSem;
		PostTask(ev);

		//
		waitSem.wait();
		return midiState;
	}

	void MidiEditorSynther::_GetStateTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);

		EditorState* midEditorState = (EditorState*)se->ptr;
		*midEditorState = midiSynther.GetState();
		se->sem->set();
	}


	//获取结束时间(单位:秒)
	float MidiEditorSynther::GetEndSecTask()
	{
		float endSec;
		Semaphore waitSem;

		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _GetEndSecTask;
		ev->ptr = (void*)&endSec;
		ev->sem = &waitSem;
		PostTask(ev);

		//
		waitSem.wait();
		return endSec;
	}

	void MidiEditorSynther::_GetEndSecTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);

		float* endSec = (float*)se->ptr;
		*endSec = midiSynther.GetEndSec();
		se->sem->set();
	}


	//设置结束时间(单位:秒)
	void MidiEditorSynther::SetEndSecTask(Semaphore* waitSem, float sec)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetEndSecTask;
		ev->exValue[0] = sec;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_SetEndSecTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.SetEndSec(se->exValue[0]);

		se->sem->set();
	}


	// 设定播放速度
	void MidiEditorSynther::SetSpeedTask(Semaphore* waitSem, float speed)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetSpeedTask;
		ev->exValue[0] = speed;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_SetSpeedTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.SetSpeed(se->exValue[0]);

		se->sem->set();
	}

	// 禁止播放指定的轨道
	void MidiEditorSynther::DisableTrackTask(Semaphore* waitSem, Track* track)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _DisableTrackTask;
		ev->ptr = track;
		ev->sem = waitSem;
		PostTask(ev);
	}

	// 禁止播放所有轨道
	void MidiEditorSynther::DisableAllTrackTask(Semaphore* waitSem)
	{
		DisableTrackTask(waitSem, nullptr);
	}


	void MidiEditorSynther::_DisableTrackTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.DisableTrack((Track*)se->ptr);
		se->sem->set();
	}


	// 启用播放指定编号的轨道
	void MidiEditorSynther::EnableTrackTask(Semaphore* waitSem, Track* track)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _EnableTrackTask;
		ev->ptr = track;
		ev->sem = waitSem;
		PostTask(ev);
	}

	// 启用播放所有轨道
	void MidiEditorSynther::EnableAllTrackTask(Semaphore* waitSem)
	{
		EnableTrackTask(waitSem, nullptr);
	}

	void MidiEditorSynther::_EnableTrackTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.EnableTrack((Track*)se->ptr);
		se->sem->set();
	}

	// 禁止播放指定编号轨道通道
	void MidiEditorSynther::DisableChannelTask(Semaphore* waitSem, int channelIdx)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _DisableChannelTask;
		ev->value = channelIdx;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_DisableChannelTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.DisableChannel(se->value);
		se->sem->set();
	}

	// 启用播放指定编号轨道通道
	void MidiEditorSynther::EnableChannelTask(Semaphore* waitSem, int channelIdx)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _EnableChannelTask;
		ev->value = channelIdx;
		ev->sem = waitSem;
		PostTask(ev);

	}

	void MidiEditorSynther::_EnableChannelTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.EnableChannel(se->value);
		se->sem->set();
	}

	// 设置对应轨道的乐器
	void MidiEditorSynther::SetVirInstrumentTask(
		Semaphore* waitSem, Track* track,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetVirInstrumentTask;
		ev->ptr = track;
		ev->exValue[0] = bankSelectMSB;
		ev->exValue[1] = bankSelectLSB;
		ev->exValue[2] = instrumentNum;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_SetVirInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);

		midiSynther.SetVirInstrument(
			(Track*)se->ptr,
			(int)se->exValue[0], (int)se->exValue[1], (int)se->exValue[2]);

		se->sem->set();
	}

	//设置打击乐号
	void MidiEditorSynther::SetBeatVirInstrumentTask(
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetBeatVirInstrumentTask;
		ev->exValue[0] = bankSelectMSB;
		ev->exValue[1] = bankSelectLSB;
		ev->exValue[2] = instrumentNum;
		PostTask(ev);
	}

	void MidiEditorSynther::_SetBeatVirInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);

		midiSynther.SetBeatVirInstrument(
			(int)se->exValue[0], (int)se->exValue[1], (int)se->exValue[2]);
	}


	void MidiEditorSynther::SetMarkerListTask(Semaphore* waitSem, MidiMarkerList* mrklist)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetMarkerListTask;
		ev->ptr = mrklist;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_SetMarkerListTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.SetMarkerList((MidiMarkerList*)se->ptr);

		se->sem->set();
	}


	void MidiEditorSynther::NewTrackTask(Semaphore* waitSem)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _NewTrackTask;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_NewTrackTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.NewTrack();

		se->sem->set();
	}

	void MidiEditorSynther::DeleteTrackTask(Semaphore* waitSem, Track* track)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _DeleteTrackTask;
		ev->ptr = track;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_DeleteTrackTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.DeleteTrack((Track*)se->ptr);

		se->sem->set();
	}


	void MidiEditorSynther::MoveInstFragmentTask(
		Semaphore* waitSem, InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _MoveInstFragmentTask;
		ev->ptr = instFragment;
		ev->exPtr[0] = dstTrack;
		ev->value = dstBranchIdx;
		ev->exValue[0] = sec;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_MoveInstFragmentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.MoveInstFragment((InstFragment*)se->ptr, (Track*)se->exPtr[0], se->value, se->exValue[0]);

		se->sem->set();
	}

	void MidiEditorSynther::RemoveInstFragmentTask(Semaphore* waitSem, InstFragment* instFragment)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _RemoveInstFragmentTask;
		ev->ptr = instFragment;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_RemoveInstFragmentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.RemoveInstFragment((InstFragment*)se->ptr);
		se->sem->set();
	}


	void MidiEditorSynther::ComputeTrackEventsTimeTask(Semaphore* waitSem, Track* track)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _ComputeTrackEventsTimeTask;
		ev->ptr = track;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_ComputeTrackEventsTimeTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.ComputeTrackEventsTime((Track*)se->ptr);
		se->sem->set();
	}

	void MidiEditorSynther::ComputeEndSecTask(Semaphore* waitSem)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _ComputeEndSecTask;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void MidiEditorSynther::_ComputeEndSecTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		MidiEditorSynther& midiSynther = (MidiEditorSynther&)*(se->synther);
		midiSynther.ComputeEndSec();
		se->sem->set();
	}
}
