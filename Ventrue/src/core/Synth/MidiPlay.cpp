#include"MidiPlay.h"
#include"Track.h"
#include"Ventrue.h"
#include"Channel.h"
#include"KeySounder.h"
#include"Midi/MidiEvent.h"
#include"Midi/MidiTrack.h"
#include"Midi/MidiFile.h"
#include"VirInstrument.h"
#include"Preset.h"

namespace ventrue
{
	MidiPlay::MidiPlay(Ventrue* ventrue)
	{
		this->ventrue = ventrue;
		assistTrack = new Track(0);
	}

	MidiPlay::~MidiPlay()
	{
		DEL(midiFile);

		for (int i = 0; i < trackList.size(); i++)
			DEL(trackList[i]);

		for (int i = 0; i < assistMidiEvList.size(); i++)
			DEL(assistMidiEvList[i]);

		DEL(assistTrack);
	}

	// 解析MidiFile
	void MidiPlay::ParseMidiFile(string midiFilePath, TrackChannelMergeMode mode)
	{
		midiFile = new MidiFile();
		midiFile->SetTrackChannelMergeMode(mode);
		midiFile->Parse(midiFilePath);

		//
		midiTrackList = midiFile->GetTrackList();
		Clear();

		//
		state = MidiPlayState::PLAY;
		isComputeEventTime = true;
		GotoEnd();
		TrackRun(0);
		isComputeEventTime = false;
		state = MidiPlayState::STOP;
		GotoStart();

		//
		//printf("midi文件总时长:%.2f秒 \n", endSec);
	}


	//停止播放
	void MidiPlay::Stop()
	{
		for (int i = 0; i < trackList.size(); i++)
		{
			for (int j = 0; j < 16; j++) {
				OffVirInstrumentByTrackChannel(i, j);
			}
		}

		Clear();
		state = MidiPlayState::STOP;
	}

	//开始播放
	void MidiPlay::Play()
	{
		for (int i = 0; i < trackList.size(); i++)
		{
			for (int j = 0; j < 16; j++) {
				OnVirInstrumentByTrackChannel(i, j);
			}
		}
		state = MidiPlayState::PLAY;
	}

	//暂停播放
	void MidiPlay::Suspend()
	{
		for (int i = 0; i < trackList.size(); i++)
		{
			for (int j = 0; j < 16; j++) {
				OffVirInstrumentByTrackChannel(i, j);
			}
		}

		Clear();
		this->gotoSec = ventrue->sec;
		state = MidiPlayState::SUSPEND;
	}


	//移除
	void MidiPlay::Remove()
	{
		for (int i = 0; i < trackList.size(); i++)
		{
			for (int j = 0; j < 16; j++) {
				RemoveVirInstrumentByTrackChannel(i, j);
			}
		}

		Clear();
		state = MidiPlayState::STOP;
	}




	//设置播放的起始时间点
	void MidiPlay::Goto(double gotoSec)
	{
		MidiPlayState oldState = state;

		Channel* channel;
		for (int i = 0; i < trackList.size(); i++)
		{
			for (int j = 0; j < 16; j++) {
				channel = trackList[i]->channels[j];
				ventrue->OffVirInstrumentAllKeysByChannel(channel, false);
			}
		}

		Clear();
		isGotoEnd = false;
		this->gotoSec = gotoSec;
		state = oldState;
	}

	//设置快进到开头
	void MidiPlay::GotoStart()
	{
		Goto(0);
	}

	//设置快进到结尾
	void MidiPlay::GotoEnd()
	{
		Goto(9999999);
		isGotoEnd = true;
	}




	//打开轨道通道对应的虚拟乐器
	void MidiPlay::OnVirInstrumentByTrackChannel(int trackIdx, int channelIdx)
	{
		Channel* channel = trackList[trackIdx]->channels[channelIdx];
		ventrue->OnVirInstrumentByChannel(channel);
	}

