#include"MidiEditor.h"
#include"Track.h"
#include"Synth/Tau.h"
#include"Synth/Synther/Synther.h"
#include"Synth/Channel.h"
#include"Synth/KeySounder.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include"Midi/MidiFile.h"
#include"Synth/VirInstrument.h"
#include"Synth/Preset.h"
#include"Editor.h"
#include"MeasureInfo.h"


namespace tau
{
	MidiEditor::MidiEditor(Synther* synther)
	{
		midiSynther = synther;
		tau = synther->tau;
		editor = tau->editor;

		//初始化值
		SetCurtPlaySec(editor->GetPlaySec());
		SetState(editor->GetPlayState());
		speed = editor->GetSpeed();
		playMode = editor->playMode;
		playType = editor->playType;
		midiMarkerList = &editor->midiMarkerList;
	}

	MidiEditor::~MidiEditor()
	{

	}

	bool MidiEditor::NoteCmp(MidiEvent* a, MidiEvent* b) {
		return a->startSec < b->startSec;
	}

	//设置标记
	void MidiEditor::SetMarkerList(MidiMarkerList* mrklist)
	{
		midiMarkerList = mrklist;
	}

	//新建轨道，空轨道
	void MidiEditor::NewTrack()
	{
		Track* track = new Track(this);
		tracks.push_back(track);
		trackCount++;

		SetVirInstrument(track, 0, 0, 0);
	}

