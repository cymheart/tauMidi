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


namespace tau
{
	MidiEditor::MidiEditor(MidiEditorSynther* midiSynther)
	{
		this->midiSynther = midiSynther;
		tau = midiSynther->tau;
	}

	MidiEditor::~MidiEditor()
	{
		for (int i = 0; i < trackList.size(); i++)
			DEL(trackList[i]);
	}

	//新建轨道，空轨道
	void MidiEditor::NewTrack()
	{
		Track* track = new Track(this);
		trackList.push_back(track);
		tempTracks.push_back(track);
		trackCount++;

		VirInstrument* vinst = ((Synther*)midiSynther)->EnableVirInstrument(track->GetChannel(), 0, 0, 0);
		vinst->SetRealtime(false);
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


	//设置标记
	void MidiEditor::SetMarkerList(MidiMarkerList* mrklist)
	{
		midiMarkerList.Copy(*mrklist);
	}


	//移除乐器片段
	void MidiEditor::RemoveInstFragment(InstFragment* instFragment)
	{
		Track* track = instFragment->GetTrack();
		if (track != nullptr)
			track->RemoveInstFragment(instFragment);
	}


	//移动乐器片段到目标轨道分径的指定时间点
	void MidiEditor::MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec)
	{
		if (dstTrack != nullptr && (
			instFragment->GetTrack() != dstTrack ||
			(instFragment->GetTrack() == dstTrack && instFragment->GetBranchIdx() != dstBranchIdx)))
		{
			RemoveInstFragment(instFragment);
		}

		//
		Tempo* tempo = midiMarkerList.GetTempo(sec);
		uint32_t tickCount = tempo->GetTickCount(sec);
		instFragment->SetStartTick(tickCount);

		//
		if (dstTrack != nullptr && (
			instFragment->GetTrack() != dstTrack ||
			(instFragment->GetTrack() == dstTrack && instFragment->GetBranchIdx() != dstBranchIdx)))
		{
			dstTrack->AddInstFragment(instFragment, dstBranchIdx);
		}
	}


	//开始播放
	void MidiEditor::Play()
	{
		if (state == EditorState::PLAY)
			return;

		for (int i = 0; i < trackList.size(); i++)
			midiSynther->OnVirInstrument(trackList[i]->GetChannel());

		state = EditorState::PLAY;
		playStartSec = midiSynther->sec;

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

		for (int i = 0; i < trackList.size(); i++)
			midiSynther->OffVirInstrument(trackList[i]->GetChannel());

		isGotoEnd = false;
		isDirectGoto = false;
		isOpen = false;
		gotoSec = 0;
		state = EditorState::STOP;
	}


	//移除
	void MidiEditor::Remove()
	{
		for (int i = 0; i < trackList.size(); i++)
			midiSynther->RemoveVirInstrument(trackList[i]->GetChannel());

		isGotoEnd = false;
		isDirectGoto = false;
		isOpen = false;
		gotoSec = 0;
		state = EditorState::STOP;
	}


	//设置播放的起始时间点
	void MidiEditor::Goto(double gotoSec_)
	{
		EditorState oldState = state;

		for (int i = 0; i < trackList.size(); i++)
			midiSynther->OffVirInstrumentAllKeys(trackList[i]->GetChannel());

		isDirectGoto = false;
		isOpen = false;
		isGotoEnd = false;
		gotoSec = gotoSec_;
		playStartSec = midiSynther->sec;
		curtPlaySec = 0;
		baseSpeedSec = 0;
		state = oldState;
	}

	//设置快进到开头
	void MidiEditor::GotoStart()
	{
		Goto(0);
	}