	//关闭轨道通道对应的虚拟乐器
	void MidiPlay::OffVirInstrumentByTrackChannel(int trackIdx, int channelIdx)
	{
		Channel* channel = trackList[trackIdx]->channels[channelIdx];
		ventrue->OffVirInstrumentByChannel(channel);
	}

	//移除轨道通道对应的虚拟乐器
	void MidiPlay::RemoveVirInstrumentByTrackChannel(int trackIdx, int channelIdx)
	{
		Channel* channel = trackList[trackIdx]->channels[channelIdx];
		ventrue->RemoveVirInstrumentByChannel(channel);
	}



	//设置打击乐号
	void MidiPlay::SetPercussionProgramNum(int num)
	{
		percussionProgramNum = num;
		for (int i = 0; i < trackList.size(); i++)
		{
			trackList[i]->SetPercussionProgramNum(percussionProgramNum);
		}
	}


	void MidiPlay::Clear()
	{
		isGotoEnd = false;
		isDirectGoto = false;
		isOpen = false;
		startTime = 0;
		gotoSec = 0;

		for (int i = 0; i < assistMidiEvList.size(); i++)
			DEL(assistMidiEvList[i]);

		assistTrack->Clear();

		//生成音轨演奏信息
		for (int i = 0; i < trackList.size(); i++)
		{
			trackList[i]->Clear();
		}

		if (trackList.size() < midiTrackList->size())
		{
			size_t count = midiTrackList->size() - trackList.size();
			for (size_t i = 0; i < count; i++)
			{
				trackList.push_back(new Track((int)(trackList.size())));
			}
		}
	}

	void MidiPlay::DisableTrack(int trackIdx)
	{
		if (trackIdx >= trackList.size())
			return;

		trackList[trackIdx]->isDisablePlay = true;
	}

	void MidiPlay::DisableAllTrack()
	{
		for (int i = 0; i < trackList.size(); i++) {
			trackList[i]->isDisablePlay = true;
		}

		assistTrack->isDisablePlay = true;
	}


	void MidiPlay::EnableTrack(int trackIdx)
	{
		if (trackIdx >= trackList.size())
			return;

		trackList[trackIdx]->isDisablePlay = false;
	}

	void MidiPlay::EnableAllTrack()
	{
		for (int i = 0; i < trackList.size(); i++) {
			trackList[i]->isDisablePlay = false;
		}

		assistTrack->isDisablePlay = false;
	}


	void MidiPlay::DisableTrackChannel(int trackIdx, int channel)
	{
		if (trackIdx >= trackList.size())
			return;

		trackList[trackIdx]->DisablePlayChannel(channel);
	}

	void MidiPlay::EnableTrackChannel(int trackIdx, int channel)
	{
		if (trackIdx >= trackList.size())
			return;

		trackList[trackIdx]->EnablePlayChannel(channel);
	}

	void MidiPlay::DisableTrackAllChannels(int trackIdx)
	{
		if (trackIdx >= trackList.size())
			return;

		for (int i = 0; i < 16; i++)
			trackList[trackIdx]->DisablePlayChannel(i);
	}

	void MidiPlay::EnableTrackAllChannels(int trackIdx)
	{
		if (trackIdx >= trackList.size())
			return;

		for (int i = 0; i < 16; i++)
			trackList[trackIdx]->EnablePlayChannel(i);
	}



	//轨道运行
	void MidiPlay::TrackRun(double sec)
	{
		if (state != MidiPlayState::PLAY)
			return;

		if (isOpen == false)
		{
			isOpen = true;
			list<MidiEvent*>* eventList;
			for (int i = 0; i < trackList.size(); i++)
			{
				eventList = (*midiTrackList)[i]->GetEventList();
				trackList[i]->eventOffsetIter = eventList->begin();
				trackList[i]->baseTickTime = sec;
				trackList[i]->SetPercussionProgramNum(percussionProgramNum);
			}

			assistTrack->baseTickTime = sec;

			if (gotoSec > 0)
			{
				isDirectGoto = true;
				TrackPlayCore(gotoSec + sec);
				isDirectGoto = false;
				isGotoEnd = false;
			}
		}

		TrackPlayCore(gotoSec + sec);
	}