	//删除轨道
	void MidiEditor::DeleteTrack(Track* track)
	{
		if (track == nullptr)
			return;

		vector<VirInstrument*>& vinsts = track->GetChannel()->GetVirInstruments();
		for (int i = 0; i < vinsts.size(); i++)
			vinsts[i]->Remove(true);

		vector<Track*>::iterator it = tracks.begin();
		for (; it != tracks.end(); it++) {
			if (*it == track) {
				tracks.erase(it);
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

		Tempo* tempo = midiMarkerList->GetTempo(sec);
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
			tempo = midiMarkerList->GetTempo((int)instFrag->startTick);
			instFrag->startSec = (float)tempo->GetTickSec(instFrag->startTick);

			eventList = &(instFrag->midiEvents);
			LinkedListNode<MidiEvent*>* node = eventList->GetHeadNode();
			for (; node; node = node->next)
			{
				ev = node->elem;
				int evStartTick = instFrag->startTick + ev->startTick;
				int evEndTick = instFrag->startTick + ev->endTick;
				tempo = midiMarkerList->GetTempo(evStartTick);
				ev->endSec = ev->startSec = (float)tempo->GetTickSec(evStartTick);

				if (ev->type == MidiEventType::NoteOff)
					((NoteOffEvent*)ev)->noteOnEvent->endSec = ev->endSec;

				if (ev->endSec > instFrag->endSec)
					instFrag->endSec = ev->endSec;

				if (ev->endSec > track->endSec)
					track->endSec = ev->endSec;

				//
				if (evEndTick > instFrag->endTick)
					instFrag->endTick = evEndTick;

				if (evEndTick > track->endTick)
					track->endTick = evEndTick;

			}

			if (instFrag->startSec < curtPlaySec)
				track->_isUpdatePlayPrevPos = true;
		}

		//
		if (track->_isUpdatePlayPos || track->_isUpdatePlayPrevPos)
		{
			if (track->_isUpdatePlayPos)
				midiSynther->OffAllKeys(track->GetChannel());

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
		for (int i = 0; i < tracks.size(); i++)
		{
			if (tracks[i]->endSec > endSec)
				endSec = tracks[i]->endSec;
		}
	}

	//计算结束tick点
	void MidiEditor::ComputeEndTick()
	{
		endTick = 0;
		for (int i = 0; i < tracks.size(); i++)
		{
			if (tracks[i]->endTick > endTick)
				endTick = tracks[i]->endTick;
		}
	}

	//开始播放
	void MidiEditor::Play()
	{
		if (state == EditorState::PLAY)
			return;

		for (int i = 0; i < tracks.size(); i++)
			midiSynther->OpenVirInstrument(tracks[i]->GetChannel(), false);

		SetState(EditorState::PLAY);
		Process(0);
	}

	//暂停播放
	void MidiEditor::Pause()
	{
		if (state != EditorState::PLAY)
			return;

		for (int i = 0; i < tracks.size(); i++) {
			midiSynther->OffAllKeys(tracks[i]->GetChannel());
		}

		SetState(EditorState::PAUSE);
	}



	//停止播放
	void MidiEditor::Stop()
	{
		if (state == EditorState::STOP)
			return;

		for (int i = 0; i < tracks.size(); i++) {
			midiSynther->CloseVirInstrument(tracks[i]->GetChannel());
			tracks[i]->Clear();
		}

		SetCurtPlaySec(editor->initStartPlaySec);
		SetState(EditorState::STOP);

	}


	//移除
	void MidiEditor::Remove()
	{
		for (int i = 0; i < tracks.size(); i++) {
			midiSynther->RemoveVirInstrument(tracks[i]->GetChannel());
			tracks[i]->Clear();
		}

		SetCurtPlaySec(editor->initStartPlaySec);
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
			sec -= curtPlaySec;
			Process(sec, true);
		}
		else {
			Goto(sec);
		}
	}

	//设置播放的起始时间点
	void MidiEditor::Goto(double sec)
	{
		if (sec == curtPlaySec)
			return;

		for (int i = 0; i < tracks.size(); i++)
			midiSynther->OffAllKeys(tracks[i]->GetChannel());

		if (sec < curtPlaySec)
		{
			for (int i = 0; i < tracks.size(); i++)
				tracks[i]->Clear();
			SetCurtPlaySec(0);
			simpleModeTrackNotesOffset = simpleModeTrackNotes.GetHeadNode();
		}

		ProcessCore(sec - curtPlaySec, true);

	}


	//设置快进到开头
	void MidiEditor::GotoStart()
	{
		Goto(editor->initStartPlaySec);
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
		midiSynther->OffAllKeys(track->GetChannel());
	}

	void MidiEditor::DisableTrack(int trackIdx)
	{
		if (trackIdx < 0 || trackIdx >= tracks.size())
			return;

		tracks[trackIdx]->isDisablePlay = true;
		midiSynther->OffAllKeys(tracks[trackIdx]->GetChannel());
	}

	void MidiEditor::DisableAllTrack()
	{
		DisableChannel(-1);
	}

	void MidiEditor::DisableChannel(int channelIdx)
	{
		for (int i = 0; i < tracks.size(); i++) {

			if (channelIdx != -1 &&
				tracks[i]->GetChannelNum() != channelIdx)
				continue;

			tracks[i]->isDisablePlay = true;
			midiSynther->OffAllKeys(tracks[i]->GetChannel());
		}
	}


	void MidiEditor::EnableTrack(Track* track)
	{
		if (track == nullptr)
			return;

		track->isDisablePlay = false;
	}

	void MidiEditor::EnableTrack(int trackIdx)
	{
		if (trackIdx < 0 || trackIdx >= tracks.size())
			return;

		tracks[trackIdx]->isDisablePlay = false;
	}

	void MidiEditor::EnableAllTrack()
	{
		EnableChannel(-1);
	}


	void MidiEditor::EnableChannel(int channelIdx)
	{
		for (int i = 0; i < tracks.size(); i++) {

			if (channelIdx != -1 && 
				tracks[i]->GetChannelNum() != channelIdx)
				continue;

			tracks[i]->isDisablePlay = false;
		}
	}

	//设置通道声音增益(单位:dB)
	void MidiEditor::SetChannelVolumeGain(int channelIdx, float gainDB)
	{
		for (int i = 0; i < tracks.size(); i++) {

			if (channelIdx != -1 && 
				tracks[i]->GetChannelNum() != channelIdx)
				continue;

			tracks[i]->GetChannel()->SetVolumeGain(gainDB);
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
		if (midiSynther->maxCacheSize <= 0 || !midiSynther->isEnableCache)
			editor->curtPlaySec = curtPlaySec;
	}

	//设置状态
	void MidiEditor::SetState(EditorState s)
	{
		state = s;
		if (midiSynther->maxCacheSize <= 0 || !midiSynther->isEnableCache)
			editor->playState = state;
	}

	//设置轨道乐器
	void MidiEditor::SetVirInstrument(Track* track, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (track == nullptr)
			return;

		Channel* channel = track->GetChannel();
		midiSynther->EnableVirInstrument(channel, bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	//设置轨道乐器
	void MidiEditor::SetVirInstrument(int trackIdx, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (trackIdx < 0 || trackIdx >= tracks.size())
			return;

		SetVirInstrument(tracks[trackIdx], bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	//设置打击乐器
	void MidiEditor::SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		for (int i = 0; i < tracks.size(); i++) {

			if (tracks[i]->GetChannelNum() != 9)
				continue;

			vector<VirInstrument*>& vinsts = tracks[i]->GetChannel()->GetVirInstruments();
			for (int i = 0; i < vinsts.size(); i++)
			{
				vinsts[i]->ChangeProgram(
					bankSelectMSB, bankSelectLSB, instrumentNum);
			}

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
		float measureEndSec = editor->measureInfo.GetMeasureEndSec(
			editor->measureInfo.GetMeasureCount());

		if (curtPlaySec >= measureEndSec) {
			printf("当前轨道midi时间处理结束! \n");
			Pause();
		}
	}


	//当前时间curtPlaySec往前处理一个sec的时间长度的所有midi事件
	void MidiEditor::ProcessCore(double sec, bool isDirectGoto)
	{
		SetCurtPlaySec(curtPlaySec + sec);

		for (int i = 0; i < tracks.size(); i++)
		{
			ProcessTrack(tracks[i], isDirectGoto);
		}

		//
		ProcessSimpleModeTrack(simpleModeTrack, isDirectGoto);
	}



	/// <summary>
	/// 处理轨道事件
	/// </summary>
	/// <param name="track">待处理的轨道</param>
	/// <param name="isDirectGoto">是否直接指定播放位置</param>
	void MidiEditor::ProcessTrack(Track* track, bool isDirectGoto)
	{
		//重新处理当前时间点在事件处理时间中间时，可以重新启用此时间
		vector<MidiEvent*>& evs = track->reProcessMidiEvents;
		if (!evs.empty()) {
			for (int i = 0; i < evs.size(); i++)
				ProcessEvent(evs[i], track, isDirectGoto);
			evs.clear();
		}

		//
		LinkedList<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;
		auto& instFragmentBranchs = track->instFragmentBranchs;
		for (int i = 0; i < instFragmentBranchs.size(); i++)
		{
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[i]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[i]->end();
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
						ev->startSec <= curtPlaySec &&
						ev->endSec >= curtPlaySec)
					{
						if (editor->noteSoundStartSec >= 0 &&
							editor->noteSoundEndSec >= 0 &&
							(ev->startSec < editor->noteSoundStartSec ||
								ev->startSec >= editor->noteSoundEndSec))
							continue;

						if (!IsNeedWaitKeySignal(ev, track))
							track->reProcessMidiEvents.push_back(ev);
					}


					//
					if (ev->startSec > curtPlaySec)
						break;


					//当note不在发音范围内时(noteSoundStartSec, noteSoundEndSec)，将不处理这个note ev
					if (editor->noteSoundStartSec >= 0 &&
						editor->noteSoundEndSec >= 0 &&
						(ev->startSec < editor->noteSoundStartSec ||
							ev->startSec >= editor->noteSoundEndSec))
					{
						if (ev->type == MidiEventType::NoteOn ||
							ev->type == MidiEventType::NoteOff)
							continue;
					}


					ProcessEvent(ev, track, isDirectGoto);
				}

				instFrag->eventOffsetNode = node;
			}
		}

		if (curtPlaySec >= endSec)
		{
			//轨道播放结束后，清除相关设置
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
		vector<VirInstrument*>& vinsts = channel->GetVirInstruments();

		if (vinsts.empty())
			return;

		//
		switch (midEv->type)
		{
		case MidiEventType::NoteOn:
		{
			if (isDirectGoto || track->isDisablePlay)
				break;

			NoteOnEvent* noteOnEv = (NoteOnEvent*)midEv;

			if (!IsNeedWaitKeySignal(midEv, track))
			{
				if (!editor->isOpenAccompany)
					break;

				midiSynther->OnKey(channel, noteOnEv->note,
					(float)noteOnEv->velocity, noteOnEv->endTick - noteOnEv->startTick + 1);
			}
			else {
				editor->NeedOnKeySignal(noteOnEv->note, noteOnEv);
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

			if (!IsNeedWaitKeySignal(midEv, track))
			{
				if (!editor->isOpenAccompany)
					break;

				midiSynther->OffKey(channel, noteOffEv->note, (float)noteOffEv->velocity);
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
				for (int i = 0; i < vinsts.size(); i++)
					vinsts[i]->ChangeProgram(128, 0, 0);
				return;
			}

			for (int i = 0; i < vinsts.size(); i++)
				vinsts[i]->SetProgramNum(ev->value);
		}
		break;


		case MidiEventType::PitchBend:
		{
			PitchBendEvent* ev = (PitchBendEvent*)midEv;
			for (int i = 0; i < vinsts.size(); i++)
				vinsts[i]->SetPitchBend(ev->value);
		}
		break;

		case MidiEventType::Controller:
		{
			ControllerEvent* ev = (ControllerEvent*)midEv;
			for (int i = 0; i < vinsts.size(); i++)
				vinsts[i]->SetController(ev->ctrlType, ev->value);
		}
		break;

		default:
			break;
		}
	}

	void MidiEditor::ProcessSimpleModeTrack(Track* track, bool isDirectGoto)
	{
		if (simpleModeTrackNotes.Empty())
			return;

		Channel* channel = track->GetChannel();
		MidiEvent* ev;
		auto node = simpleModeTrackNotesOffset;


		for (; node; node = node->next)
		{
			ev = node->elem;
			if (ev->startSec > curtPlaySec)
				break;

			if (isDirectGoto || track->isDisablePlay)
				continue;

			if (editor->noteSoundStartSec >= 0 &&
				editor->noteSoundEndSec >= 0 &&
				(ev->startSec < editor->noteSoundStartSec ||
					ev->startSec >= editor->noteSoundEndSec))
			{
				continue;
			}

			if (ev->type == MidiEventType::NoteOn)
			{
				NoteOnEvent* noteOnEv = (NoteOnEvent*)ev;

				if (!IsNeedWaitKeySignal(noteOnEv, track))
				{
					midiSynther->OnKey(channel, noteOnEv->note,
						(float)noteOnEv->velocity, noteOnEv->endTick - noteOnEv->startTick + 1);
				}
				else {
					editor->NeedOnKeySignal(noteOnEv->note, noteOnEv);
				}

			}
			else if (ev->type == MidiEventType::NoteOff)
			{
				NoteOffEvent* noteOffEv = (NoteOffEvent*)ev;
				if (!IsNeedWaitKeySignal(ev, track))
				{
					midiSynther->OffKey(channel, noteOffEv->note, (float)noteOffEv->velocity);
				}
				else {
					editor->NeedOffKeySignal(noteOffEv->note);
				}
			}
		}

		simpleModeTrackNotesOffset = node;
	}



	//是否需要等待按键信号
	bool MidiEditor::IsNeedWaitKeySignal(MidiEvent* midEv, Track* track)
	{
		if (playMode != EditorPlayMode::Wait &&
			playMode != EditorPlayMode::Mute)
			return false;

		if ((midEv->type == MidiEventType::NoteOn &&
			editor->excludeNeedWaitKey[((NoteOnEvent*)midEv)->note]) ||
			(midEv->type == MidiEventType::NoteOff &&
				editor->excludeNeedWaitKey[((NoteOffEvent*)midEv)->note]))
		{
			return false;
		}

		return IsPointerPlayNote(midEv, track);
	}

	//是否为手指弹奏的音符
	bool MidiEditor::IsPointerPlayNote(MidiEvent* ev, Track* track)
	{
		//
		if (track->playType == MidiEventPlayType::Custom)
		{
			if ((playType == MidiEventPlayType::DoubleHand &&
				ev->playType != MidiEventPlayType::Background) ||

				(playType == MidiEventPlayType::LeftHand &&
					ev->playType == MidiEventPlayType::LeftHand) ||

				(playType == MidiEventPlayType::RightHand &&
					ev->playType == MidiEventPlayType::RightHand))
			{
				return true;
			}

			return false;
		}

		if (playType == MidiEventPlayType::DoubleHand &&
			track->playType != MidiEventPlayType::Background)
			return true;

		if (playType == track->playType)
			return true;

		return false;
	}




}
