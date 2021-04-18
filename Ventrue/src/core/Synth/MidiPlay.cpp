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
	MidiPlay::MidiPlay()
	{
		//noteOnEvOnKeyInfos = new NoteOnEvOnKeyInfoList;
		assistTrack = new Track;
	}

	MidiPlay::~MidiPlay()
	{
		for (int i = 0; i < trackList.size(); i++)
			DEL(trackList[i]);

		for (int i = 0; i < assistMidiEvList.size(); i++)
			DEL(assistMidiEvList[i]);

		DEL(assistTrack);
		//DEL(noteOnEvOnKeyInfos);
	}

	/// <summary>
	/// 设置发音合成器ventrue
	/// </summary>
	/// <param name="ventrue"></param>
	void MidiPlay::SetVentrue(Ventrue* ventrue)
	{
		this->ventrue = ventrue;
	}



	/// <summary>
	/// 设置MidiFile
	/// </summary>
	/// <param name="midiFile"></param>
	void MidiPlay::SetMidiFile(MidiFile* midiFile)
	{
		this->midiFile = midiFile;
		midiTrackList = midiFile->GetTrackList();
		Clear();
	}

	void MidiPlay::Stop()
	{
		Clear();
	}


	void MidiPlay::Clear()
	{
		isDirectGoto = false;
		isOpen = false;
		startTime = 0;

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
				trackList.push_back(new Track());
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

	//设置播放的起始时间点
	void MidiPlay::GotoSec(double gotoSec)
	{
		Stop();
		this->gotoSec = gotoSec;
	}

	//轨道播放
	void MidiPlay::TrackPlay(double sec)
	{
		if (isOpen == false)
		{
			isOpen = true;
			for (int i = 0; i < trackList.size(); i++)
				trackList[i]->startSec = sec;

			assistTrack->startSec = sec;

			if (gotoSec > 0)
			{
				isDirectGoto = true;
				TrackPlayCore(sec + gotoSec);
				isDirectGoto = false;
			}
		}

		TrackPlayCore(sec + gotoSec);
	}


	void MidiPlay::TrackPlayCore(double sec)
	{
		MidiEventList* eventList;
		for (int i = 0; i < trackList.size(); i++)
		{
			if (trackList[i]->isEnded)
				continue;

			trackList[i]->CalCurtTicksCount(sec);

			eventList = (*midiTrackList)[i]->GetEventList();
			int j = trackList[i]->eventOffsetIdx;

			for (; j < eventList->size(); j++)
			{
				if ((*eventList)[j]->startTick > trackList[i]->curtTickCount)
				{
					trackList[i]->eventOffsetIdx = j;
					break;
				}

				ProcessTrackEvent((*eventList)[j], i, sec);
			}

			if (j == eventList->size())
			{
				trackList[i]->isEnded = true;
			}
		}


		//处理辅助播放轨道的midi事件
		assistTrack->CalCurtTicksCount(sec);
		int j = assistTrack->eventOffsetIdx;
		for (; j < assistMidiEvList.size(); j++)
		{
			if (assistMidiEvList[j]->startTick > assistTrack->curtTickCount)
			{
				assistTrack->eventOffsetIdx = j;
				break;
			}

			ProcessTrackEvent(assistMidiEvList[j], i, sec);
		}
	}

	//处理轨道事件
	void MidiPlay::ProcessTrackEvent(MidiEvent* midEv, int trackIdx, double sec)
	{
		switch (midEv->type)
		{
		case MidiEventType::Tempo:
		{
			TempoEvent* tempoEv = (TempoEvent*)midEv;

			// 设置轨道速度
			trackList[trackIdx]->SetTempo(tempoEv->microTempo, midiFile->GetTickForQuarterNote(), sec);

			if (midiFile->GetFormat() == MidiFileFormat::SyncTracks)
			{
				for (int i = 1; i < trackList.size(); i++)
				{
					trackList[i]->SetTempo(tempoEv->microTempo, midiFile->GetTickForQuarterNote(), sec);
				}
			}
		}
		break;

		case MidiEventType::NoteOn:
		{
			if (isDirectGoto || trackList[trackIdx]->isDisablePlay)
				break;

			NoteOnEvent* noteOnEv = (NoteOnEvent*)midEv;
			Channel& channel = *(*trackList[trackIdx])[noteOnEv->channel];
			Preset* preset = ventrue->GetInstrumentPreset(channel.GetBankSelectMSB(), channel.GetBankSelectLSB(), channel.GetProgramNum());
			if (preset == nullptr)
				return;


			VirInstrument* virInst = ventrue->EnableVirInstrument(preset, &channel);
			virInst->OnKey(noteOnEv->note, (float)noteOnEv->velocity, noteOnEv->endTick - noteOnEv->startTick + 1, false);

			//printf("%s:%d\n", virInst->GetPreset()->name.c_str(), channel.GetProgramNum());
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
			if (isDirectGoto || trackList[trackIdx]->isDisablePlay)
				break;

			NoteOffEvent* noteOffEv = (NoteOffEvent*)midEv;
			Channel& channel = *(*trackList[trackIdx])[noteOffEv->channel];
			Preset* preset = ventrue->GetInstrumentPreset(channel.GetBankSelectMSB(), channel.GetBankSelectLSB(), channel.GetProgramNum());
			if (preset == nullptr)
				return;

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
