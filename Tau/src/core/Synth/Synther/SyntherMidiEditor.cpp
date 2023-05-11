#include"Synther.h"
#include"Synth/VirInstrument.h"
#include"Synth/Editor/MidiEditor.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"
#include"Synth/Tau.h"

namespace tau
{

	// 处理midi事件
	void Synther::ProcessMidiEvents()
	{
		midiEditor->Process(tau->unitSampleSec * midiEditor->GetSpeed());
	}

	//建立MidiEditor
	void Synther::CreateMidiEditor()
	{
		if (midiEditor != nullptr)
			return;

		midiEditor = new MidiEditor(this);
	}


	//播放
	void Synther::Play()
	{
		midiEditor->Play();

		if (IsCacheEnable())
			CachePlay();
	}

	//暂停
	void Synther::Pause()
	{
		if (IsCacheEnable())
			CachePause();
		else
			midiEditor->Pause();
	}

	//快进到指定位置
	void Synther::Goto(double sec)
	{
		double endsec = midiEditor->GetEndSec();
		if (sec > endsec)
			sec = endsec;

		midiEditor->Goto(sec);

		if (IsCacheEnable()) {
			if (CacheGoto(sec)) {
				midiEditor->Play();
			}
		}
	}

	//停止
	void Synther::Stop()
	{
		midiEditor->Stop();

		if (IsCacheEnable()) {
			//当停止或移除播放midi时，当前播放midi所对应的乐器的发声会有一个渐渐减弱的时间
			//在缓存状态下，需要直接关闭这些发音乐器和清空效果器，而直接使用缓存中的残留声音进行减弱处理
			StopAllVirInstrument();
			effects->Clear();
			CacheStop();
		}
	}

	//移除
	void Synther::Remove()
	{
		midiEditor->Remove();

		if (IsCacheEnable()) {
			//当停止或移除播放midi时，当前播放midi所对应的乐器的发声会有一个渐渐减弱的时间
			//在缓存状态下，需要直接关闭这些发音乐器和清空效果器，而直接使用缓存中的残留声音进行减弱处理
			DelAllVirInstrument();
			effects->Clear();
			CacheStop();
		}
	}

	//重新缓存
	void Synther::ReCache()
	{
		if (IsCacheEnable())
			CacheReset();
	}

	void Synther::SetPlayType(MidiEventPlayType playType)
	{
		midiEditor->SetPlayType(playType);
	}

	void Synther::GetCurTimeLateNeedWaitKeySignalNote(int note, float lateSec)
	{
		midiEditor->GetCurTimeLateNeedWaitKeySignalNote(note, lateSec);
	}


	void Synther::EnterPlayMode(EditorPlayMode playMode)
	{
		isStepPlayMode = false;
		midiEditor->EnterPlayMode(playMode);

		if ((playMode == EditorPlayMode::Step ||
			playMode == EditorPlayMode::Wait ||
			playMode == EditorPlayMode::Mute) &&
			maxCacheSize > 0 && isEnableCache)
		{
			midiEditor->Goto(curtCachePlaySec);
			ReCache();
			CacheEnterStepPlayMode(); //关闭缓存
		}

	}

	void Synther::LeavePlayMode()
	{
		isStepPlayMode = false;
		midiEditor->LeavePlayMode();
	}


	void Synther::Runto(double sec)
	{
		midiEditor->Runto(sec);
	}

	//获取播放状态(通用)
	EditorState Synther::GetPlayStateCommon()
	{
		if (maxCacheSize > 0 && isEnableCache)
			return GetCachePlayState();
		return midiEditor->GetState();
	}

	//获取播放时间(通用)
	double Synther::GetPlaySecCommon()
	{
		if (maxCacheSize > 0 && isEnableCache)
			return GetCachePlaySec();
		return midiEditor->GetPlaySec();
	}

	//获取播放时间
	double Synther::GetPlaySec()
	{
		return midiEditor->GetPlaySec();
	}

