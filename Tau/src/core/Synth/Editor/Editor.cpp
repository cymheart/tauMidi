#include"Editor.h"
#include"Track.h"
#include"MidiEditor.h"
#include"Synth/Tau.h"
#include"Synth/Synther/MidiEditorSynther.h"
#include"Synth/Channel.h"
#include"Synth/KeySounder.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include"Midi/MidiFile.h"
#include"Synth/VirInstrument.h"
#include"Synth/Preset.h"


namespace tau
{
	Editor::Editor(Tau* tau)
	{
		this->tau = tau;
		computedPerSyntherLimitTrackCount = tau->perSyntherLimitTrackCount;

		//
		isStepPlayMode = false;
		isWait = false;
	}

	Editor::~Editor()
	{
		Remove();
	}


	//载入
	void Editor::Load(string& midiFilePath)
	{
		Remove();

		//
		MidiFile* midiFile = new MidiFile();
		midiFile->SetTrackChannelMergeMode(tau->midiFileMergeMode);
		midiFile->Parse(midiFilePath);

		list<MidiEvent*>* golbalEvents = midiFile->GetGolbalEventList();

		midiMarkerList.SetTickForQuarterNote(midiFile->GetTickForQuarterNote());
		midiMarkerList.AppendFormMidiEvents(*golbalEvents);

		AddMidiTracks(*midiFile->GetTrackList());

		midiFile->ClearMidiTrackList();
		DEL(midiFile);

		//打印工程信息
		PrintProjectInfo();
	}

	void Editor::PrintProjectInfo()
	{
		printf("\n=================================== \n");
		printf("synther总数:%d \n", tau->syntherCount);
		printf("midi文件总时长:%.2f秒 \n", endSec);
		printf("midi轨道总数:%zd \n", tracks.size());
	}

	//移除
	void Editor::Remove()
	{
		if (tau->syntherCount != 0)
		{
			int openedCount = 0;
			for (int i = 0; i < tau->syntherCount; i++)
			{
				if (tau->midiEditorSynthers[i]->IsOpened())
					openedCount++;
			}

			if (openedCount > 0)
			{
				waitSem.reset(openedCount - 1);
				for (int i = 0; i < tau->syntherCount; i++)
				{
					if (tau->midiEditorSynthers[i]->IsOpened())
						tau->midiEditorSynthers[i]->RemoveTask(&waitSem);
				}

				waitSem.wait();
			}
		}

		DelEmptyTrackRealtimeSynther();

		//等所有发音结束后删除轨道事件
		for (int i = 0; i < tracks.size(); i++)
			DEL(tracks[i]);

		computedPerSyntherLimitTrackCount = tau->perSyntherLimitTrackCount;
		tracks.clear();
		midiMarkerList.Clear();
	}


	//播放
	void Editor::Play()
	{
		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->PlayTask(&waitSem);

		waitSem.wait();
	}

	//停止
	void Editor::Stop()
	{
		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->StopTask(&waitSem);

		waitSem.wait();

	}

	//暂停
	void Editor::Pause()
	{
		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->PauseTask(&waitSem);

		waitSem.wait();
	}


	//进入到步进播放模式
	void Editor::EnterStepPlayMode()
	{
		if (isWaitPlayMode) {
			printf("当前模式为:等待播放模式，需要离开等待播放模式! \n");
			return;
		}

		isStepPlayMode = true;
		isWait = false;
		//		
	}

	//离开步进播放模式
	void Editor::LeaveStepPlayMode()
	{
		if (isWaitPlayMode)
			return;

		isStepPlayMode = false;
	}

	//进入到等待播放模式
	void Editor::EnterWaitPlayMode()
	{
		if (isStepPlayMode) {
			printf("当前模式为:步进播放模式，需要离开步进播放模式! \n");
			return;
		}

		isWaitPlayMode = true;
		isWait = false;
		memset(onkey, 0, sizeof(int) * 128);
		memset(needOnkey, 0, sizeof(int) * 128);
		memset(needOffkey, 0, sizeof(int) * 128);
		needOnKeyCount = 0;
		needOffKeyCount = 0;

		waitSem.reset(tau->syntherCount - 1);
		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->EnterWaitPlayModeTask(&waitSem);

		waitSem.wait();

	}

