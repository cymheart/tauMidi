#include"Tau.h"
#include"Midi/MidiFile.h"
#include"MidiTrackRecord.h"
#include"Synther/Synther.h"
#include"Editor/Editor.h"

namespace tau
{

	//设置是否开启伴奏
	void Tau::SetOpenAccompany(bool isOpen)
	{
		editor->SetOpenAccompany(isOpen);
	}

	//设置初始化开始播放时间点
	void Tau::SetInitStartPlaySec(double sec)
	{
		editor->SetInitStartPlaySec(sec);
	}

	// 按下按键
	void Tau::OnKey(int key, float velocity, int trackIdx, int id)
	{
		editor->OnKey(key, velocity, trackIdx, id);
	}

	// 释放按键 
	void Tau::OffKey(int key, float velocity, int trackIdx, int id)
	{
		editor->OffKey(key, velocity, trackIdx, id);
	}

	// 释放指定轨道的所有按键 
	void Tau::OffAllKeysForTrack(int trackIdx)
	{
		editor->OffAllKeysForTrack(trackIdx);
	}

	// 释放所有按键 
	void Tau::OffAllKeys()
	{
		editor->OffAllKeys();
	}

	// 释放与指定id匹配的所有按键 
	void Tau::OffAllKeys(int id)
	{
		editor->OffAllKeys(id);
	}

	//判断是否载入完成
	bool Tau::IsLoadCompleted()
	{
		return editor->IsLoadCompleted();
	}

	//判断是否全部解析了midiFile
	bool Tau::IsFullParsedMidiFile()
	{
		return editor->IsFullParsedMidiFile();
	}

	//载入
	//在非阻塞模式isWaitLoadCompleted = false下，
	//Load前面不能有任何与之相冲突的调用（play(), pasue(),stop()等,因为这些函数出于效率考虑没有加互斥锁）
	void Tau::Load(string& midiFilePath, bool isWaitLoadCompleted)
	{
		editor->Load(midiFilePath, isWaitLoadCompleted);
	}

	//设置简单模式下, 白色按键的数量
	void Tau::SetSimpleModePlayWhiteKeyCount(int count)
	{
		editor->SetSimpleModePlayWhiteKeyCount(count);
	}

	//生成简单模式音符轨道
	void Tau::CreateSimpleModeTrack()
	{
		editor->CreateSimpleModeTrack();
	}

	//新建轨道
	void Tau::NewTracks(int count)
	{
		editor->NewTracks(count);
	}

	//是否等待中
	bool Tau::IsWait()
	{
		return editor->IsWait();
	}

	//获取播放模式
	EditorPlayMode Tau::GetPlayMode()
	{
		return editor->GetPlayMode();
	}

	//播放
	void Tau::Play()
	{
		editor->Play();

	}

	//停止播放midi
	void Tau::Stop()
	{
		editor->Stop();
	}

	//暂停播放midi
	void Tau::Pause()
	{
		editor->Pause();
	}

	//移除midi
	void Tau::Remove()
	{
		editor->Remove();
	}

	//进入到步进播放模式
	void Tau::EnterStepPlayMode()
	{
		editor->EnterStepPlayMode();
	}

	//进入到等待播放模式
	void Tau::EnterWaitPlayMode()
	{
		editor->EnterWaitPlayMode();
	}

	//进入到静音模式
	void Tau::EnterMuteMode()
	{
		editor->EnterMuteMode();
	}

	//离开播放模式
	void Tau::LeavePlayMode()
	{
		editor->LeavePlayMode();
	}

	//设置事件演奏方式
	void Tau::SetPlayType(MidiEventPlayType playType)
	{
		editor->SetPlayType(playType);
	}

	//等待(区别于暂停，等待相当于在原始位置播放)
	void Tau::Wait()
	{
		editor->Wait();
	}
	//继续，相对于等待命令
	void Tau::Continue()
	{
		editor->Continue();
	}

