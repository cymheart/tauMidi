#include"MidiEditor.h"
#include"Track.h"
#include"Synth/Tau.h"
#include"Synth/Synther/MidiEditorSynther.h"
#include"Synth/Channel.h"
#include"Synth/KeySounder.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include"Midi/MidiFile.h"
#include"Synth/VirInstrument.h"
#include"Synth/Preset.h"
#include"Synth/Editor/Editor.h"


namespace tau
{
	MidiEditor::MidiEditor(MidiEditorSynther* midiSynther)
	{
		this->midiSynther = midiSynther;
		tau = midiSynther->tau;
		editor = tau->editor;


		//初始化值
		SetCurtPlaySec(editor->GetPlaySec());
		SetState(editor->GetState());
		speed = editor->GetSpeed();
		playMode = editor->playMode;
		playType = editor->playType;

		midiMarkerList.Copy(editor->midiMarkerList);
	}

	MidiEditor::~MidiEditor()
	{
		midiMarkerList.Clear();
	}

	//设置标记
	void MidiEditor::SetMarkerList(MidiMarkerList* mrklist)
	{
		midiMarkerList.Copy(*mrklist);
	}

	//新建轨道，空轨道
	void MidiEditor::NewTrack()
	{
		Track* track = new Track(this);
		trackList.push_back(track);
		tempTracks.push_back(track);
		trackCount++;

		SetVirInstrument(track, 0, 0, 0);
	}

	//删除轨道
	void MidiEditor::DeleteTrack(Track* track)
	{
		if (track == nullptr)
			return;

		VirInstrument* vinst = track->GetChannel()->GetVirInstrument();
		vinst->Remove(true);

		vector<Track*>::iterator it = trackList.begin();
		for (; it != trackList.end(); it++) {
			if (*it == track) {
				trackList.erase(it);
				break;
			}
		}

		DEL(track);
		trackCount--;
	}


	//移除乐器片段
	void MidiEditor::RemoveInstFragment(InstFragment* instFragment)
	{
		Track* track = instFragment->GetTrack();
		if (track == nullptr)
			return;

		track->RemoveInstFragment(instFragment);

		//计算更新位置
		if (instFragment->startSec <= curtPlaySec &&
			instFragment->endSec >= curtPlaySec)
			track->_isUpdatePlayPos = true;
		else if (instFragment->startSec < curtPlaySec)
			track->_isUpdatePlayPrevPos = true;
	}


