#include"Editor.h"
#include"Track.h"
#include"MidiEditor.h"
#include"Synth/Tau.h"
#include"Synth/Synther/Synther.h"
#include"Synth/Channel.h"
#include"Synth/KeySounder.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include"Midi/MidiFile.h"
#include"Synth/VirInstrument.h"
#include"Synth/Preset.h"
#include"MeasureInfo.h"
#include"Synth/SyntherEvent.h"


namespace tau
{
	Editor::Editor(Tau* tau)
	{
		this->tau = tau;
		mainSynther = tau->mainSynther;
		midiEditor = mainSynther->midiEditor;

		//
		measureInfo = new MeasureInfo();

		for (int i = 0; i < 128; i++)
			excludeNeedWaitKey[i] = false;

		ResetParams();
	}

	Editor::~Editor()
	{
		Remove();

		DEL(measureInfo);

		if (releaseCallBack)
			releaseCallBack(this);
	}

	void Editor::ResetParams()
	{
		isWait = false;
		curtPlaySec = initStartPlaySec;
		playState = EditorState::STOP;
		cacheState = CacheState::CacheStop;
	}


	//载入
	//在非阻塞模式isWaitLoadCompleted = false下，
	//Load完成前不能有任何与之相冲突的调用（play(), pasue(),stop() goto()等,因为这些函数出于效率考虑没有加互斥锁）
	//在调用play(), pasue(),stop() goto()等操作时，需要调用IsLoadCompleted()进行确认判断
	void Editor::Load(string& midiFilePath, bool isWaitLoadCompleted)
	{
		loadingMidiFilelocker.lock();

		while (loadMidiFileState == 1) {
			midiFile->StopParse();
			isStopLoad = true;
			loadingMidiFilelocker.unlock();
			loadingMidiFileWaitSem.wait();
			loadingMidiFilelocker.lock();
		}

		DEL(midiFile);
		RemoveCore();

		if (loadStartCallBack != nullptr)
			loadStartCallBack(this);

		isFullParsedMidiFile = false;
		isStopLoad = false;
		loadMidiFileState = 1;
		this->midiFilePath = midiFilePath;

		//
		int start = midiFilePath.find_last_of('\\');
		int end = midiFilePath.find_last_of('.');
		midiName = midiFilePath.substr(start + 1, end - start - 1);

		//
		midiFile = new MidiFile();
		midiFile->SetEnableMidiEventCountOptimize(tau->enableMidiEventCountOptimize);
		midiFile->SetKeepSameStartTickNoteOnEventsCount(tau->midiKeepSameTimeNoteOnCount);
		midiFile->SetEnableCopySameChannelControlEvents(tau->enableCopySameChannelControlEvents);
		midiFile->SetEnableParseLimitTime(tau->isEnableMidiEventParseLimitTime, tau->midiEventLimitParseSec);

		//
		thread t(ReadMidiFileThread, this);
		t.detach();

		loadingMidiFilelocker.unlock();

		if (isWaitLoadCompleted)
			loadingMidiFileWaitSem.wait();

	}

	void Editor::ReadMidiFileThread(void* param)
	{
		Editor* editor = (Editor*)param;
		editor->ReadMidiFile();
	}

	void Editor::ReadMidiFile()
	{
		bool isParseCompleted = midiFile->Parse(midiFilePath);

		loadingMidiFilelocker.lock();
		if (isParseCompleted) {
			_Load();
			if (!isStopLoad) {
				if (loadCompletedCallBack != nullptr)
					loadCompletedCallBack(this);
				loadMidiFileState = 2;
			}
			else
				loadMidiFileState = 0;
		}
		else
			loadMidiFileState = 0;

		loadingMidiFilelocker.unlock();
		loadingMidiFileWaitSem.set();

	}

