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


namespace tau
{
	//设置轨道事件演奏方式
	void Editor::SetTrackPlayType(int trackIdx, MidiEventPlayType playType)
	{
		TauLock(tau);
		synther->SetTrackPlayType(midiEditor->tracks[trackIdx], playType);
	}

	//设置演奏类型
	void Editor::SetPlayType(MidiEventPlayType playType)
	{
		TauLock(tau);
		synther->SetPlayType(playType);
	}


	//是否有等待中的按键
	bool Editor::HavWaitKey()
	{
		TauLock(tau);
		return needOnKeyCount > 0;
	}

	//是否是等待中的按键
	bool Editor::IsWaitKey(int key)
	{
		TauLock(tau);
		return needOnkey[key] > 0;
	}

	//获取等待中按键的数量
	int Editor::GetWaitKeyCount(int key)
	{
		TauLock(tau);
		return needOnkey[key];
	}

	//是否是等待中的按键事件
	bool Editor::IsWaitNoteOnEvent(NoteOnEvent* noteOnEv)
	{
		TauLock(tau);
	
		if (needOnkey[noteOnEv->note] == 0)
			return false;

		auto node = needOnKeyEventList[noteOnEv->note].GetHeadNode();
		for (; node; node = node->next) {
			if (noteOnEv == node->elem)
				return true;
		}

		return false;
	}


	//进入到步进播放模式
	void Editor::EnterStepPlayMode()
	{
		if (playMode == EditorPlayMode::Step) {
			printf("当前模式已经为步进播放模式! \n");
			return;
		}

		TauLock(tau);
		playMode = EditorPlayMode::Step;
		isWait = false;
		synther->EnterPlayMode(EditorPlayMode::Step);
	}


	//进入到等待播放模式
	void Editor::EnterWaitPlayMode()
	{
		if (playMode == EditorPlayMode::Wait) {
			printf("当前模式已经为等待播放模式! \n");
			return;
		}

		TauLock(tau);
		playMode = EditorPlayMode::Wait;
		ClearPlayModeData();

		synther->EnterPlayMode(EditorPlayMode::Wait);
	}


	//进入到静音模式
	void Editor::EnterMuteMode()
	{
		if (playMode == EditorPlayMode::Mute) {
			printf("当前模式已经为静音模式! \n");
			return;
		}

		TauLock(tau);
		playMode = EditorPlayMode::Mute;
		ClearPlayModeData();

		synther->EnterPlayMode(EditorPlayMode::Mute);

	}

	//离开当前播放模式
	void Editor::LeavePlayMode()
	{
		TauLock(tau);
		playMode = EditorPlayMode::Common;
		synther->LeavePlayMode();
	}


	//移动到指定时间点
	void Editor::Runto(double sec)
	{
		TauLock(tau);
		//如果是等待播放模式，将清空等待播放模式的数据
		if (playMode == EditorPlayMode::Wait)
			ClearPlayModeData();

		synther->Runto(sec);
	}

	void Editor::ClearPlayModeData()
	{
		for (int i = 0; i < 128; i++) {
			auto node = needOnKeyEventList[i].GetHeadNode();
			if (node == &(needOnKeyEventNode[i]))
				needOnKeyEventList[i].Remove(node);
			needOnKeyEventList[i].Release();
		}

		memset(onkey, 0, sizeof(int) * 128);
		memset(needOnkey, 0, sizeof(int) * 128);
		memset(needOffkey, 0, sizeof(int) * 128);

		//
		needOnKeyCount = 0;
		needOffKeyCount = 0;
		onKeyCount = 0;
		isWait = false;
	}

	//等待(区别于暂停，等待相当于在原始位置播放)
	void Editor::Wait()
	{
		isWait = true;
	}

	//继续，相对于等待命令
	void Editor::Continue()
	{
		isWait = false;
	}

