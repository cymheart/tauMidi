#include"Tau.h"
#include"Midi/MidiFile.h"
#include"MidiTrackRecord.h"
#include"Synther/MidiEditorSynther.h"
#include"Editor/Editor.h"

namespace tau
{

	//载入
	void Tau::Load(string& midiFilePath)
	{
		editor->Load(midiFilePath);
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


	// 指定midi文件播放的起始时间点
	void Tau::Goto(float sec)
	{
		editor->Goto(sec);
	}

	//获取midi状态
	EditorState Tau::GetEditorState()
	{
		return editor->GetState();
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
	void Tau::EnableAllMidiTrack()
	{

	}


	// 禁止播放指定编号Midi文件通道
	void Tau::DisableMidiChannel(int channelIdx)
	{

	}

	// 启用播放指定编号Midi文件通道
	void Tau::EnableMidiChannel(int channelIdx)
	{

	}

	// 设置midi对应轨道的乐器
	void Tau::SetMidiVirInstrument(int trackIdx,
		int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{

	}


	//为midi文件设置打击乐号
	void Tau::SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{

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

		for (int i = 0; i < syntherCount; i++)
			midiEditorSynthers[i]->RecordMidiTask();
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

		for (int i = 0; i < syntherCount; i++)
			midiEditorSynthers[i]->RecordMidiTask(virInst);
	}

	/// <summary>
	/// 停止所有乐器当前midi录制
	/// </summary>
	void Tau::StopRecordMidi()
	{
		if (!isOpened)
			return;

		for (int i = 0; i < syntherCount; i++)
			midiEditorSynthers[i]->StopRecordMidiTask();

	}

	/// <summary>
	/// 停止录制指定乐器弹奏midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	void Tau::StopRecordMidi(VirInstrument* virInst)
	{
		if (!isOpened)
			return;

		for (int i = 0; i < syntherCount; i++)
			midiEditorSynthers[i]->StopRecordMidiTask(virInst);
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
		vector<MidiTrack*>* midiTracks = nullptr;
		for (int i = 0; i < syntherCount; i++)
		{
			midiTracks = midiEditorSynthers[i]->TakeRecordMidiTracksTask(virInsts, size, recordMidiTickForQuarterNote, &recordTempos);
			if (midiTracks != nullptr)
			{
				for (int j = 0; j < midiTracks->size(); j++)
				{
					(*midiTracks)[j]->SetMidiEventsChannel(j % 16);
					midiFile->AddMidiTrack((*midiTracks)[j]);
				}

				DEL(midiTracks);
			}
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