	//设置快进到结尾
	void MidiEditor::GotoEnd()
	{
		Goto(9999999);
		isGotoEnd = true;
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


	//设置播放速率(相对于正常播放速率1.0的倍率)
	void MidiEditor::SetSpeed(float speed_)
	{
		baseSpeedSec = baseSpeedSec + speed * (midiSynther->sec - playStartSec - baseSpeedSec);
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


	//计算指定轨道所有事件的实际时间点
	void MidiEditor::ComputeTrackEventsTime(Track* track)
	{
		Tempo* tempo;
		list<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;

		auto& instFragments = track->instFragments;
		for (int i = 0; i < instFragments.size(); i++)
		{
			list<InstFragment*>::iterator frag_it = instFragments[i]->begin();
			list<InstFragment*>::iterator frag_end = instFragments[i]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				instFrag = *frag_it;
				eventList = &(instFrag->midiEvents);
				list<MidiEvent*>::iterator it = eventList->begin();
				list<MidiEvent*>::iterator end = eventList->end();
				for (; it != end; it++)
				{
					ev = *it;
					tempo = midiMarkerList.GetTempo((int)ev->startTick);
					ev->endSec = ev->startSec = (float)tempo->GetTickSec(ev->startTick);

					if (ev->type == MidiEventType::NoteOff)
					{
						((NoteOffEvent*)ev)->noteOnEvent->endSec = ev->endSec;

						if (ev->endSec > track->endSec)
							track->endSec = ev->endSec;
					}
				}
			}
		}
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

	//运行
	void MidiEditor::Run(double sec)
	{
		if (state != EditorState::PLAY)
			return;

		if (isOpen == false)
		{
			isOpen = true;

			for (int i = 0; i < trackList.size(); i++)
			{
				trackList[i]->Clear();
			}

			if (gotoSec > 0)
			{
				isDirectGoto = true;
				RunCore(gotoSec / speed);
				isDirectGoto = false;
				isGotoEnd = false;
			}
		}

		//
		RunCore(gotoSec / speed + sec - playStartSec);
	}


	void MidiEditor::RunCore(double sec)
	{

		list<MidiEvent*>* eventList;
		InstFragment* instFrag;
		MidiEvent* ev;
		int orgInstFragCount = 0;
		int trackEndCount = 0;
		int instFragCount = 0;
		curtPlaySec = baseSpeedSec + (sec - baseSpeedSec) * speed;
		Tempo* tempo = midiMarkerList.GetTempo(curtPlaySec);
		uint32_t curtEndTick = tempo->GetTickCount(curtPlaySec);

		for (int i = 0; i < trackList.size(); i++)
		{
			if (trackList[i]->isEnded) {
				trackEndCount++;
				continue;
			}

			instFragCount = 0;
			auto& instFragments = trackList[i]->instFragments;
			for (int j = 0; j < instFragments.size(); j++)
			{
				orgInstFragCount += instFragments[j]->size();
				list<InstFragment*>::iterator frag_it = instFragments[j]->begin();
				list<InstFragment*>::iterator frag_end = instFragments[j]->end();
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

						//
						if (!isGotoEnd && ev->startTick + instFrag->startTick > curtEndTick)
						{
							instFrag->eventOffsetIter = it;
							break;
						}

						ProcessEvent(ev, i, tempo);
					}

					if (it == end)
					{
						instFrag->isEnded = true;
					}
				}
			}

			if (instFragCount == orgInstFragCount)
			{
				trackList[i]->isEnded = true;

				//轨道播发结束后，清除相关设置
				Channel* channel = trackList[i]->GetChannel();
				if (channel != nullptr) {
					channel->SetControllerValue(MidiControllerType::SustainPedalOnOff, 0);
					midiSynther->ModulationVirInstParams(channel);
				}
			}
		}

		//检测播放是否结束
		if (trackEndCount == trackList.size())
		{
			printf("当前轨道midi时间处理结束! \n");
			Pause();
		}
	}

	//处理轨道事件
	void MidiEditor::ProcessEvent(MidiEvent* midEv, int trackIdx, Tempo* tempo)
	{
		Channel* channel = trackList[trackIdx]->GetChannel();
		VirInstrument* virInst = channel->GetVirInstrument();

		//
		switch (midEv->type)
		{
		case MidiEventType::NoteOn:
		{
			if (isDirectGoto || trackList[trackIdx]->isDisablePlay)
				break;

			NoteOnEvent* noteOnEv = (NoteOnEvent*)midEv;
			virInst->OnKey(noteOnEv->note, (float)noteOnEv->velocity, noteOnEv->endTick - noteOnEv->startTick + 1);
		}
		break;

		case MidiEventType::NoteOff:
		{
			if (isDirectGoto || trackList[trackIdx]->isDisablePlay)
				break;

			NoteOffEvent* noteOffEv = (NoteOffEvent*)midEv;
			if (noteOffEv->noteOnEvent == nullptr)
				break;

			virInst->OffKey(noteOffEv->note, (float)noteOffEv->velocity);
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
		}
	}

}