	//离开等待播放模式
	void Editor::LeaveWaitPlayMode()
	{
		if (isStepPlayMode)
			return;

		isWaitPlayMode = false;

		waitSem.reset(tau->syntherCount - 1);
		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->LeaveWaitPlayModeTask(&waitSem);

		waitSem.wait();
	}


	//移动到指定时间点
	void Editor::Runto(double sec)
	{
		//如果是等待播放模式，将清空等待播放模式的数据
		if (isWaitPlayMode) {
			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			needOnKeyCount = 0;
			needOffKeyCount = 0;
			isWait = false;
		}

		waitSem.reset(tau->syntherCount - 1);
		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->RuntoTask(&waitSem, sec);

		waitSem.wait();
	}

	//等待(区别于暂停，等待相当于在原始位置播放)
	void Editor::Wait()
	{
		if (isStepPlayMode || isWaitPlayMode)
			return;

		isWait = true;
	}

	//继续，相对于等待命令
	void Editor::Continue()
	{
		if (isStepPlayMode || isWaitPlayMode)
			return;

		isWait = false;
	}

	//需要按键信号
	void Editor::NeedOnKeySignal(int key)
	{
		waitOnKeyLock.lock();

		needOnkey[key]++;

		if (needOnkey[key] > 0)
			needOnKeyCount++;

		if (needOnKeyCount > 0)
		{
			isWait = true;
			printf("等待按键:%d \n", key);
		}

		waitOnKeyLock.unlock();
	}

	//需要松开按键信号
	void Editor::NeedOffKeySignal(int key)
	{
		waitOnKeyLock.lock();

		if (onkey[key] == 0) {
			waitOnKeyLock.unlock();
			return;
		}

		needOffkey[key]++;

		if (needOffkey[key] > 0)
			needOffKeyCount++;

		if (needOffKeyCount > 0) {
			isWait = true;
			printf("等待松开键:%d \n", key);
		}


		waitOnKeyLock.unlock();
	}

	//按键信号
	void Editor::OnKeySignal(int key)
	{
		waitOnKeyLock.lock();

		needOnkey[key]--;
		onkey[key]++;

		if (needOnkey[key] >= 0)
			needOnKeyCount--;

		if (needOnKeyCount <= 0 && needOffKeyCount <= 0) {
			isWait = false;
			printf("按下按键,继续:%d \n", key);
		}
		else
		{
			printf("按下按键:%d \n", key);
		}

		waitOnKeyLock.unlock();
	}

	//松开按键信号
	void Editor::OffKeySignal(int key)
	{
		waitOnKeyLock.lock();

		if (onkey[key] > 0 && needOffkey[key] == 0 && needOnkey[key] == 0)
		{
			needOnkey[key]++;
			needOnKeyCount++;
			onkey[key]--;

			if (needOnKeyCount > 0) {
				isWait = true;
				printf("等待按键:%d \n", key);
			}

			waitOnKeyLock.unlock();
			return;
		}


		if (needOffkey[key] == 0)
		{
			if (onkey[key] > 0)
				onkey[key]--;

			waitOnKeyLock.unlock();
			return;
		}

		needOffkey[key]--;

		if (onkey[key] > 0)
			onkey[key]--;

		if (needOffkey[key] >= 0)
			needOffKeyCount--;

		if (needOnKeyCount <= 0 && needOffKeyCount <= 0) {
			isWait = false;
			printf("松开按键，继续:%d \n", key);
		}
		else
		{
			printf("松开按键:%d \n", key);
		}

		waitOnKeyLock.unlock();
	}


	// 设置播放时间点
	void Editor::Goto(double sec)
	{
		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->GotoTask(&waitSem, sec);

		waitSem.wait();
	}


	//获取状态
	EditorState Editor::GetState()
	{
		MidiEditor* midiEditor = tau->midiEditorSynthers[0]->GetMidiEditor();
		if (midiEditor == nullptr)
			return EditorState::STOP;

		return tau->midiEditorSynthers[0]->GetStateTask();
	}

	//获取当前播放时间点
	double Editor::GetPlaySec()
	{
		MidiEditor* midiEditor = tau->midiEditorSynthers[0]->GetMidiEditor();
		if (midiEditor == nullptr)
			return 0;

		return midiEditor->GetPlaySec();
	}


	// 设定速度
	void Editor::SetSpeed(float speed_)
	{
		speed = speed_;

		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
		{
			tau->midiEditorSynthers[i]->SetSpeedTask(&waitSem, speed);
		}

		waitSem.wait();
	}