	//获取状态
	EditorState Synther::GetPlayState()
	{
		return midiEditor->GetState();
	}

	//获取结束时间(单位:秒)
	double Synther::GetEndSec()
	{
		return midiEditor->GetEndSec();
	}

	//设置结束时间(单位:秒)
	void Synther::SetEndSec(double sec)
	{
		return midiEditor->SetEndSec(sec);
	}


	//设置播放速度
	void Synther::SetSpeed(float speed)
	{
		midiEditor->SetSpeed(speed);
		ReCache();
	}

	// 禁止播放指定轨道
	void Synther::DisableTrack(Track* track)
	{
		if (track == nullptr)
			midiEditor->DisableAllTrack();
		else
			midiEditor->DisableTrack(track);

		ReCache();
	}

	// 禁止播放指定的轨道
	void Synther::DisableTrack(int trackIdx)
	{
		midiEditor->DisableTrack(trackIdx);
		ReCache();
	}

	// 启用播放指定的轨道
	void Synther::EnableTrack(Track* track)
	{
		if (track == nullptr)
			midiEditor->EnableAllTrack();
		else
			midiEditor->EnableTrack(track);

		ReCache();
	}

	// 启用播放指定的轨道
	void Synther::EnableTrack(int trackIdx)
	{
		midiEditor->EnableTrack(trackIdx);
		ReCache();
	}


	// 禁止播放指定编号通道
	void Synther::DisableChannel(int channelIdx)
	{
		if (channelIdx == -1)
			midiEditor->DisableAllTrack();
		else
			midiEditor->DisableChannel(channelIdx);

		ReCache();
	}

	// 启用播放指定编号通道
	void Synther::EnableChannel(int channelIdx)
	{
		if (channelIdx == -1)
			midiEditor->EnableAllTrack();
		else
			midiEditor->EnableChannel(channelIdx);

		ReCache();
	}

	//设置轨道事件演奏方式
	void Synther::SetTrackPlayType(Track* track, MidiEventPlayType playType)
	{
		midiEditor->SetTrackPlayType(track, playType);
	}

	// 设置对应轨道的乐器
	void Synther::SetVirInstrument(Track* track,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		midiEditor->SetVirInstrument(
			track, bankSelectMSB, bankSelectLSB, instrumentNum);

		ReCache();
	}

	// 设置对应轨道的乐器
	void Synther::SetVirInstrument(int trackIdx,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		midiEditor->SetVirInstrument(
			trackIdx, bankSelectMSB, bankSelectLSB, instrumentNum);

		ReCache();
	}


	//设置打击乐号
	void Synther::SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		midiEditor->SetBeatVirInstrument(bankSelectMSB, bankSelectLSB, instrumentNum);
		ReCache();
	}

	//设置标记
	void Synther::SetMarkerList(MidiMarkerList* mrklist)
	{
		midiEditor->SetMarkerList(mrklist);
	}

	//新建轨道
	void Synther::NewTrack()
	{
		midiEditor->NewTrack();
	}

	//删除轨道
	void Synther::DeleteTrack(Track* track)
	{
		midiEditor->DeleteTrack(track);
		ReCache();
	}

	//移动乐器片段到目标轨道的指定时间点
	void Synther::MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec)
	{
		midiEditor->MoveInstFragment(instFragment, dstTrack, dstBranchIdx, sec);
		ReCache();
	}

	//移除乐器片段
	void Synther::RemoveInstFragment(InstFragment* instFragment)
	{
		midiEditor->RemoveInstFragment(instFragment);
		ReCache();
	}


	//计算指定轨道所有事件的实际时间点
	void Synther::ComputeTrackEventsTime(Track* track)
	{
		midiEditor->ComputeTrackEventsTime(track);
	}

	//计算结束时间点
	void Synther::ComputeEndSec()
	{
		midiEditor->ComputeEndSec();
	}

}