	//载入
	void Editor::_Load()
	{
		if (midiFile == nullptr)
			return;

		isFullParsedMidiFile = midiFile->IsFullParsed();

		//
		LinkedList<MidiEvent*>* golbalEvents = midiFile->GetGolbalEventList();
		if (golbalEvents == nullptr) {
			DEL(midiFile);
			loadMidiFileState = 0;
			return;
		}

		midiMarkerList.SetTickForQuarterNote(midiFile->GetTickForQuarterNote());
		midiMarkerList.AppendFromMidiEvents(*golbalEvents);


		AddMidiTracks(*midiFile->GetTrackList());

		midiFile->ClearMidiTrackList();
		DEL(midiFile);

		//打印工程信息
		PrintProjectInfo();
	}

	//被合并音符的最大时长
	void Editor::SetMergeSimpleSrcNoteLimitSec(float sec)
	{
		midiEditor->mergeSimpleSrcNoteLimitSec = sec;
	}

	//合并到目标音符的最大时长
	void Editor::SetMergeSimpleDestNoteLimitSec(float sec)
	{
		midiEditor->mergeSimpleDestNoteLimitSec = sec;
	}

	//获取被合并音符的最大时长
	float Editor::GetMergeSimpleSrcNoteLimitSec()
	{
		return midiEditor->mergeSimpleSrcNoteLimitSec;
	}

	//获取合并到目标音符的最大时长
	float Editor::GetMergeSimpleDestNoteLimitSec()
	{
		return midiEditor->mergeSimpleDestNoteLimitSec;
	}

	//设置简单模式下, 白色按键的数量
	void Editor::SetSimpleModePlayWhiteKeyCount(int count)
	{
		midiEditor->SetSimpleModePlayWhiteKeyCount(count);
	}

	//获取简单模式下, 白色按键的数量
	int Editor::GetSimpleModePlayWhiteKeyCount()
	{
		return midiEditor->simpleModePlayWhiteKeyCount;
	}


	//生成简单模式音符轨道
	void Editor::CreateSimpleModeTrack()
	{
		midiEditor->CreateSimpleModeTrack();
	}


	//获取简单模式轨道音符
	LinkedList<MidiEvent*>& Editor::GetSimpleModeTrackNotes()
	{
		return midiEditor->simpleModeTrackNotes;
	}

	//是否读取完成
	bool Editor::IsLoadCompleted()
	{
		int state;
		loadingMidiFilelocker.lock();
		state = loadMidiFileState;
		loadingMidiFilelocker.unlock();
		return state == 2 ? true : false;
	}

	void Editor::PrintProjectInfo()
	{
		TauLock(tau);
		printf("\n=================================== \n");
		printf("midi文件:%s \n", midiName.c_str());
		printf("总时长:%.2f秒 \n", endSec);
		printf("轨道总数:%zd \n", midiEditor->tracks.size());
	}

	//移除
	void Editor::Remove()
	{
		loadingMidiFilelocker.lock();
		while (loadMidiFileState == 1) {
			midiFile->StopParse();
			loadingMidiFilelocker.unlock();
			loadingMidiFileWaitSem.wait();
			loadingMidiFilelocker.lock();
		}

		DEL(midiFile);
		RemoveCore();
		midiFilePath.clear();
		loadingMidiFilelocker.unlock();

	}

	//移除核心
	void Editor::RemoveCore()
	{
		if (!tau->IsOpened())
			return;

		tau->lockMutex.lock();
		mainSynther->Remove();
		tau->lockMutex.unlock();

		mainSynther->waitSem.wait();

		//等所有发音结束后删除轨道事件
		for (int i = 0; i < midiEditor->tracks.size(); i++)
			DEL(midiEditor->tracks[i]);

		midiEditor->tracks.clear();
		midiMarkerList.Clear();
		measureInfo->Clear();
	}


	vector<Track*>& Editor::GetTracks()
	{
		return midiEditor->tracks;
	}

	// 按下按键
	void Editor::OnKey(int key, float velocity, int trackIdx, int id)
	{
		TauLock(tau);
		OnKey(key, velocity, midiEditor->tracks[trackIdx], id);
	}

	// 释放按键 
	void Editor::OffKey(int key, float velocity, int trackIdx, int id)
	{
		TauLock(tau);
		OffKey(key, velocity, midiEditor->tracks[trackIdx], id);
	}

