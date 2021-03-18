#include"VentrueCmd.h"

namespace ventrue
{
	VentrueCmd::VentrueCmd(Ventrue* ventrue)
	{
		this->ventrue = ventrue;
		ventrueEventPool = &(VentruePool::GetInstance().VentrueEventPool());
	}

	//处理任务
	void VentrueCmd::ProcessTask(TaskCallBack taskCallBack, void* data, int delay)
	{
		ventrue->PostTask(taskCallBack, data, delay);
	}

	//// 按下按键
	void VentrueCmd::OnKey(int key, float velocity, VirInstrument* virInst)
	{

		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::OnKey;
		ev->processCallBack = _OnKey;
		ev->key = key;
		ev->velocity = velocity;
		ev->virInst = virInst;
		ventrue->PostRealtimeKeyOpTask(ev);
	}

	void VentrueCmd::_OnKey(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		ventrue.OnKey(ventrueEvent->key, ventrueEvent->velocity, ventrueEvent->virInst);
	}

	// 释放按键
	void VentrueCmd::OffKey(int key, float velocity, VirInstrument* virInst)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::OffKey;
		ev->processCallBack = _OffKey;
		ev->key = key;
		ev->velocity = velocity;
		ev->virInst = virInst;
		ventrue->PostRealtimeKeyOpTask(ev);
	}

	void VentrueCmd::_OffKey(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		ventrue.OffKey(ventrueEvent->key, ventrueEvent->velocity, ventrueEvent->virInst);
	}


	//增加效果器
	void VentrueCmd::AddEffect(VentrueEffect* effect)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::Unknown;
		ev->processCallBack = _AddEffect;
		ev->ptr = effect;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_AddEffect(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		ventrue.AddEffect((VentrueEffect*)ventrueEvent->ptr);
	}

	// 发送文本
	void VentrueCmd::SendText(string text)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::Text;
		ev->processCallBack = _SendText;
		ev->text.assign(text);
		ev->midiFile = nullptr;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_SendText(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		cout << ventrueEvent->text << "(ms)" << endl;
	}


	// 添加Midi文件
	void VentrueCmd::AppendMidiFile(string midifile)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::AppendMidiFile;
		ev->processCallBack = _AppendMidi;
		ev->midiFilePath.assign(midifile);
		ev->midiFile = nullptr;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_AppendMidi(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		MidiFile* midiFile = new MidiFile();
		midiFile->Parse(ventrueEvent->midiFilePath);
		ventrue.midiFileList->push_back(midiFile);
	}


	// 添加并播放Midi文件
	void VentrueCmd::PlayMidiFile(string midifile)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::PlayMidiFile;
		ev->processCallBack = _PlayMidi;
		ev->midiFilePath.assign(midifile);
		ev->midiFile = nullptr;
		ventrue->PostTask(ev);
	}

	// 请求播放Midi文件
	void VentrueCmd::PlayMidi(MidiFile* midiFile)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::PlayMidi;
		ev->processCallBack = _PlayMidi;
		ev->midiFile = midiFile;
		ventrue->PostTask(ev);
	}

	// 播放指定编号的内部Midi文件
	void VentrueCmd::PlayMidi(int midiFileIdx)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::PlayMidiIdx;
		ev->processCallBack = _PlayMidi;
		ev->midiFile = nullptr;
		ev->midiFileIdx = midiFileIdx;
		ventrue->PostTask(ev);
	}


	void VentrueCmd::_PlayMidi(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		MidiFile* midiFile = nullptr;

		if (ventrueEvent->evType == VentrueEventType::PlayMidiFile)
		{
			midiFile = new MidiFile();
			midiFile->Parse(ventrueEvent->midiFilePath);
			ventrue.midiFileList->push_back(midiFile);
		}
		else if (ventrueEvent->evType == VentrueEventType::PlayMidi)
		{
			midiFile = ventrueEvent->midiFile;
		}
		else if (ventrueEvent->evType == VentrueEventType::PlayMidiIdx)
		{
			if (ventrueEvent->midiFileIdx >= 0 &&
				ventrueEvent->midiFileIdx < ventrue.midiFileList->size())
			{
				midiFile = (*ventrue.midiFileList)[ventrueEvent->midiFileIdx];
			}
		}

		if (midiFile == nullptr) {
			return;
		}

		MidiPlay* midiPlay = new MidiPlay();
		midiPlay->SetVentrue(&ventrue);
		midiPlay->SetMidiFile(midiFile);
		ventrue.midiPlayList->push_back(midiPlay);
	}


	// 禁止播放指定编号Midi文件的轨道
	void VentrueCmd::DisableMidiTrack(int midiFileIdx, int trackIdx)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::DisablePlayMidiTrack;
		ev->processCallBack = _DisableMidiTrack;
		ev->midiFile = nullptr;
		ev->midiFileIdx = midiFileIdx;
		ev->midiTrackIdx = trackIdx;
		ventrue->PostTask(ev);
	}

	// 禁止播放Midi的所有轨道
	void VentrueCmd::DisableAllMidiTrack(int midiFileIdx)
	{
		DisableMidiTrack(midiFileIdx, -1);
	}


	void VentrueCmd::_DisableMidiTrack(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		MidiFile* midiFile = nullptr;

		if (ventrueEvent->evType == VentrueEventType::DisablePlayMidiTrack)
		{
			if (ventrue.midiPlayList->size() <= ventrueEvent->midiFileIdx) {
				return;
			}

			if (ventrueEvent->midiTrackIdx == -1)
				(*ventrue.midiPlayList)[ventrueEvent->midiFileIdx]->DisableAllTrack();
			else
				(*ventrue.midiPlayList)[ventrueEvent->midiFileIdx]->DisableTrack(ventrueEvent->midiTrackIdx);
		}

	}


	// 启用播放指定编号Midi文件的轨道
	void VentrueCmd::EnableMidiTrack(int midiFileIdx, int trackIdx)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::DisablePlayMidiTrack;
		ev->processCallBack = _EnableMidiTrack;
		ev->midiFile = nullptr;
		ev->midiFileIdx = midiFileIdx;
		ev->midiTrackIdx = trackIdx;
		ventrue->PostTask(ev);
	}

	// 启用播放Midi的所有轨道
	void VentrueCmd::EnableAllMidiTrack(int midiFileIdx)
	{
		EnableMidiTrack(midiFileIdx, -1);
	}

	void VentrueCmd::_EnableMidiTrack(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);

		if (ventrueEvent->evType == VentrueEventType::DisablePlayMidiTrack)
		{
			if (ventrue.midiPlayList->size() <= ventrueEvent->midiFileIdx) {
				return;
			}

			if (ventrueEvent->midiTrackIdx == -1)
				(*ventrue.midiPlayList)[ventrueEvent->midiFileIdx]->EnableAllTrack();
			else
				(*ventrue.midiPlayList)[ventrueEvent->midiFileIdx]->EnableTrack(ventrueEvent->midiTrackIdx);
		}

	}


	// 指定midi文件播放的起始时间点
	void VentrueCmd::MidiGotoSec(int midiFileIdx, float sec)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::PlayMidiGoto;
		ev->processCallBack = _MidiGotoSec;
		ev->midiFile = nullptr;
		ev->midiFileIdx = midiFileIdx;
		ev->sec = sec;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_MidiGotoSec(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);

		if (ventrue.midiPlayList->size() <= ventrueEvent->midiFileIdx)
			return;

		(*ventrue.midiPlayList)[ventrueEvent->midiFileIdx]->GotoSec(ventrueEvent->sec);
	}


	// 设置设备通道Midi控制器值
	void VentrueCmd::SetDeviceChannelMidiControllerValue(int deviceChannelNum, MidiControllerType midiController, int value)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::SetDeviceChannelMidiControllerValue;
		ev->processCallBack = _SetDeviceChannelMidiControllerValue;
		ev->deviceChannelNum = deviceChannelNum;
		ev->midiCtrlType = midiController;
		ev->value = value;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_SetDeviceChannelMidiControllerValue(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);

		Channel* channel = ventrue.GetDeviceChannel(ventrueEvent->deviceChannelNum);
		if (channel == nullptr) {
			return;
		}

		channel->SetControllerValue(ventrueEvent->midiCtrlType, ventrueEvent->value);
		ventrue.ModulationVirInstParams(channel);
	}


	/// <summary>
	/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在将在虚拟乐器列表中自动创建它
	/// 注意如果deviceChannelNum已近被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
	/// 而不会同时在一个通道上创建超过1个的虚拟乐器
	/// </summary>
	/// <param name="deviceChannel">乐器所在的设备通道</param>
	/// <param name="bankSelectMSB">声音库选择0</param>
	/// <param name="bankSelectLSB">声音库选择1</param>
	 /// <param name="instrumentNum">乐器编号</param>
	/// <returns></returns>
	VirInstrument* VentrueCmd::EnableVirInstrument(uint32_t deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		thread_local VirInstrument* inst = 0;
		thread_local Semaphore waitGetInstrumentSem;

		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::EnableInstrument;
		ev->processCallBack = _EnableInstrument;
		ev->deviceChannelNum = deviceChannelNum;
		ev->bankSelectLSB = bankSelectLSB;
		ev->bankSelectMSB = bankSelectMSB;
		ev->instrumentNum = instrumentNum;
		ev->ptr = (void*)&inst;
		ev->sem = &waitGetInstrumentSem;
		ventrue->PostTask(ev);

		//
		waitGetInstrumentSem.wait();
		return inst;
	}

	void VentrueCmd::_EnableInstrument(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		VirInstrument* vinst =
			ventrue.EnableVirInstrument(
				ventrueEvent->deviceChannelNum,
				ventrueEvent->bankSelectMSB,
				ventrueEvent->bankSelectLSB,
				ventrueEvent->instrumentNum);

		VirInstrument** threadVInst = (VirInstrument**)ventrueEvent->ptr;
		*threadVInst = vinst;
		ventrueEvent->sem->set();
	}

	/// <summary>
	/// 录制所有乐器弹奏为midi
	/// </summary>
	/// <param name="bpm">录制的BPM</param>
	 /// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
	void VentrueCmd::RecordMidi(float bpm, float tickForQuarterNote)
	{
		RecordMidi(nullptr, bpm, tickForQuarterNote);
	}

	/// <summary>
	/// 录制指定乐器弹奏为midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	/// <param name="bpm">录制的BPM</param>
	/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
	void VentrueCmd::RecordMidi(VirInstrument* virInst, float bpm, float tickForQuarterNote)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::RecordMidi;
		ev->virInst = virInst;
		ev->bpm = bpm;
		ev->tickForQuarterNote = tickForQuarterNote;
		ev->processCallBack = _RecordMidi;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_RecordMidi(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		ventrue.RecordMidi(ventrueEvent->virInst, ventrueEvent->bpm, ventrueEvent->tickForQuarterNote);
	}

	/// <summary>
	/// 停止所有乐器当前midi录制
	/// </summary>
	void VentrueCmd::StopRecordMidi()
	{
		StopRecordMidi(nullptr);
	}

	/// <summary>
	/// 停止录制指定乐器弹奏midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	void VentrueCmd::StopRecordMidi(VirInstrument* virInst)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::StopRecordMidi;
		ev->virInst = virInst;
		ev->processCallBack = _StopRecordMidi;
		ventrue->PostTask(ev);
	}


	void VentrueCmd::_StopRecordMidi(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		ventrue.StopRecordMidi(ventrueEvent->virInst);
	}

	/// <summary>
	/// 生成所有乐器已录制的midi到midiflie object中
	/// </summary>
	/// <returns>midiflie object</returns>
	MidiFile* VentrueCmd::CreateRecordMidiFileObject()
	{
		return CreateRecordMidiFileObject(nullptr, 0);
	}

	/// <summary>
	/// 根据给定的乐器，生成它的已录制的midi到midiflie object中
	/// </summary>
	/// <param name="virInsts">乐器</param>
	/// <returns>midiflie object</returns>
	MidiFile* VentrueCmd::CreateRecordMidiFileObject(VirInstrument* virInst)
	{
		return CreateRecordMidiFileObject(&virInst, 1);
	}

	/// <summary>
	/// 根据给定的乐器组，生成它们已录制的midi到midiflie object中
	/// </summary>
	/// <param name="virInsts">乐器组</param>
	/// <returns>midiflie object</returns>
	MidiFile* VentrueCmd::CreateRecordMidiFileObject(VirInstrument** virInsts, int size)
	{
		thread_local MidiFile* midiFile = 0;
		thread_local Semaphore waitSem;

		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::CreateRecordMidiFileObject;
		ev->processCallBack = _CreateRecordMidiFileObject;
		ev->ptr = (void*)&midiFile;
		ev->exPtr[0] = virInsts;
		ev->exValue[0] = (float)size;
		ev->sem = &waitSem;
		ventrue->PostTask(ev);

		//
		waitSem.wait();
		return midiFile;

	}

	void VentrueCmd::_CreateRecordMidiFileObject(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		MidiFile* midiFile = ventrue.CreateRecordMidiFileObject((VirInstrument**)ventrueEvent->exPtr[0], (int)(ventrueEvent->exValue[0]));
		MidiFile** threadMidiFile = (MidiFile**)ventrueEvent->ptr;
		*threadMidiFile = midiFile;
		ventrueEvent->sem->set();
	}

	//保存midiFile到文件
	void VentrueCmd::SaveMidiFileToDisk(MidiFile* midiFile, string saveFilePath)
	{
		VentrueEvent* ev = ventrueEventPool->Pop();
		ev->ventrue = ventrue;
		ev->evType = VentrueEventType::SaveMidiFileToDisk;
		ev->midiFile = midiFile;
		ev->midiFilePath = saveFilePath;
		ev->processCallBack = _SaveMidiFileToDisk;
		ventrue->PostTask(ev);
	}

	void VentrueCmd::_SaveMidiFileToDisk(Task* ev)
	{
		VentrueEvent* ventrueEvent = (VentrueEvent*)ev;
		Ventrue& ventrue = *(ventrueEvent->ventrue);
		ventrue.SaveMidiFileToDisk(ventrueEvent->midiFile, ventrueEvent->midiFilePath);
	}

}