	void MidiPlay::TrackPlayCore(double sec)
	{
		list<MidiEvent*>* eventList;
		MidiEvent* ev;
		int trackEndCount = 0;

		for (int i = 0; i < trackList.size(); i++)
		{
			if (trackList[i]->isEnded) {
				trackEndCount++;
				continue;
			}

			trackList[i]->CalCurtTicksCount(sec);

			eventList = (*midiTrackList)[i]->GetEventList();
			list<MidiEvent*>::iterator it = trackList[i]->eventOffsetIter;
			list<MidiEvent*>::iterator end = eventList->end();
			for (; it != end; it++)
			{
				ev = *it;

				//
				while (trackList[i]->NeedSettingTempo() &&
					ev->startTick >= trackList[i]->GetSettingStartTickCount())
				{
					trackList[i]->SetTempoBySetting();
					trackList[i]->CalCurtTicksCount(sec);
				}

				//
				if (!isGotoEnd && ev->startTick > trackList[i]->curtTickCount)
				{
					trackList[i]->eventOffsetIter = it;
					break;
				}


				ProcessTrackEvent(ev, i, sec);
			}

			if (it == end)
			{
				trackList[i]->isEnded = true;

				//轨道播发结束后，清除相关设置
				for (int j = 0; j < 16; j++)
				{
					Channel* channel = (*trackList[i])[j];
					if (channel != nullptr) {
						channel->SetControllerValue(MidiControllerType::SustainPedalOnOff, 0);
						ventrue->ModulationVirInstParams(channel);
					}
				}
			}
		}

		//检测播放是否结束
		if (!isComputeEventTime && trackEndCount == trackList.size())
		{
			bool isEnd = true;
			for (int i = 0; i < trackList.size(); i++)
			{
				for (int j = 0; j < 16; j++) {
					Channel* channel = trackList[i]->channels[j];
					VirInstrument* inst = ventrue->GetVirInstrumentByChannel(channel);
					if (inst == nullptr || inst->IsSoundEnd())
						continue;

					isEnd = false;
					break;
				}
			}

			if (isEnd) {
				//printf("播放结束! \n");
				Stop();
			}
		}


		//处理辅助播放轨道的midi事件
		assistTrack->CalCurtTicksCount(sec);
		int j = assistTrack->eventOffsetIdx;
		for (; j < assistMidiEvList.size(); j++)
		{
			if (!isGotoEnd && assistMidiEvList[j]->startTick > assistTrack->curtTickCount)
			{
				assistTrack->eventOffsetIdx = j;
				break;
			}

			ProcessTrackEvent(assistMidiEvList[j], 0, sec);
		}
	}