	// 禁止播放指定编号的轨道
	void Editor::DisableTrack(int trackIdx)
	{
		if (trackIdx < 0 || trackIdx >= tracks.size())
			return;

		vector<int> trackIdxs;
		trackIdxs.push_back(trackIdx);
		DisableTracks(trackIdxs);
	}

	// 禁止播放指定编号的轨道
	void Editor::DisableTracks(vector<int>& trackIdxs)
	{
		waitSem.reset(trackIdxs.size() - 1);
		MidiEditorSynther* synther;
		for (int i = 0; i < trackIdxs.size(); i++)
		{
			synther = tracks[trackIdxs[i]]->GetMidiEditor()->GetSynther();
			synther->DisableTrackTask(&waitSem, tracks[trackIdxs[i]]);
		}
		waitSem.wait();
	}

	// 禁止播放所有轨道
	void Editor::DisableAllTrack()
	{
		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->DisableAllTrackTask(&waitSem);

		waitSem.wait();
	}

	// 启用播放指定编号的轨道
	void Editor::EnableTrack(int trackIdx)
	{
		if (trackIdx < 0 || trackIdx >= tracks.size())
			return;

		vector<int> trackIdxs;
		trackIdxs.push_back(trackIdx);
		EnableTracks(trackIdxs);
	}

	// 启用播放指定编号的轨道
	void Editor::EnableTracks(vector<int>& trackIdxs)
	{
		waitSem.reset(trackIdxs.size() - 1);
		MidiEditorSynther* synther;
		for (int i = 0; i < trackIdxs.size(); i++)
		{
			synther = tracks[trackIdxs[i]]->GetMidiEditor()->GetSynther();
			synther->EnableTrackTask(&waitSem, tracks[trackIdxs[i]]);
		}
		waitSem.wait();
	}

	// 启用播放所有轨道
	void Editor::EnableAllTrack()
	{
		waitSem.reset(tau->syntherCount - 1);

		for (int i = 0; i < tau->syntherCount; i++)
			tau->midiEditorSynthers[i]->EnableAllTrackTask(&waitSem);

		waitSem.wait();
	}


	// 禁止播放指定编号通道
	void Editor::DisableChannel(int channelIdx)
	{
		vector<MidiEditorSynther*> synhter;
		for (int i = 0; i < tracks.size(); i++)
		{
			if (tracks[i]->GetChannelNum() == channelIdx)
				syntherSet.insert(tracks[i]->GetMidiEditor()->GetSynther());
		}

		waitSem.reset(syntherSet.size() - 1);
		for (auto it = syntherSet.begin(); it != syntherSet.end(); ++it) {
			(*it)->DisableChannelTask(&waitSem, channelIdx);
		}
		waitSem.wait();

		syntherSet.clear();
	}


	// 启用播放指定编号通道
	void Editor::EnableChannel(int channelIdx)
	{
		vector<MidiEditorSynther*> synhter;
		for (int i = 0; i < tracks.size(); i++)
		{
			if (tracks[i]->GetChannelNum() == channelIdx)
				syntherSet.insert(tracks[i]->GetMidiEditor()->GetSynther());
		}

		waitSem.reset(syntherSet.size() - 1);
		for (auto it = syntherSet.begin(); it != syntherSet.end(); ++it) {
			(*it)->EnableChannelTask(&waitSem, channelIdx);
		}
		waitSem.wait();

		syntherSet.clear();
	}

	//设置轨道事件演奏方式
	void Editor::SetTrackPlayType(int trackIdx, MidiEventPlayType playType)
	{
		waitSem.reset(0);
		MidiEditorSynther* synther = tracks[trackIdx]->GetMidiEditor()->GetSynther();
		synther->SetTrackPlayTypeTask(&waitSem, tracks[trackIdx], playType);
		waitSem.wait();
	}

	// 设置对应轨道的乐器
	void Editor::SetVirInstrument(int trackIdx,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		MidiEditorSynther* synther = tracks[trackIdx]->GetMidiEditor()->GetSynther();
		waitSem.reset(0);
		synther->SetVirInstrumentTask(&waitSem, tracks[trackIdx], bankSelectMSB, bankSelectLSB, instrumentNum);
		waitSem.wait();
	}