	//移动乐器片段到目标轨道分径的指定时间点
	void MidiEditor::MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec)
	{
		bool isAdd = false;
		if (dstTrack != nullptr && (
			instFragment->GetTrack() != dstTrack ||
			(instFragment->GetTrack() == dstTrack && instFragment->GetBranchIdx() != dstBranchIdx)))
		{
			RemoveInstFragment(instFragment);
			isAdd = true;
		}

		Tempo* tempo = midiMarkerList.GetTempo(sec);
		uint32_t tickCount = tempo->GetTickCount(sec);
		instFragment->SetStartTick(tickCount);

		if (isAdd)
			dstTrack->AddInstFragment(instFragment, dstBranchIdx);

		dstTrack = instFragment->track;
		if (dstTrack == nullptr)
			return;

		//计算更新位置
		dstTrack->_updateInstFrags.push_back(instFragment);
		if (!isAdd &&
			instFragment->startSec <= curtPlaySec &&
			instFragment->endSec >= curtPlaySec) {
			dstTrack->_isUpdatePlayPos = true;
		}
		if (sec < curtPlaySec)
			dstTrack->_isUpdatePlayPrevPos = true;
	}

	//计算指定轨道所有事件的实际时间点
	void MidiEditor::ComputeTrackEventsTime(Track* track)
	{
		Tempo* tempo;
		LinkedList<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;

		auto& instFragments = track->_updateInstFrags;
		for (int i = 0; i < instFragments.size(); i++)
		{
			instFrag = instFragments[i];
			instFrag->Clear();
			tempo = midiMarkerList.GetTempo((int)instFrag->startTick);
			instFrag->startSec = (float)tempo->GetTickSec(instFrag->startTick);

			eventList = &(instFrag->midiEvents);
			LinkedListNode<MidiEvent*>* node = eventList->GetHeadNode();
			for (; node; node = node->next)
			{
				ev = node->elem;
				int evStartTick = ev->startTick + instFrag->startTick;
				tempo = midiMarkerList.GetTempo(evStartTick);
				ev->endSec = ev->startSec = (float)tempo->GetTickSec(evStartTick);

				if (ev->type == MidiEventType::NoteOff)
					((NoteOffEvent*)ev)->noteOnEvent->endSec = ev->endSec;

				if (ev->endSec > instFrag->endSec)
					instFrag->endSec = ev->endSec;

				if (ev->endSec > track->endSec)
					track->endSec = ev->endSec;
			}

			if (instFrag->startSec < curtPlaySec)
				track->_isUpdatePlayPrevPos = true;
		}

		//
		if (track->_isUpdatePlayPos || track->_isUpdatePlayPrevPos)
		{
			if (track->_isUpdatePlayPos)
				midiSynther->OffVirInstrumentAllKeys(track->GetChannel());

			track->Clear();
			ProcessTrack(track, true);
		}

		track->_isUpdatePlayPos = false;
		track->_isUpdatePlayPrevPos = false;
		track->_updateInstFrags.clear();
	}



	//计算结束时间点
	void MidiEditor::ComputeEndSec()
	{
		endSec = 0;
		for (int i = 0; i < trackList.size(); i++)
		{
			if (trackList[i]->endSec > endSec)
				endSec = trackList[i]->endSec;
		}
	}


	/// <summary>
	/// 获取当前时间之后的notekeys
	/// </summary>
	/// <param name="lateSec">当前时间之后的秒数</param>
	void MidiEditor::GetCurTimeLateNoteKeys(double lateSec)
	{
		tempNoteKeys.clear();
		for (int i = 0; i < trackList.size(); i++)
			GetNoteKeys(trackList[i], lateSec);
	}

	void MidiEditor::GetNoteKeys(Track* track, double lateSec)
	{
		LinkedList<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;
		double sec;
		double minSec = lateSec + 1;
		auto& instFragmentBranchs = track->instFragmentBranchs;
		for (int j = 0; j < instFragmentBranchs.size(); j++)
		{
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[j]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[j]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				instFrag = *frag_it;
				eventList = &(instFrag->midiEvents);

				LinkedListNode<MidiEvent*>* node = instFrag->eventOffsetNode;
				for (; node; node = node->next)
				{
					ev = node->elem;
					sec = ev->startSec - curtPlaySec;
					if (sec > lateSec)
						break;

					if (ev->type == MidiEventType::NoteOn &&
						sec <= minSec &&
						!PlayModeAndTypeTest(ev, track))
					{
						if (sec < minSec)
							tempNoteKeys.clear();
						minSec = sec;
						tempNoteKeys.push_back(((NoteOnEvent*)ev)->note);

					}
				}
			}
		}
	}


	//开始播放
	void MidiEditor::Play()
	{
		if (state == EditorState::PLAY)
			return;

		if (state == EditorState::STOP) {
			for (int i = 0; i < trackList.size(); i++)
				trackList[i]->Clear();
		}

		for (int i = 0; i < trackList.size(); i++)
			midiSynther->OnVirInstrument(trackList[i]->GetChannel(), false);

		SetState(EditorState::PLAY);
	}

	//暂停播放
	void MidiEditor::Pause()
	{
		if (state != EditorState::PLAY)
			return;

		for (int i = 0; i < trackList.size(); i++) {
			midiSynther->OffVirInstrumentAllKeys(trackList[i]->GetChannel());
		}

		SetState(EditorState::PAUSE);
	}



	//停止播放
	void MidiEditor::Stop()
	{
		if (state == EditorState::STOP)
			return;

		for (int i = 0; i < trackList.size(); i++) {
			midiSynther->OffVirInstrument(trackList[i]->GetChannel());
			trackList[i]->Clear();
		}

		SetCurtPlaySec(0);
		SetState(EditorState::STOP);

	}


	//移除
	void MidiEditor::Remove()
	{
		for (int i = 0; i < trackList.size(); i++) {
			midiSynther->RemoveVirInstrument(trackList[i]->GetChannel());
			trackList[i]->Clear();
		}

		SetCurtPlaySec(0);
		SetState(EditorState::STOP);
	}


	//移动到指定时间点
	void MidiEditor::Runto(double sec)
	{
		//当状态不是处于播放状态时，可以直接使用goto到目标位置（使用goto会关闭当前发音）
		if (state != EditorState::PLAY || playMode != EditorPlayMode::Step) {
			Goto(sec);
			return;
		}

		if (sec >= curtPlaySec) {
			Process((sec - curtPlaySec) / speed, true);
		}
		else {
			Goto(sec);
		}
	}

	//设置播放的起始时间点
	void MidiEditor::Goto(double sec)
	{
		for (int i = 0; i < trackList.size(); i++) {
			midiSynther->OffVirInstrumentAllKeys(trackList[i]->GetChannel());
			trackList[i]->Clear();
		}

		if (sec > endSec)
			sec = endSec;

		SetCurtPlaySec(0);
		ProcessCore(sec / speed, true);
	}


	//设置快进到开头
	void MidiEditor::GotoStart()
	{
		Goto(0);
	}

	//设置快进到结尾
	void MidiEditor::GotoEnd()
	{
		Goto(endSec + 1);
	}

	void MidiEditor::DisableTrack(Track* track)
	{
		if (track == nullptr)
			return;

		track->isDisablePlay = true;
		midiSynther->OffVirInstrumentAllKeys(track->GetChannel());
	}

	void MidiEditor::DisableAllTrack()
	{
		for (int i = 0; i < trackList.size(); i++) {
			trackList[i]->isDisablePlay = true;
			midiSynther->OffVirInstrumentAllKeys(trackList[i]->GetChannel());
		}
	}

	void MidiEditor::DisableChannel(int channelIdx)
	{
		for (int i = 0; i < trackList.size(); i++) {

			if (trackList[i]->GetChannelNum() != channelIdx)
				continue;

			trackList[i]->isDisablePlay = true;
			midiSynther->OffVirInstrumentAllKeys(trackList[i]->GetChannel());
		}
	}


	void MidiEditor::EnableTrack(Track* track)
	{
		if (track == nullptr)
			return;

		track->isDisablePlay = false;
	}

	void MidiEditor::EnableAllTrack()
	{
		for (int i = 0; i < trackList.size(); i++) {
			trackList[i]->isDisablePlay = false;
		}
	}


	void MidiEditor::EnableChannel(int channelIdx)
	{
		for (int i = 0; i < trackList.size(); i++) {

			if (trackList[i]->GetChannelNum() != channelIdx)
				continue;

			trackList[i]->isDisablePlay = false;
		}
	}

	//设置轨道事件演奏方式
	void MidiEditor::SetTrackPlayType(Track* track, MidiEventPlayType playType)
	{
		if (track == nullptr)
			return;

		track->playType = playType;
	}


	//设置播放速率(相对于正常播放速率1.0的倍率)
	void MidiEditor::SetSpeed(float speed_)
	{
		speed = speed_;
	}

	//设置当前播放时间
	void MidiEditor::SetCurtPlaySec(double sec)
	{
		curtPlaySec = sec;
		if (editor->GetMainMidiEditor() == this &&
			midiSynther->maxCacheSize <= 0 || !midiSynther->isEnableCache)
			editor->curtPlaySec = curtPlaySec;
	}

	//设置状态
	void MidiEditor::SetState(EditorState s)
	{
		state = s;
		if (editor->GetMainMidiEditor() == this &&
			midiSynther->maxCacheSize <= 0 || !midiSynther->isEnableCache)
			editor->playState = state;
	}

	//设置轨道乐器
	void MidiEditor::SetVirInstrument(Track* track, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (track == nullptr)
			return;

		Channel* channel = track->GetChannel();
		VirInstrument* virInst = ((Synther*)midiSynther)->EnableVirInstrument(
			channel, bankSelectMSB, bankSelectLSB, instrumentNum);

		virInst->SetRealtime(false);
	}

	//设置打击乐器
	void MidiEditor::SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		for (int i = 0; i < trackList.size(); i++) {

			if (trackList[i]->GetChannelNum() != 9)
				continue;

			trackList[i]->GetChannel()->GetVirInstrument()->ChangeProgram(
				bankSelectMSB, bankSelectLSB, instrumentNum);
		}
	}


	//当前时间curtPlaySec往前处理一个sec的时间长度的所有midi事件
	void MidiEditor::Process(double sec, bool isStepOp)
	{
		//这个选项控制了是由非步进模式进入处理，还是步进模式进入处理
		//非步进模式，如果不是由Runto()进入处理的，将直接返回不进行Process
		if (playMode == EditorPlayMode::Step && !isStepOp)
			return;

		//如果进入等待,将不步进一个时间长度sec
		if (editor->isWait)
			sec = 0;

		if (state != EditorState::PLAY)
			return;

		ProcessCore(sec);

		//检测播放是否结束
		if (curtPlaySec >= endSec) {
			printf("当前轨道midi时间处理结束! \n");
			Pause();
			SetState(EditorState::ENDPAUSE);
		}
	}


	//当前时间curtPlaySec往前处理一个sec的时间长度的所有midi事件
	void MidiEditor::ProcessCore(double sec, bool isDirectGoto)
	{
		SetCurtPlaySec(curtPlaySec + sec * speed);

		for (int i = 0; i < trackList.size(); i++)
		{
			//重新处理当前时间点在事件处理时间中间时，可以重新启用此时间
			vector<MidiEvent*>& evs = trackList[i]->reProcessMidiEvents;
			if (!evs.empty()) {
				for (int j = 0; j < evs.size(); j++)
					ProcessEvent(evs[j], trackList[i], isDirectGoto);
				evs.clear();
			}

			//
			ProcessTrack(trackList[i], isDirectGoto);
		}
	}



	/// <summary>
	/// 处理轨道事件
	/// </summary>
	/// <param name="track">待处理的轨道</param>
	/// <param name="isDirectGoto">是否直接指定播放位置</param>
	void MidiEditor::ProcessTrack(Track* track, bool isDirectGoto)
	{
		LinkedList<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;

		auto& instFragmentBranchs = track->instFragmentBranchs;
		for (int j = 0; j < instFragmentBranchs.size(); j++)
		{
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[j]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[j]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				instFrag = *frag_it;
				eventList = &(instFrag->midiEvents);

				LinkedListNode<MidiEvent*>* node = instFrag->eventOffsetNode;
				for (; node; node = node->next)
				{
					ev = node->elem;

					//当直接指定播放位置时,
					//如果当前时间点在事件处理时间中间，将会重新启用此发声音符事件
					if (isDirectGoto &&
						ev->startSec < curtPlaySec &&
						ev->endSec > curtPlaySec)
					{
						track->reProcessMidiEvents.push_back(ev);
					}

					//
					if (ev->startSec > curtPlaySec)
						break;

					ProcessEvent(ev, track, isDirectGoto);
				}

				if (instFrag->eventOffsetNode != node)
					instFrag->eventOffsetNode = node;
			}
		}

		if (curtPlaySec >= endSec)
		{
			//轨道播发结束后，清除相关设置
			Channel* channel = track->GetChannel();
			if (channel != nullptr) {
				channel->SetControllerValue(MidiControllerType::SustainPedalOnOff, 0);
				midiSynther->ModulationVirInstParams(channel);
			}
		}
	}


	//处理轨道事件
	void MidiEditor::ProcessEvent(MidiEvent* midEv, Track* track, bool isDirectGoto)
	{
		Channel* channel = track->GetChannel();
		VirInstrument* virInst = channel->GetVirInstrument();
		if (virInst == nullptr)
			return;

		//
		switch (midEv->type)
		{
		case MidiEventType::NoteOn:
		{
			if (isDirectGoto || track->isDisablePlay)
				break;

			NoteOnEvent* noteOnEv = (NoteOnEvent*)midEv;

			if (PlayModeAndTypeTest(midEv, track))
			{
				virInst->OnKey(noteOnEv->note, (float)noteOnEv->velocity, noteOnEv->endTick - noteOnEv->startTick + 1);
			}
			else {
				editor->NeedOnKeySignal(noteOnEv->note);
			}
		}
		break;

		case MidiEventType::NoteOff:
		{
			if (isDirectGoto || track->isDisablePlay)
				break;

			NoteOffEvent* noteOffEv = (NoteOffEvent*)midEv;
			if (noteOffEv->noteOnEvent == nullptr)
				break;

			if (PlayModeAndTypeTest(midEv, track))
			{
				virInst->OffKey(noteOffEv->note, (float)noteOffEv->velocity);
			}
			else {
				editor->NeedOffKeySignal(noteOffEv->note);
			}
		}
		break;

		case MidiEventType::ProgramChange:
		{
			ProgramChangeEvent* ev = (ProgramChangeEvent*)midEv;
			//通道9为鼓点音色，一般不需要选择
			if (ev->channel == 9) {
				virInst->ChangeProgram(128, 0, 0);
				return;
			}

			virInst->SetProgramNum(ev->value);
		}
		break;


		case MidiEventType::PitchBend:
		{
			PitchBendEvent* ev = (PitchBendEvent*)midEv;
			virInst->SetPitchBend(ev->value);
		}
		break;

		case MidiEventType::Controller:
		{
			ControllerEvent* ev = (ControllerEvent*)midEv;
			virInst->SetController(ev->ctrlType, ev->value);
		}
		break;

		default:
			break;
		}
	}

	bool MidiEditor::PlayModeAndTypeTest(MidiEvent* midEv, Track* track)
	{
		if (playMode != EditorPlayMode::Wait)
			return true;


		if (track->playType == MidiEventPlayType::Custom)
		{
			if ((playType == MidiEventPlayType::DoubleHand &&
				midEv->playType != MidiEventPlayType::Background) ||

				(playType == MidiEventPlayType::LeftHand &&
					midEv->playType == MidiEventPlayType::LeftHand) ||

				(playType == MidiEventPlayType::RightHand &&
					midEv->playType == MidiEventPlayType::RightHand))
			{
				return false;
			}

			return true;
		}

		if (playType == MidiEventPlayType::DoubleHand &&
			track->playType != MidiEventPlayType::Background)
			return false;

		if (playType == track->playType)
			return false;

		return true;
	}

}