	// 释放指定轨道的所有按键 
	void Editor::OffAllKeys(int trackIdx)
	{
		TauLock(tau);
		mainSynther->OffAllKeys(midiEditor->tracks[trackIdx]->GetChannel());
	}

	// 释放所有按键 
	void Editor::OffAllKeys()
	{
		TauLock(tau);
		mainSynther->OffAllKeys();
	}

	// 按下按键
	void Editor::OnKey(int key, float velocity, Track* track, int id)
	{
		mainSynther->OnKey(key, velocity, track, id);
	}

	// 释放按键 
	void Editor::OffKey(int key, float velocity, Track* track, int id)
	{
		mainSynther->OffKey(key, velocity, track, id);
	}


	//播放
	void Editor::Play()
	{
		TauLock(tau);
		mainSynther->Play();
	}

	//停止
	void Editor::Stop()
	{
		if (loadMidiFileState == 1)
			return;

		TauLock(tau);
		mainSynther->Stop();
	}

	//暂停
	void Editor::Pause()
	{
		TauLock(tau);
		mainSynther->Pause();
	}


	// 设置播放时间点
	void Editor::Goto(double sec)
	{
		TauLock(tau);

		//如果是等待播放模式，将清空等待播放模式的数据
		if (playMode == EditorPlayMode::Wait) {
			for (int i = 0; i < 128; i++)
			{
				needOnKeyTrack[i].clear();
				needOnKeyVelocity[i].clear();
			}
			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			memset(needWaitKey, 0, sizeof(bool) * 128);

			onKeyCount = 0;
			needOnKeyCount = 0;
			needOffKeyCount = 0;
			isWait = false;
		}

		mainSynther->Goto(sec);

	}


	//获取当前bpm
	float Editor::GetCurtBPM()
	{
		Tempo* tempo = midiMarkerList.GetTempo(GetPlaySec());
		return tempo->GetBPM();
	}

	//根据指定秒数获取tick数
	uint32_t Editor::GetSecTickCount(double sec)
	{
		Tempo* tempo = midiMarkerList.GetTempo(sec);
		return tempo->GetTickCount(sec);
	}

	//根据指定tick数秒数获取时间点
	double Editor::GetTickSec(uint32_t tick)
	{
		Tempo* tempo = midiMarkerList.GetTempo((int)tick);
		return tempo->GetTickSec(tick);
	}


	//获取轨道乐器
	vector<VirInstrument*>& Editor::GetTrackInst(int idx)
	{
		TauLock(tau);
		return midiEditor->tracks[idx]->GetChannel()->GetVirInstruments();
	}

	//获取播放状态
	EditorState Editor::GetPlayState()
	{
		return playState;
	}

	//获取缓存状态
	CacheState Editor::GetCacheState()
	{
		return cacheState;
	}

	//获取当前播放时间点
	double Editor::GetPlaySec()
	{
		return curtPlaySec;
	}

	// 设定速度
	void Editor::SetSpeed(float speed_)
	{
		TauLock(tau);
		speed = speed_;
		mainSynther->SetSpeed(speed);
	}

	// 禁止播放指定编号的轨道
	void Editor::DisableTrack(int trackIdx)
	{
		TauLock(tau);
		mainSynther->DisableTrack(midiEditor->tracks[trackIdx]);
	}

	// 禁止播放指定编号的轨道
	void Editor::DisableTracks(vector<int>& trackIdxs)
	{
		TauLock(tau);
		for (int i = 0; i < trackIdxs.size(); i++)
			mainSynther->DisableTrack(trackIdxs[i]);
	}

	// 禁止播放所有轨道
	void Editor::DisableAllTrack()
	{
		TauLock(tau);
		mainSynther->DisableTrack(nullptr);
	}

	// 启用播放指定编号的轨道
	void Editor::EnableTrack(int trackIdx)
	{
		TauLock(tau);
		mainSynther->EnableTrack(trackIdx);
	}

	// 启用播放指定编号的轨道
	void Editor::EnableTracks(vector<int>& trackIdxs)
	{
		TauLock(tau);
		for (int i = 0; i < trackIdxs.size(); i++)
			mainSynther->EnableTrack(trackIdxs[i]);
	}