	//设置打击乐号
	void Editor::SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		for (int i = 0; i < tau->syntherCount; i++)
		{
			tau->midiEditorSynthers[i]->SetBeatVirInstrumentTask(bankSelectMSB, bankSelectLSB, instrumentNum);
		}
	}


	//删除空轨实时RealtimeSynther
	void Editor::DelEmptyTrackRealtimeSynther()
	{
		for (int i = 1; i < tau->syntherCount; i++)
		{
			MidiEditor* midiEditor = tau->midiEditorSynthers[i]->GetMidiEditor();
			if (midiEditor == nullptr || midiEditor->GetTrackCount() == 0)
				DEL(tau->midiEditorSynthers[i]);
		}
		tau->syntherCount = 1;
	}

	//增加轨道，来自于MidiTrackList
	void Editor::AddMidiTracks(vector<MidiTrack*>& midiTracks,
		int start, int end, Track* dstFirstTrack, int dstFirstBranchIdx, float sec)
	{
		vector<InstFragment*> instFragments;
		vector<Track*> dstTracks;
		vector<float> secs;
		if (sec < 0) { sec = 0; }
		secs.push_back(sec);

		if (start < 0 || end < 0) {
			start = 0;
			end = midiTracks.size() - 1;
		}

		InstFragment* instFrag;
		for (int i = start; i <= end; i++)
		{
			instFrag = new InstFragment();
			instFrag->AddMidiEvents(*midiTracks[i]->GetEventList());
			instFragments.push_back(instFrag);
		}

		int startPos = tracks.size();
		if (dstFirstTrack == nullptr) {

			int trackIdx = tracks.size();
			NewTracks(instFragments.size());

			//给轨道设置默认乐器
			for (int i = start; i <= end; i++)
			{
				tracks[trackIdx]->SetChannelNum(midiTracks[i]->GetChannelNum());

				auto program = midiTracks[i]->GetDefaultProgramChangeEvent();
				if (midiTracks[i]->GetChannelNum() == 9) {

					SetVirInstrument(trackIdx, 128, 0, 0);
				}
				else if (program == nullptr) {
					SetVirInstrument(trackIdx, 0, 0, 0);
				}
				else {
					SetVirInstrument(trackIdx, 0, 0, program->value);
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
					dstTracks.push_back(tracks[i]);
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
	void Editor::AddMarkers(list<MidiEvent*>& midiEvents)
	{
		midiMarkerList.AppendFormMidiEvents(midiEvents);

		waitSem.reset(tau->syntherCount - 1);
		for (int i = 0; i < tau->syntherCount; i++)
		{
			tau->midiEditorSynthers[i]->SetMarkerListTask(&waitSem, &midiMarkerList);
		}
		waitSem.wait();
	}


	//新建轨道
	void Editor::NewTracks(int count)
	{
		MidiEditorSynther* synther;
		for (int i = 0; i < tau->syntherCount; i++)
		{
			count = _NewTracks(tau->midiEditorSynthers[i], count);
			if (count == 0)
				break;
		}

		if (count != 0 && tau->syntherCount >= tau->limitSyntherCount)
			count = ResetTrackCountNewTracks(count);

		while (count != 0)
		{
			synther = tau->CreateMidiEditorSynther();
			count = _NewTracks(synther, count);

			if (count != 0 && tau->syntherCount >= tau->limitSyntherCount)
				count = ResetTrackCountNewTracks(count);
		}
	}

	int Editor::ResetTrackCountNewTracks(int count)
	{
		int needTrackCount = count + tracks.size();
		computedPerSyntherLimitTrackCount = needTrackCount / tau->syntherCount;
		if (needTrackCount % tau->syntherCount != 0)
			computedPerSyntherLimitTrackCount++;

		for (int i = 0; i < tau->syntherCount; i++)
		{
			count = _NewTracks(tau->midiEditorSynthers[i], count);
			if (count == 0)
				break;
		}

		return count;
	}

	int Editor::_NewTracks(MidiEditorSynther* synther, int count)
	{
		bool isNewMidiEditor = false;
		MidiEditor* midiEditor = synther->GetMidiEditor();
		if (midiEditor == nullptr) {
			midiEditor = synther->CreateMidiEditor();
			isNewMidiEditor = true;
		}

		if (midiEditor->GetTrackCount() < computedPerSyntherLimitTrackCount)
		{
			int n = min(count, computedPerSyntherLimitTrackCount - midiEditor->GetTrackCount());
			count -= n;
			waitSem.reset(n - 1);
			for (int i = 0; i < n; i++) {
				synther->NewTrackTask(&waitSem);
			}
			waitSem.wait();

			vector<Track*>& tmpTracks = midiEditor->tempTracks;
			for (int i = 0; i < tmpTracks.size(); i++)
				tracks.push_back(tmpTracks[i]);
			tmpTracks.clear();
		}

		return count;
	}


	//删除轨道
	void Editor::DeleteTracks(vector<Track*>& delTracks)
	{
		for (int i = 0; i < delTracks.size(); i++)
		{
			vector<Track*>::iterator iVector = std::find(tracks.begin(), tracks.end(), delTracks[i]);
			if (iVector != tracks.end())
				tracks.erase(iVector);
		}

		waitSem.reset(delTracks.size() - 1);
		for (int i = 0; i < delTracks.size(); i++)
		{
			MidiEditorSynther* synther = delTracks[i]->GetMidiEditor()->GetSynther();
			synther->DeleteTrackTask(&waitSem, delTracks[i]);
		}
		waitSem.wait();

		DelEmptyTrackRealtimeSynther();

		//重新给每个MidiEditor设置相同的最大结束时间
		ComputeMidiEditorMaxSec();
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
		return tracks[trackIdx]->GetInstFragment(branchIdx, instFragIdx);
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
		vector<InstFragment*> instFragments;
		for (int i = 0; i < selectedInstFrags.size(); i++)
		{
			InstFragment* instFrag = GetInstFragment(
				selectedInstFrags[i].trackIdx,
				selectedInstFrags[i].branchIdx,
				selectedInstFrags[i].instFragmentIdx);

			if (instFrag != nullptr)
				instFragments.push_back(instFrag);
		}

		vector<Track*> dstTrackPtrs;
		for (int i = 0; i < dstTracks.size(); i++)
		{
			dstTrackPtrs.push_back(tracks[dstTracks[i]]);
		}

		MoveInstFragments(instFragments, dstTrackPtrs, dstBranchIdxs, secs);
	}


	//移动乐器片段到目标轨道的指定时间点
	void Editor::MoveInstFragments(
		vector<InstFragment*>& instFragments,
		vector<Track*>& dstTracks, vector<int>& dstBranchIdxs, vector<float>& secs)
	{
		//1.数据组合缓存到list中
		orgList.clear();
		for (int i = 0; i < instFragments.size(); i++)
		{
			InstFragmentToTrackInfo data;
			data.instFragment = instFragments[i];
			data.track = dstTracks[min(i, dstTracks.size() - 1)];
			data.branchIdx = dstBranchIdxs[min(i, dstBranchIdxs.size() - 1)];
			data.sec = secs[min(i, secs.size() - 1)];
			orgList.push_back(data);
		}

		//2.挑选出synther1 == nullptr ||  synther2 == nullptr || synther1 == synther2的数据到dataGroup中，
		//同时把这些数据从orgList删除
		//因为使用同一个synther，所以处理会在同一线程任务中，可以直接一次使用MoveInstFragmentTask，
		//而不需要先从不同的synther移除片段
		dataGroup.clear();
		MidiEditorSynther* synther2 = nullptr;
		MidiEditorSynther* synther1 = nullptr;
		InstFragment* frag;
		Track* track;
		list<InstFragmentToTrackInfo>::iterator it = orgList.begin();
		for (; it != orgList.end(); )
		{
			InstFragmentToTrackInfo& a = *it;
			frag = a.instFragment;
			track = a.track;

			synther1 = nullptr;
			synther2 = nullptr;

			Track* orgTrack = frag->GetTrack();
			if (orgTrack != nullptr)
				synther1 = orgTrack->GetMidiEditor()->GetSynther();

			if (track != nullptr)
				synther2 = track->GetMidiEditor()->GetSynther();

			if (synther1 == nullptr || synther2 == nullptr || synther1 == synther2) {

				if (synther1 != nullptr) { dataGroup[synther1].push_back(a); }
				else { dataGroup[synther2].push_back(a); }
				it = orgList.erase(it);
			}
			else {
				it++;
			}
		}

		//3.处理dataGroup中synther1 == nullptr ||  synther2 == nullptr || synther1 == synther2的数据，
		//因为使用同一个synther，所以处理会在同一线程任务中，可以直接一次使用MoveInstFragmentTask，
		//而不需要先从不同的synther移除片段
		for (auto iter = dataGroup.begin(); iter != dataGroup.end(); ++iter)
		{
			vector<InstFragmentToTrackInfo>& a = iter->second;
			MidiEditorSynther* synther = iter->first;

			waitSem.reset(a.size() - 1);
			for (int i = 0; i < a.size(); i++)
			{
				synther->MoveInstFragmentTask(&waitSem, a[i].instFragment, a[i].track, a[i].branchIdx, a[i].sec);

				//收集被修改的的轨道
				if (a[i].track != nullptr)
					modifyTrackMap[synther].insert(a[i].track);

				if (a[i].instFragment->GetTrack() != nullptr)
					modifyTrackMap[synther].insert(a[i].instFragment->GetTrack());
			}
			waitSem.wait();
		}


		//4.挑选出在同一synther中的片段到dataGroup中
		waitSem.reset(orgList.size() - 1);
		for (it = orgList.begin(); it != orgList.end(); it++)
		{
			InstFragmentToTrackInfo& a = *it;
			frag = a.instFragment;
			synther1 = frag->GetTrack()->GetMidiEditor()->GetSynther();
			synther1->RemoveInstFragmentTask(&waitSem, frag);

			//收集被修改的的轨道
			modifyTrackMap[synther1].insert(frag->GetTrack());
		}
		waitSem.wait();


		//5.挑选出在同一synther中的track到dataGroup中,因为步骤4，5，已经移除了片段，这些片段现在是没有被线程使用的，
		//此时可以直接分组到目标轨道组中，进行一次性MoveInstFragmentTask
		dataGroup.clear();
		it = orgList.begin();
		for (; it != orgList.end(); it++)
		{
			InstFragmentToTrackInfo& a = *it;
			Track* track = a.track;
			synther2 = nullptr;
			if (track != nullptr)
				synther2 = track->GetMidiEditor()->GetSynther();
			dataGroup[synther2].push_back(a);
		}

		for (auto iter = dataGroup.begin(); iter != dataGroup.end(); ++iter)
		{
			vector<InstFragmentToTrackInfo>& a = iter->second;
			MidiEditorSynther* synther = iter->first;

			waitSem.reset(a.size() - 1);
			for (int i = 0; i < a.size(); i++)
			{
				synther->MoveInstFragmentTask(&waitSem, a[i].instFragment, a[i].track, a[i].branchIdx, a[i].sec);

				//收集被修改的的轨道
				modifyTrackMap[synther].insert(a[i].track);
			}
			waitSem.wait();
		}

		//6.计算被修改轨道的所有事件时间，并重新计算每个MidiEditor的结束时间
		int n = modifyTrackMap.size();
		MidiEditorSynther* synther;
		auto beg = modifyTrackMap.begin();
		auto end = modifyTrackMap.end();

		for (auto iter = beg; iter != end; ++iter)
			n += iter->second.size();

		waitSem.reset(n - 1);
		for (auto iter = beg; iter != end; ++iter)
		{
			synther = iter->first;
			unordered_set<Track*>& trackset = iter->second;
			for (auto it = trackset.begin(); it != trackset.end(); ++it) {
				synther->ComputeTrackEventsTimeTask(&waitSem, *it);
			}
		}

		for (auto iter = beg; iter != end; ++iter)
		{
			synther = iter->first;
			synther->ComputeEndSecTask(&waitSem);
		}

		waitSem.wait();


		//7.重新给每个MidiEditor设置相同的最大结束时间
		ComputeMidiEditorMaxSec();

		//
		orgList.clear();
		dataGroup.clear();
		modifyTrackMap.clear();


		//如果是等待播放模式，将清空等待播放模式的数据
		if (isWaitPlayMode) {
			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			needOnKeyCount = 0;
			needOffKeyCount = 0;
			isWait = false;
		}

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


	//重新给每个MidiEditor设置相同的最大结束时间
	void Editor::ComputeMidiEditorMaxSec()
	{
		MidiEditor* midiEditor;
		endSec = 0;
		for (int i = 0; i < tau->syntherCount; i++)
		{
			midiEditor = tau->midiEditorSynthers[i]->GetMidiEditor();
			if (midiEditor->GetEndSec() > endSec)
				endSec = midiEditor->GetEndSec();
		}

		waitSem.reset(tau->syntherCount - 1);
		for (int i = 0; i < tau->syntherCount; i++)
		{
			tau->midiEditorSynthers[i]->SetEndSecTask(&waitSem, endSec);
		}
		waitSem.wait();
	}
}
