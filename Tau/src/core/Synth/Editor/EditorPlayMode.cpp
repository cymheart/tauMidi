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
		mainSynther->SetTrackPlayType(midiEditor->tracks[trackIdx], playType);
	}

	//设置演奏类型
	void Editor::SetPlayType(MidiEventPlayType playType)
	{
		TauLock(tau);
		mainSynther->SetPlayType(playType);
	}



	//获取当前时间之后需要等待按键信号的notes
	void Editor::GetNeedWaitKeySignalNote(int note, float lateSec)
	{
		mainSynther->GetCurTimeLateNeedWaitKeySignalNote(note, lateSec);
		lateNoteInfo = mainSynther->midiEditor->lateNoteInfo;
		noteOffLateNoteInfo = mainSynther->midiEditor->noteOffLateNoteInfo;
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
		mainSynther->EnterPlayMode(EditorPlayMode::Step);
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
		isWait = false;

		for (int i = 0; i < 128; i++)
		{
			needOnKeyTrack[i].clear();
			needOnKeyVelocity[i].clear();
		}

		memset(onkey, 0, sizeof(int) * 128);
		memset(needOnkey, 0, sizeof(int) * 128);
		memset(needOffkey, 0, sizeof(int) * 128);
		memset(needWaitKey, 0, sizeof(bool) * 128);

		needOnKeyCount = 0;
		needOffKeyCount = 0;
		onKeyCount = 0;

		mainSynther->EnterPlayMode(EditorPlayMode::Wait);
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
		isWait = false;

		for (int i = 0; i < 128; i++)
		{
			needOnKeyTrack[i].clear();
			needOnKeyVelocity[i].clear();
		}

		memset(onkey, 0, sizeof(int) * 128);
		memset(needOnkey, 0, sizeof(int) * 128);
		memset(needOffkey, 0, sizeof(int) * 128);
		memset(needWaitKey, 0, sizeof(bool) * 128);

		needOnKeyCount = 0;
		needOffKeyCount = 0;
		onKeyCount = 0;

		mainSynther->EnterPlayMode(EditorPlayMode::Mute);

	}

	//离开当前播放模式
	void Editor::LeavePlayMode()
	{
		TauLock(tau);
		playMode = EditorPlayMode::Common;
		mainSynther->LeavePlayMode();
	}


	//移动到指定时间点
	void Editor::Runto(double sec)
	{
		TauLock(tau);
		//如果是等待播放模式，将清空等待播放模式的数据
		if (playMode == EditorPlayMode::Wait) {

			for (int i = 0; i < 128; i++)
			{
				needOnKeyTrack[i].clear();
				needOnKeyVelocity[i].clear();
			}

			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			memset(needWaitKey, 0, sizeof(bool) * 128);

			needOnKeyCount = 0;
			needOffKeyCount = 0;
			onKeyCount = 0;
			isWait = false;
		}

		mainSynther->Runto(sec);
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
			Track* track;
			float vel;
			for (int j = needOnkey[key] - 1; j >= 0; j--) {
				track = needOnKeyTrack[key][j];
				vel = needOnKeyVelocity[key][j];
				OnKey(key, vel, track);
			}

			needOnKeyTrack[key].clear();
			needOnKeyVelocity[key].clear();

			needOnKeyCount -= needOnkey[key];
			needOnkey[key] = 0;
			if (needOnKeyCount == 0)
				isWait = false;
		}
	}


	//需要按键信号
	void Editor::NeedOnKeySignal(int key, float velocity, Track* track)
	{
		if (playMode != EditorPlayMode::Wait)
			return;

		if (onkey[key] > 0) {
			onkey[key]--;
			onKeyCount--;
			return;
		}

		needOnKeyTrack[key].push_back(track);
		needOnKeyVelocity[key].push_back(velocity);
		needOnkey[key]++;
		needOnKeyCount++;

		isWait = true;
		//printf("等待按键:%d \n", key);
	}

	//需要松开按键信号
	void Editor::NeedOffKeySignal(int key, float velocity, Track* track)
	{
		if (playMode != EditorPlayMode::Wait)
			return;

		if (needWaitKey[key]) {
			isWait = true;
			needWaitKey[key] = false;
		}

		/*
		if (onkey[key] == 0)
			return;

		needOffKeyTrack[key].push_back(track);
		needOffKeyVelocity[key].push_back(velocity);
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

		GetNeedWaitKeySignalNote(key, lateNoteSec);

		//printf("按下按键:%d \n", key);
		onkey[key]++;
		onKeyCount++;

		curtNeedOnKeyTrack = nullptr;
		curtNeedOnKeyVel = 1;

		if (needOnkey[key] > 0) //存在需要对应按下的按键
		{
			onkey[key]--;
			onKeyCount--;

			//
			curtNeedOnKeyTrack = needOnKeyTrack[key].front();
			needOnKeyTrack[key].pop_front();

			curtNeedOnKeyVel = needOnKeyVelocity[key].front();
			needOnKeyVelocity[key].pop_front();

			needOnkey[key]--;
			needOnKeyCount--;
			if (needOnKeyCount == 0 && needOffKeyCount == 0)
				isWait = false;
		}
		else if (lateNoteInfo.note >= 0)  //当前时间之后有这个按键事件
		{
			curtNeedOnKeyTrack = lateNoteInfo.track;
			curtNeedOnKeyVel = lateNoteInfo.vel;
		}
		else if (noteOffLateNoteInfo.note >= 0)
		{
			needWaitKey[key] = true;
			curtNeedOnKeyTrack = noteOffLateNoteInfo.track;
			curtNeedOnKeyVel = noteOffLateNoteInfo.vel;
		}
		else  //当前时间之后没有这个按键事件
		{
			isWait = true;
			//printf("按错键,等待中\n");
		}

	}

	//所有等待按键信号
	void Editor::OnWaitKeysSignal()
	{
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


		if (onkey[key] > 0)
		{
			//printf("松开按键:%d \n", key);
			onkey[key]--;
			onKeyCount--;

			needWaitKey[key] = false;

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