	//设置编辑器排除需要等待的按键
	void Tau::EditorSetExcludeNeedWaitKeys(int* excludeKeys, int size)
	{
		editor->SetExcludeNeedWaitKeys(excludeKeys, size);
	}

	//设置编辑器排除需要等待的按键
	void Tau::EditorSetExcludeNeedWaitKey(int key)
	{
		editor->SetExcludeNeedWaitKey(key);
	}

	//设置编辑器包含需要等待的按键
	void Tau::EditorSetIncludeNeedWaitKey(int key)
	{
		editor->SetIncludeNeedWaitKey(key);
	}

	//发送编辑器按键信号
	void Tau::EditorOnKeySignal(int key)
	{
		editor->OnKeySignal(key);
	}

	//发送编辑器松开按键信号
	void Tau::EditorOffKeySignal(int key)
	{
		editor->OffKeySignal(key);
	}

	//移动到指定时间点
	void Tau::Runto(double sec)
	{
		editor->Runto(sec);
	}

	// 指定midi文件播放的起始时间点
	void Tau::Goto(float sec)
	{
		editor->Goto(sec);
	}

	//获取midi播放状态
	EditorState Tau::GetPlayState()
	{
		return editor->GetPlayState();
	}

	//获取当前播放时间点
	double Tau::GetPlaySec()
	{
		return editor->GetPlaySec();
	}

	//获取结束时间点
	double Tau::GetEndSec()
	{
		return editor->GetEndSec();
	}

	//获取当前bpm
	float Tau::GetCurtBPM()
	{
		return editor->GetCurtBPM();
	}

	//根据指定tick数秒数获取时间点
	double Tau::GetTickSec(uint32_t tick)
	{
		return editor->GetTickSec(tick);
	}


	// 设定播放速度
	void Tau::SetSpeed(float speed)
	{
		editor->SetSpeed(speed);
	}

	// 禁止播放指定编号Midi文件的轨道
	void Tau::DisableTrack(int trackIdx)
	{
		editor->DisableTrack(trackIdx);
	}


	// 禁止播放Midi的所有轨道
	void Tau::DisableAllTrack()
	{
		editor->DisableAllTrack();
	}

	// 启用播放指定编号Midi文件的轨道
	void Tau::EnableTrack(int trackIdx)
	{
		editor->EnableTrack(trackIdx);
	}

	// 启用播放Midi的所有轨道
	void Tau::EnableAllTrack()
	{
		editor->EnableAllTrack();
	}


	// 禁止播放指定编号Midi文件通道
	void Tau::DisableChannel(int channelIdx)
	{
		editor->DisableChannel(channelIdx);
	}

	// 启用播放指定编号Midi文件通道
	void Tau::EnableChannel(int channelIdx)
	{
		editor->EnableChannel(channelIdx);
	}

	//设置通道声音增益(单位:dB)
	void Tau::SetChannelVolumeGain(int channelIdx, float gainDB)
	{
		editor->SetChannelVolumeGain(channelIdx, gainDB);
	}


	void Tau::SetSelectInstFragMode(SelectInstFragMode mode)
	{
		editor->SetSelectInstFragMode(mode);
	}

	void Tau::SelectInstFragment(int trackIdx, int branchIdx, int instFragIdx)
	{
		editor->SelectInstFragment(trackIdx, branchIdx, instFragIdx);
	}
	void Tau::UnSelectInstFragment(int trackIdx, int branchIdx, int instFragIdx)
	{
		editor->UnSelectInstFragment(trackIdx, branchIdx, instFragIdx);
	}

	void Tau::UnSelectAllInstFragment()
	{
		editor->UnSelectAllInstFragment();
	}

	//移动乐器片段到目标轨道分径的指定时间点
	void Tau::MoveSelectedInstFragment(int dstTrack, int dstBranchIdx, float sec)
	{
		editor->MoveSelectedInstFragment(dstTrack, dstBranchIdx, sec);
	}