	//处理轨道事件
	void MidiPlay::ProcessTrackEvent(MidiEvent* midEv, int trackIdx, double sec)
	{
		if (isComputeEventTime &&
			midEv->type != MidiEventType::NoteOn)
		{
			midEv->endSec = midEv->startSec =
				trackList[trackIdx]->GetTickSec(midEv->startTick);

			if (midEv->endSec > endSec)
				endSec = midEv->endSec;
		}

		//
		switch (midEv->type)
		{
		case MidiEventType::Tempo:
		{
			TempoEvent* tempoEv = (TempoEvent*)midEv;

			// 设置轨道速度
			trackList[trackIdx]->SetTempo(tempoEv->microTempo, midiFile->GetTickForQuarterNote(), tempoEv->startTick);
			trackList[trackIdx]->CalCurtTicksCount(sec);

			if (midiFile->GetFormat() == MidiFileFormat::SyncTracks)
			{
				for (int i = 0; i < trackList.size(); i++)
				{
					if (i == trackIdx)
						continue;

					trackList[i]->AddTempoSetting(tempoEv->microTempo, midiFile->GetTickForQuarterNote(), tempoEv->startTick);
				}
			}
		}
		break;

		case MidiEventType::NoteOn:
		{
			NoteOnEvent* noteOnEv = (NoteOnEvent*)midEv;

			if (isComputeEventTime)
				noteOnEv->startSec = trackList[trackIdx]->GetTickSec(noteOnEv->startTick);

			if (isDirectGoto || trackList[trackIdx]->isDisablePlay)
				break;

			Channel& channel = *(*trackList[trackIdx])[noteOnEv->channel];
			if (channel.IsDisablePlay())
				break;

			Preset* preset = ventrue->GetInstrumentPreset(channel.GetBankSelectMSB(), channel.GetBankSelectLSB(), channel.GetProgramNum());
			if (preset == nullptr)
			{
				preset = ventrue->GetInstrumentPreset(0, 0, channel.GetProgramNum());
				if (preset == nullptr)
					return;
			}

			VirInstrument* virInst = ventrue->EnableVirInstrument(preset, &channel);
			virInst->SetType(VirInstrumentType::MidiTrackType);
			virInst->OnKey(noteOnEv->note, (float)noteOnEv->velocity, noteOnEv->endTick - noteOnEv->startTick + 1, false);
			//printf("开始:%.2f秒 - 结束:%.2f秒 \n", noteOnEv->startSec, noteOnEv->endSec);

			//对缺少对应关闭音符事件的NoteOn，在辅助轨道上添加一个0.5s后关闭的事件
			if (noteOnEv->noteOffEvent == nullptr)
			{
				NoteOffEvent* noteOffEvent = new NoteOffEvent();
				noteOffEvent->startTick = assistTrack->GetTickCount(sec + 0.5f);
				noteOffEvent->note = noteOnEv->note;
				noteOffEvent->velocity = 127;
				noteOffEvent->channel = noteOnEv->channel;
				noteOffEvent->noteOnEvent = noteOnEv;
				assistMidiEvList.push_back(noteOffEvent);
			}
		}
		break;

		case MidiEventType::NoteOff:
		{
			NoteOffEvent* noteOffEv = (NoteOffEvent*)midEv;
			if (noteOffEv->noteOnEvent == nullptr)
				break;

			if (isComputeEventTime) {
				noteOffEv->noteOnEvent->endSec = midEv->endSec;

				if (midEv->endSec > endSec)
					endSec = midEv->endSec;
			}

			if (isDirectGoto || trackList[trackIdx]->isDisablePlay)
				break;


			Channel& channel = *(*trackList[trackIdx])[noteOffEv->channel];
			if (channel.IsDisablePlay())
				break;

			Preset* preset = ventrue->GetInstrumentPreset(channel.GetBankSelectMSB(), channel.GetBankSelectLSB(), channel.GetProgramNum());
			if (preset == nullptr)
			{
				preset = ventrue->GetInstrumentPreset(0, 0, channel.GetProgramNum());
				if (preset == nullptr)
					return;
			}

			VirInstrument* virInst = ventrue->EnableVirInstrument(preset, &channel);
			virInst->OffKey(noteOffEv->note, (float)noteOffEv->velocity, false);

		}
		break;

		case MidiEventType::ProgramChange:
		{
			ProgramChangeEvent* ev = (ProgramChangeEvent*)midEv;
			Channel& channel = *(*trackList[trackIdx])[ev->channel];

			//通道9为鼓点音色，一般不需要选择
			if (ev->channel == 9)
				return;

			channel.SetProgramNum(ev->value);
		}
		break;

		case MidiEventType::PitchBend:
		{
			PitchBendEvent* ev = (PitchBendEvent*)midEv;
			Channel* channel = (*trackList[trackIdx])[ev->channel];
			channel->SetPitchBend(ev->value);
			ventrue->ModulationVirInstParams(channel);
		}
		break;

		case MidiEventType::Controller:
		{
			ControllerEvent* ev = (ControllerEvent*)midEv;
			Channel* channel = (*trackList[trackIdx])[ev->channel];
			channel->SetControllerValue(ev->ctrlType, ev->value);
			ventrue->ModulationVirInstParams(channel);
		}
		break;
		}
	}

}