	//设置排除需要等待的按键
	void Editor::SetExcludeNeedWaitKeys(int* excludeKeys, int size)
	{
		for (int i = 0; i < size; i++)
			SetExcludeNeedWaitKey(excludeKeys[i]);

	}

	//设置包含需要等待的按键
	void Editor::SetIncludeNeedWaitKey(int key)
	{
		excludeNeedWaitKey[key] = false;
	}

	//设置排除需要等待的按键
	void Editor::SetExcludeNeedWaitKey(int key)
	{
		TauLock(tau);
		if (excludeNeedWaitKey[key] == true)
			return;

		excludeNeedWaitKey[key] = true;
		if (needOnkey[key] != 0)
		{
			NoteOnEvent* noteOnEv;
			auto node = needOnKeyEventList[key].GetHeadNode();
			for (; node; node = node->next) {
				noteOnEv = node->elem;
				OnKey(key, noteOnEv->velocity, noteOnEv->track);
			}

			needOnKeyEventList[key].Release();
			needOnKeyCount -= needOnkey[key];
			needOnkey[key] = 0;
			if (needOnKeyCount == 0)
				isWait = false;
		}
	}


	//需要按键信号
	void Editor::NeedOnKeySignal(int key, NoteOnEvent* noteOnEv)
	{
		if (playMode != EditorPlayMode::Wait)
			return;

		if (onkey[key] > 0) {
			onkey[key]--;
			onKeyCount--;
			return;
		}

		needOnkey[key]++;
		needOnKeyCount++;

		if (noteOnEv != nullptr) {

			if (needOnkey[key] == 1) {
				needOnKeyEventNode[key].elem = noteOnEv;
				needOnKeyEventList[key].AddLast(&needOnKeyEventNode[key]);
			}
			else {
				needOnKeyEventList[key].AddLast(noteOnEv);
			}
		}

		isWait = true;
		//printf("等待按键:%d \n", key);
	}

	//需要松开按键信号
	void Editor::NeedOffKeySignal(int key)
	{
		if (playMode != EditorPlayMode::Wait)
			return;


		/*
		if (onkey[key] == 0)
			return;

		needOffkey[key]++;
		needOffKeyCount++;*/
		//isWait = true;
		//printf("等待松开键:%d \n", key);
	}

	//按键信号
	void Editor::OnKeySignal(int key)
	{
		if (playMode != EditorPlayMode::Wait)
			return;

		TauLock(tau);
	
		//printf("按下按键:%d \n", key);
		onkey[key]++;
		onKeyCount++;


		if (needOnkey[key] > 0) //存在需要对应按下的按键
		{
			onkey[key]--;
			onKeyCount--;

			//
			auto node = needOnKeyEventList[key].GetHeadNode();
			needOnKeyEventList[key].Remove(node);
			node->elem = nullptr;
			if (node != &(needOnKeyEventNode[key]))
				DEL(node);

			needOnkey[key]--;
			needOnKeyCount--;
			if (needOnKeyCount == 0 && needOffKeyCount == 0)
				isWait = false;
		}
	}

	//所有等待按键信号
	void Editor::OnWaitKeysSignal()
	{
		TauLock(tau);
		if (needOnKeyCount == 0)
			return;

		for (int i = 0; i < 128; i++)
		{
			int count = needOnkey[i];
			for (int j = 0; j < count; j++)
				OnKeySignal(i);
		}
	}



	//松开按键信号
	void Editor::OffKeySignal(int key)
	{
		if (playMode != EditorPlayMode::Wait)
			return;

		TauLock(tau);
		if (onkey[key] > 0)
		{
			//printf("松开按键:%d \n", key);
			onkey[key]--;
			onKeyCount--;

			if (needOffkey[key] > 0) {
				needOffkey[key]--;
				needOffKeyCount--;
			}

			if (needOnKeyCount == 0 && needOffKeyCount == 0 && onKeyCount == 0) {
				isWait = false;
			}
		}
	}

}