	// 启用播放所有轨道
	void Editor::EnableAllTrack()
	{
		TauLock(tau);
		mainSynther->EnableTrack(nullptr);
	}


	// 禁止播放指定编号通道
	void Editor::DisableChannel(int channelIdx)
	{
		TauLock(tau);
		mainSynther->DisableChannel(channelIdx);
	}


	// 启用播放指定编号通道
	void Editor::EnableChannel(int channelIdx)
	{
		TauLock(tau);
		mainSynther->EnableChannel(channelIdx);
	}


	//获取采样流的频谱
	int Editor::GetSampleStreamFreqSpectrums(int channel, double* outLeft, double* outRight)
	{
		TauLock(tau);
		return mainSynther->GetSampleStreamFreqSpectrums(channel, outLeft, outRight);
	}

	// 设置对应轨道的乐器
	void Editor::SetVirInstrument(int trackIdx,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		TauLock(tau);
		mainSynther->SetVirInstrument(trackIdx, bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	//设置打击乐号
	void Editor::SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		TauLock(tau);
		mainSynther->SetBeatVirInstrument(bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	void Editor::ResetVirInstruments()
	{
		TauLock(tau);
		for (int i = 0; i < midiEditor->tracks.size(); i++)
		{
			int lsb = midiEditor->tracks[i]->GetChannel()->GetBankSelectLSB();
			int msb = midiEditor->tracks[i]->GetChannel()->GetBankSelectMSB();
			int num = midiEditor->tracks[i]->GetChannel()->GetProgramNum();
			vector<VirInstrument*>& insts = midiEditor->tracks[i]->GetChannel()->GetVirInstruments();

			for (int i = 0; i < insts.size(); i++)
				mainSynther->SetVirInstrument(midiEditor->tracks[i], msb, lsb, num);
		}
	}


	//增加轨道，来自于MidiTrackList
	void Editor::AddMidiTracks(vector<MidiTrack*>& midiTracks,
		int start, int end, Track* dstFirstTrack, int dstFirstBranchIdx, float sec)
	{
		TauLock(tau);

		vector<Track*>& tracks = midiEditor->tracks;
		vector<InstFragment*> instFragments;
		vector<Track*> dstTracks;
		vector<float> secs;

		if (start < 0 || end < 0) {
			start = 0;
			end = midiTracks.size() - 1;
		}

		InstFragment* instFrag;
		for (int i = start; i <= end; i++)
		{
			instFrag = new InstFragment();
			instFrag->SetName(midiTracks[i]->GetTrackName(1));
			LinkedList<MidiEvent*>& eventList = *midiTracks[i]->GetEventList();
			instFrag->AddMidiEvents(eventList);
			eventList.Release();

			instFragments.push_back(instFrag);
			secs.push_back(0);
		}

		int startPos = tracks.size();
		if (dstFirstTrack == nullptr) {

			int trackIdx = tracks.size();

			for (int i = 0; i < instFragments.size(); i++)
				mainSynther->NewTrack();


			//给轨道设置默认乐器
			for (int i = start; i <= end; i++)
			{
				tracks[trackIdx]->SetName(midiTracks[i]->GetTrackName(1));
				tracks[trackIdx]->SetChannelNum(midiTracks[i]->GetChannelNum());

				auto program = midiTracks[i]->GetDefaultProgramChangeEvent();
				if (midiTracks[i]->GetChannelNum() == 9) {
					mainSynther->SetVirInstrument(trackIdx, 128, 0, 0);
				}
				else if (program == nullptr) {
					mainSynther->SetVirInstrument(trackIdx, 0, 0, 0);
				}
				else {
					mainSynther->SetVirInstrument(trackIdx, 0, 0, program->value);
				}

				trackIdx++;
			}

			int endSize = min(tracks.size(), startPos + instFragments.size());
			for (int i = startPos; i < endSize; i++)
				dstTracks.push_back(tracks[i]);

			MoveInstFragments(instFragments, dstTracks, secs);
		}
		else
		{
			vector<int> dstBranchIdxs;

			if (dstFirstBranchIdx >= dstFirstTrack->GetBranchCount())
				dstFirstBranchIdx = dstFirstTrack->GetBranchCount() - 1;

			bool isFind = false;
			int n = 0;
			for (int i = 0; i < tracks.size(); i++)
			{
				if (!isFind && tracks[i] != dstFirstTrack)
					continue;

				for (int j = dstFirstBranchIdx; j < tracks[i]->GetBranchCount(); j++)
				{
					dstTracks.push_back(tracks[j]);
					dstBranchIdxs.push_back(j);
					n++;

					if (n == instFragments.size())
					{
						MoveInstFragments(instFragments, dstTracks, dstBranchIdxs, secs);
						return;
					}
				}

				isFind = true;
				dstFirstBranchIdx = 0;
			}
		}
	}


	//增加标记，来自于midi事件列表中
	void Editor::AddMarkers(LinkedList<MidiEvent*>& midiEvents)
	{
		TauLock(tau);
		midiMarkerList.AppendFromMidiEvents(midiEvents);
		mainSynther->SetMarkerList(&midiMarkerList);
	}


	//新建轨道
	void Editor::NewTracks(int count)
	{
		TauLock(tau);
		for (int i = 0; i < count; i++)
			mainSynther->NewTrack();
	}

	//删除轨道
	void Editor::DeleteTracks(vector<Track*>& delTracks)
	{
		TauLock(tau);
		for (int i = 0; i < delTracks.size(); i++)
			mainSynther->DeleteTrack(delTracks[i]);

		//重新给每个MidiEditor设置相同的最大结束时间
		endSec = midiEditor->GetEndSec();
	}


	void Editor::SelectInstFragment(int trackIdx, int branchIdx, int instFragIdx)
	{
		if (selectInstFragMode == SelectInstFragMode::SingleSelect)
		{
			selectedInstFrags.clear();
		}
		else
		{
			for (int i = 0; i < selectedInstFrags.size(); i++)
			{
				if (selectedInstFrags[i].trackIdx == trackIdx &&
					selectedInstFrags[i].branchIdx == branchIdx &&
					selectedInstFrags[i].instFragmentIdx == instFragIdx)
					return;
			}
		}

		SelectInstFragmentInfo info = { trackIdx, branchIdx, instFragIdx };
		selectedInstFrags.push_back(info);
	}

	void Editor::UnSelectInstFragment(int trackIdx, int branchIdx, int instFragIdx)
	{

		for (int i = 0; i < selectedInstFrags.size(); i++)
		{
			if (selectedInstFrags[i].trackIdx == trackIdx &&
				selectedInstFrags[i].branchIdx == branchIdx &&
				selectedInstFrags[i].instFragmentIdx == instFragIdx)
			{
				selectedInstFrags.erase(selectedInstFrags.begin() + i);
				return;
			}
		}
	}

	InstFragment* Editor::GetInstFragment(int trackIdx, int branchIdx, int instFragIdx)
	{
		TauLock(tau);
		return midiEditor->tracks[trackIdx]->GetInstFragment(branchIdx, instFragIdx);
	}

	//移动乐器片段到目标轨道分径的指定时间点
	void Editor::MoveSelectedInstFragment(int dstTrack, int dstBranchIdx, float sec)
	{
		vector<int> dstTracks;
		vector<int> dstBranchIdxs;
		vector<float> secs;

		dstTracks.push_back(dstTrack);
		dstBranchIdxs.push_back(dstBranchIdx);
		secs.push_back(sec);

		MoveSelectedInstFragments(dstTracks, dstBranchIdxs, secs);
	}


	//移动乐器片段到目标轨道分径的指定时间点
	void Editor::MoveSelectedInstFragments(vector<int>& dstTracks, vector<int>& dstBranchIdxs, vector<float>& secs)
	{
		TauLock(tau);
		vector<InstFragment*> instFragments;
		for (int i = 0; i < selectedInstFrags.size(); i++)
		{
			InstFragment* instFrag = midiEditor->tracks[selectedInstFrags[i].trackIdx]->GetInstFragment(
				selectedInstFrags[i].branchIdx,
				selectedInstFrags[i].instFragmentIdx);

			if (instFrag != nullptr)
				instFragments.push_back(instFrag);
		}

		vector<Track*> dstTrackPtrs;
		for (int i = 0; i < dstTracks.size(); i++)
		{
			dstTrackPtrs.push_back(midiEditor->tracks[dstTracks[i]]);
		}

		MoveInstFragments(instFragments, dstTrackPtrs, dstBranchIdxs, secs);
	}

	//移动乐器片段到目标轨道的指定时间点
	void Editor::MoveInstFragments(vector<InstFragment*>& instFragments, vector<Track*>& dstTracks, vector<float>& secs)
	{
		vector<int> branchIdxVec;
		for (int i = 0; i < instFragments.size(); i++)
			branchIdxVec.push_back(instFragments[i]->GetBranchIdx());

		MoveInstFragments(instFragments, dstTracks, branchIdxVec, secs);
	}


	//移动乐器片段到指定时间点
	void Editor::MoveInstFragments(vector<InstFragment*>& instFragments, vector<float>& secs)
	{
		vector<Track*> trackVec;
		for (int i = 0; i < instFragments.size(); i++)
			trackVec.push_back(nullptr);

		MoveInstFragments(instFragments, trackVec, secs);
	}

	//移动乐器片段到目标轨道的指定时间点
	void Editor::MoveInstFragments(
		vector<InstFragment*>& instFragments,
		vector<Track*>& dstTracks, vector<int>& dstBranchIdxs, vector<float>& secs)
	{
		for (int i = 0; i < instFragments.size(); i++)
			mainSynther->MoveInstFragment(instFragments[i], dstTracks[i], dstBranchIdxs[i], secs[i]);


		for (int i = 0; i < dstTracks.size(); i++)
			mainSynther->ComputeTrackEventsTime(dstTracks[i]);


		for (int i = 0; i < instFragments.size(); i++) {
			if (instFragments[i]->GetTrack())
				mainSynther->ComputeTrackEventsTime(instFragments[i]->GetTrack());
		}

		mainSynther->ComputeEndSec();

		//重新获取最大结束时间
		endSec = midiEditor->GetEndSec();

		//生成小节信息
		measureInfo->Create(midiMarkerList, endSec);


		//如果是等待播放模式，将清空等待播放模式的数据
		if (playMode == EditorPlayMode::Wait) {
			for (int i = 0; i < 128; i++)
			{
				needOnKeyTrack[i].clear();
				needOnKeyVelocity[i].clear();
			}
			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			needOnKeyCount = 0;
			needOffKeyCount = 0;
			isWait = false;
		}


		//如果启用了缓存播放，将重新缓存
		mainSynther->ReCache();

	}


	//移动乐器片段到目标轨道的指定时间点
	void Editor::MoveInstFragment(InstFragment* instFragment, Track* dstTrack, float sec)
	{
		vector<InstFragment*> fragVec;
		vector<Track*> trackVec;
		vector<float> secVec;

		//
		fragVec.push_back(instFragment);
		trackVec.push_back(dstTrack);
		secVec.push_back(sec);

		//
		MoveInstFragments(fragVec, trackVec, secVec);
	}

	//移动乐器片段到指定时间点
	void Editor::MoveInstFragment(InstFragment* instFragment, float sec)
	{
		vector<InstFragment*> fragVec;
		vector<Track*> trackVec;
		vector<float> secVec;

		//
		fragVec.push_back(instFragment);
		trackVec.push_back(nullptr);
		secVec.push_back(sec);

		//
		MoveInstFragments(fragVec, trackVec, secVec);
	}

	int Editor::GetCurtNeedOnKeyTrackIdx()
	{
		TauLock(tau);
		if (curtNeedOnKeyTrack == nullptr)
			return -1;

		for (int i = 0; i < midiEditor->tracks.size(); i++)
		{
			if (midiEditor->tracks[i] == curtNeedOnKeyTrack)
				return i;
		}
		return -1;
	}
}