	//移动乐器片段到目标轨道分径的指定时间点
	void Tau::MoveSelectedInstFragments(vector<int>& dstTracks, vector<int>& dstBranchIdx, vector<float>& secs)
	{
		editor->MoveSelectedInstFragments(dstTracks, dstBranchIdx, secs);
	}

	//设置轨道事件演奏方式
	void Tau::SetTrackPlayType(int trackIdx, MidiEventPlayType playType)
	{
		editor->SetTrackPlayType(trackIdx, playType);
	}


	// 设置midi对应轨道的乐器
	void Tau::SetMidiVirInstrument(int trackIdx,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		editor->SetVirInstrument(trackIdx, bankSelectMSB, bankSelectLSB, instrumentNum);
	}


	//为midi文件设置打击乐号
	void Tau::SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		editor->SetMidiBeatVirInstrument(bankSelectMSB, bankSelectLSB, instrumentNum);
	}


	void Tau::ResetVirInstruments()
	{
		editor->ResetVirInstruments();
	}

	/// <summary>
	/// 录制所有乐器弹奏为midi
	/// </summary>
	/// <param name="bpm">录制的BPM</param>
	 /// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
	void Tau::RecordMidi(float bpm, float tickForQuarterNote)
	{
		if (!isOpened)
			return;

		TauLock(this);
		synther->RecordMidi();
	}

	/// <summary>
	/// 录制指定乐器弹奏为midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	/// <param name="bpm">录制的BPM</param>
	/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
	void Tau::RecordMidi(VirInstrument* virInst, float bpm, float tickForQuarterNote)
	{
		if (!isOpened)
			return;

		TauLock(this);
		synther->RecordMidi(virInst);
	}

	/// <summary>
	/// 停止所有乐器当前midi录制
	/// </summary>
	void Tau::StopRecordMidi()
	{
		if (!isOpened)
			return;

		TauLock(this);
		synther->StopRecordMidi();
	}

	/// <summary>
	/// 停止录制指定乐器弹奏midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	void Tau::StopRecordMidi(VirInstrument* virInst)
	{
		if (!isOpened)
			return;

		TauLock(this);
		synther->StopRecordMidi(virInst);
	}


	// 生成所有乐器已录制的midi到midiflie object中
	MidiFile* Tau::CreateRecordMidiFile()
	{
		return CreateRecordMidiFile(nullptr, 0);
	}

	// 根据给定的乐器组，生成它们已录制的midi到midiflie object中
	MidiFile* Tau::CreateRecordMidiFile(VirInstrument** virInsts, int size)
	{
		MidiFile* midiFile = new MidiFile();
		midiFile->SetFormat(MidiFileFormat::SyncTracks);
		midiFile->SetTickForQuarterNote(recordMidiTickForQuarterNote);

		//
		vector<RecordTempo> recordTempos;
		RecordTempo tempo;
		tempo.BPM = recordMidiBPM;
		tempo.sec = 0;
		recordTempos.push_back(tempo);

		//
		MidiTrack* globalTrack = new MidiTrack();
		globalTrack->CreateTempoEvents(recordMidiTickForQuarterNote, recordTempos);
		midiFile->AddMidiTrack(globalTrack);

		//
		vector<MidiTrack*>* midiTracks;
		TauLock(this);
		midiTracks = synther->TakeRecordMidiTracks(virInsts, size, recordMidiTickForQuarterNote, &recordTempos);
		if (midiTracks != nullptr)
		{
			for (int j = 0; j < midiTracks->size(); j++)
			{
				(*midiTracks)[j]->SetMidiEventsChannel(j % 16);
				midiFile->AddMidiTrack((*midiTracks)[j]);
			}

			DEL(midiTracks);
		}

		return midiFile;
	}

	//保存midiFile到文件
	void Tau::SaveMidiFileToDisk(MidiFile* midiFile, string saveFilePath)
	{
		if (midiFile == nullptr)
			return;

		midiFile->CreateMidiFormatMemData();
		midiFile->SaveMidiFormatMemDataToDist(saveFilePath);
	}

}
