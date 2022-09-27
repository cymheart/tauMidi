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

		midiEditor->Process(tau->unitSampleSec);
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

		//在缓存打开时，处理缓存命令操作
		if (maxCacheSize > 0 && isEnableCache)
			CachePlay();
	}

	//停止
	void MidiEditorSynther::Stop()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Stop();

		//在缓存打开时，处理缓存命令操作
		if (maxCacheSize > 0 && isEnableCache)
			CacheStop();
	}

	//暂停
	void MidiEditorSynther::Pause()
	{
		if (midiEditor == nullptr)
			return;

		//在缓存打开时，处理缓存命令操作
		if (maxCacheSize > 0 && isEnableCache)
			CachePause();
		else
			midiEditor->Pause();
	}

	//快进到指定位置
	void MidiEditorSynther::Goto(double sec)
	{
		if (midiEditor == nullptr)
			return;

		double endsec = midiEditor->GetEndSec();
		if (sec > endsec)
			sec = endsec;

		midiEditor->Goto(sec);

		//在缓存打开时，处理缓存命令操作
		if (maxCacheSize > 0 && isEnableCache) {
			if (CacheGoto(sec))
				midiEditor->Play();
		}
	}

	//移除midiEditor
	void MidiEditorSynther::Remove()
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Remove();
		DEL(midiEditor);

		//在缓存打开时，处理缓存命令操作
		if (maxCacheSize > 0 && isEnableCache)
			CacheStop(true);
	}

	//重新缓存
	void MidiEditorSynther::ReCache()
	{
		//在缓存打开时，处理缓存命令操作
		if (maxCacheSize > 0 && isEnableCache)
			CacheReset();
	}

	void MidiEditorSynther::SetPlayType(MidiEventPlayType playType)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetPlayType(playType);
	}

	void MidiEditorSynther::GetCurTimeLateNoteKeys(float lateSec)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->GetCurTimeLateNoteKeys(lateSec);
	}


	void MidiEditorSynther::EnterPlayMode(EditorPlayMode playMode)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->EnterPlayMode(playMode);

		if ((playMode == EditorPlayMode::Step ||
			playMode == EditorPlayMode::Wait) &&
			maxCacheSize > 0 && isEnableCache)
		{
			midiEditor->Goto(curtCachePlaySec);
			ReCache();
			CacheEnterStepPlayMode(); //关闭缓存
		}

	}

	void MidiEditorSynther::LeavePlayMode()
	{
		if (midiEditor == nullptr)
			return;

		isStepPlayMode = false;
		midiEditor->LeavePlayMode();
	}


	void MidiEditorSynther::Runto(double sec)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->Runto(sec);
	}

	//获取播放状态(通用)
	EditorState MidiEditorSynther::GetPlayStateCommon()
	{
		if (midiEditor == nullptr)
			return EditorState::STOP;

		if (maxCacheSize > 0 && isEnableCache)
			return GetCachePlayState();
		return midiEditor->GetState();
	}

	//获取播放时间(通用)
	double MidiEditorSynther::GetPlaySecCommon()
	{
		if (midiEditor == nullptr)
			return 0;

		if (maxCacheSize > 0 && isEnableCache)
			return GetCachePlaySec();
		return midiEditor->GetPlaySec();
	}

	//获取播放时间
	double MidiEditorSynther::GetPlaySec()
	{
		if (midiEditor == nullptr)
			return 0;

		return midiEditor->GetPlaySec();
	}

	//获取状态
	EditorState MidiEditorSynther::GetPlayState()
	{
		if (midiEditor == nullptr)
			return EditorState::STOP;

		return midiEditor->GetState();
	}

	//获取结束时间(单位:秒)
	double MidiEditorSynther::GetEndSec()
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
		ReCache();
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

		ReCache();
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

		ReCache();
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

		ReCache();
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

		ReCache();
	}

	//设置轨道事件演奏方式
	void MidiEditorSynther::SetTrackPlayType(Track* track, MidiEventPlayType playType)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetTrackPlayType(track, playType);
	}

	// 设置对应轨道的乐器
	void MidiEditorSynther::SetVirInstrument(Track* track,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetVirInstrument(
			track, bankSelectMSB, bankSelectLSB, instrumentNum);

		ReCache();
	}


	//设置打击乐号
	void MidiEditorSynther::SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->SetBeatVirInstrument(bankSelectMSB, bankSelectLSB, instrumentNum);
		ReCache();
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
		ReCache();
	}

	//移动乐器片段到目标轨道的指定时间点
	void MidiEditorSynther::MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->MoveInstFragment(instFragment, dstTrack, dstBranchIdx, sec);
		ReCache();
	}

	//移除乐器片段
	void MidiEditorSynther::RemoveInstFragment(InstFragment* instFragment)
	{
		if (midiEditor == nullptr)
			return;

		midiEditor->RemoveInstFragment(instFragment);
		ReCache();
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

	bool MidiEditorSynther::CanCache()
	{
		//仅当state == play时，才可以缓存
		if (!isCacheWriteSoundEnd ||
			(midiEditor != nullptr && midiEditor->GetState() == EditorState::PLAY))
			return true;

		isCacheWriteSoundEnd = true;
		return false;
	}

}
