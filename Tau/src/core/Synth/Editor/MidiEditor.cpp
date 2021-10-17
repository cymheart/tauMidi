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
		curtPlaySec = editor->GetPlaySec();
		speed = editor->GetSpeed();
		isWaitPlayMode = editor->isWaitPlayMode;
		state = editor->GetState();
		midiMarkerList.Copy(editor->midiMarkerList);
	}

	MidiEditor::~MidiEditor()
	{
		for (int i = 0; i < trackList.size(); i++)
			DEL(trackList[i]);
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
		list<MidiEvent*>* eventList;
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
			list<MidiEvent*>::iterator it = eventList->begin();
			list<MidiEvent*>::iterator end = eventList->end();
			for (; it != end; it++)
			{
				ev = *it;
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

	//开始播放
	void MidiEditor::Play()
	{
		if (state == EditorState::PLAY)
			return;


		if (state == EditorState::STOP) {
			curtPlaySec = 0;
			for (int i = 0; i < trackList.size(); i++)
				trackList[i]->Clear();
		}

		for (int i = 0; i < trackList.size(); i++)
			midiSynther->OnVirInstrument(trackList[i]->GetChannel());

		state = EditorState::PLAY;

	}

	//暂停播放
	void MidiEditor::Pause()
	{
		if (state != EditorState::PLAY)
			return;

		Goto(curtPlaySec);
		state = EditorState::PAUSE;
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

		curtPlaySec = 0;
		state = EditorState::STOP;
	}


	//移除
	void MidiEditor::Remove()
	{
		for (int i = 0; i < trackList.size(); i++) {
			midiSynther->RemoveVirInstrument(trackList[i]->GetChannel());
			trackList[i]->Clear();
		}

		curtPlaySec = 0;
		state = EditorState::STOP;
	}


	//移动到指定时间点
	void MidiEditor::Runto(double sec)
	{
		//当状态不是处于播放状态时，可以直接使用goto到目标位置（使用goto会关闭当前发音）
		if (state != EditorState::PLAY || !editor->isStepPlayMode) {
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

		curtPlaySec = 0;
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


	//处理
	void MidiEditor::Process(double sec, bool isStepOp)
	{
		if (editor->isStepPlayMode && !isStepOp)
			return;

		if (editor->isWait)
			sec = 0;

		if (state != EditorState::PLAY)
			return;

		ProcessCore(sec);
	}


	void MidiEditor::ProcessCore(double sec, bool isDirectGoto)
	{
		int trackEndCount = 0;
		curtPlaySec = curtPlaySec + sec * speed;

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
			trackEndCount += ProcessTrack(trackList[i], isDirectGoto);
		}

		//检测播放是否结束
		if (trackEndCount == trackList.size())
		{
			printf("当前轨道midi时间处理结束! \n");
			Pause();
		}
	}

	int MidiEditor::ProcessTrack(Track* track, bool isDirectGoto)
	{
		list<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;
		int orgInstFragCount = 0;
		int instFragCount = 0;

		if (track->isEnded)
			return 1;

		auto& instFragmentBranchs = track->instFragmentBranchs;
		for (int j = 0; j < instFragmentBranchs.size(); j++)
		{
			orgInstFragCount += instFragmentBranchs[j]->size();
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[j]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[j]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				instFrag = *frag_it;
				if (instFrag->isEnded) {
					instFragCount++;
					continue;
				}

				eventList = &(instFrag->midiEvents);
				list<MidiEvent*>::iterator it = instFrag->eventOffsetIter;
				list<MidiEvent*>::iterator end = eventList->end();
				for (; it != end; it++)
				{
					ev = *it;

					//重新处理当前时间点在事件处理时间中间时，可以重新启用此事件
					if (isDirectGoto &&
						ev->startSec < curtPlaySec &&
						ev->endSec > curtPlaySec)
					{
						track->reProcessMidiEvents.push_back(ev);
					}

					//
					if (ev->startSec > curtPlaySec)
					{
						instFrag->eventOffsetIter = it;
						break;
					}

					ProcessEvent(ev, track, isDirectGoto);
				}

				if (it == end)
				{
					instFrag->isEnded = true;
				}
			}
		}

		if (instFragCount == orgInstFragCount)
		{
			track->isEnded = true;

			//轨道播发结束后，清除相关设置
			Channel* channel = track->GetChannel();
			if (channel != nullptr) {
				channel->SetControllerValue(MidiControllerType::SustainPedalOnOff, 0);
				midiSynther->ModulationVirInstParams(channel);
			}
		}

		return 0;
	}

	//处理轨道事件
	void MidiEditor::ProcessEvent(MidiEvent* midEv, Track* track, bool isDirectGoto)
	{
		Channel* channel = track->GetChannel();
		VirInstrument* virInst = channel->GetVirInstrument();

		//
		switch (midEv->type)
		{
		case MidiEventType::NoteOn:
		{
			if (isDirectGoto ||
				track->isDisablePlay)
				break;

			NoteOnEvent* noteOnEv = (NoteOnEvent*)midEv;

			if (!isWaitPlayMode ||
				track->playType == MidiEventPlayType::Background ||
				(track->playType == MidiEventPlayType::Custom &&
					midEv->playType != MidiEventPlayType::Background))
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
			if (isDirectGoto ||
				track->isDisablePlay)
				break;

			NoteOffEvent* noteOffEv = (NoteOffEvent*)midEv;
			if (noteOffEv->noteOnEvent == nullptr)
				break;

			if (!isWaitPlayMode ||
				track->playType == MidiEventPlayType::Background ||
				(track->playType == MidiEventPlayType::Custom &&
					midEv->playType != MidiEventPlayType::Background))
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

			//	virInst->SetProgramNum(ev->value);
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
		}
	}

}
