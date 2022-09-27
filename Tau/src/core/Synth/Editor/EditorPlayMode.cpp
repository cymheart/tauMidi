#include"Editor.h"
#include"Track.h"
#include"MidiEditor.h"
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
	//设置轨道事件演奏方式
	void Editor::SetTrackPlayType(int trackIdx, MidiEventPlayType playType)
	{
		waitSem.reset(0);
		MidiEditorSynther* synther = tracks[trackIdx]->GetMidiEditor()->GetSynther();
		synther->SetTrackPlayTypeTask(&waitSem, tracks[trackIdx], playType);
		waitSem.wait();
	}

	//设置演奏类型
	void Editor::SetPlayType(MidiEventPlayType playType)
	{
		//
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->SetPlayTypeTask(playType, &waitSem);
		waitSem.wait();
	}

	//获取当前时间之后的notekeys
	void Editor::GetCurTimeLateNoteKeys(float lateSec)
	{
		memset(lateHavKeys, 0, sizeof(bool) * 128);

		//
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->GetCurTimeLateNoteKeysTask(lateSec, &waitSem);
		waitSem.wait();

		//
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
		{
			vector<int>& noteKeys = tau->midiEditorSynthers[i]->midiEditor->tempNoteKeys;
			for (int i = 0; i < noteKeys.size(); i++)
				lateHavKeys[noteKeys[i]] = true;
		}

	}


	//进入到步进播放模式
	void Editor::EnterStepPlayMode()
	{
		if (playMode == EditorPlayMode::Step) {
			printf("当前模式已经为步进播放模式! \n");
			return;
		}

		if (playMode == EditorPlayMode::Wait) {
			printf("当前模式为:等待播放模式，需要离开等待播放模式! \n");
			return;
		}

		waitOnKeyLock.lock();
		playMode = EditorPlayMode::Step;
		isWait = false;
		waitOnKeyLock.unlock();

		//
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->EnterPlayModeTask(EditorPlayMode::Step, &waitSem);
		waitSem.wait();
	}


	//进入到等待播放模式
	void Editor::EnterWaitPlayMode()
	{
		if (playMode == EditorPlayMode::Wait) {
			printf("当前模式已经为等待播放模式! \n");
			return;
		}

		if (playMode == EditorPlayMode::Step) {
			printf("当前模式为:步进播放模式，需要离开步进播放模式! \n");
			return;
		}

		waitOnKeyLock.lock();
		playMode = EditorPlayMode::Wait;
		isWait = false;
		memset(onkey, 0, sizeof(int) * 128);
		memset(needOnkey, 0, sizeof(int) * 128);
		memset(needOffkey, 0, sizeof(int) * 128);
		needOnKeyCount = 0;
		needOffKeyCount = 0;
		waitOnKeyLock.unlock();

		//
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->EnterPlayModeTask(EditorPlayMode::Wait, &waitSem);
		waitSem.wait();

	}

	//离开当前播放模式
	void Editor::LeavePlayMode()
	{
		playMode = EditorPlayMode::Common;
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->LeavePlayModeTask(&waitSem);

		waitSem.wait();
	}


	//移动到指定时间点
	void Editor::Runto(double sec)
	{
		//如果是等待播放模式，将清空等待播放模式的数据
		if (playMode == EditorPlayMode::Wait) {
			waitOnKeyLock.lock();
			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			needOnKeyCount = 0;
			needOffKeyCount = 0;
			isWait = false;
			waitOnKeyLock.unlock();
		}


		//
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->RuntoTask(&waitSem, sec);

		waitSem.wait();
	}

	//等待(区别于暂停，等待相当于在原始位置播放)
	void Editor::Wait()
	{
		if (playMode != EditorPlayMode::Common)
			return;

		isWait = true;
	}

	//继续，相对于等待命令
	void Editor::Continue()
	{
		if (playMode != EditorPlayMode::Common)
			return;

		isWait = false;
	}

	//需要按键信号
	void Editor::NeedOnKeySignal(int key)
	{
		lock_guard<mutex> lock(waitOnKeyLock);

		if (onkey[key] > 0) {
			onkey[key]--;
			onKeyCount--;
			return;
		}

		needOnkey[key]++;
		needOnKeyCount++;
		isWait = true;
		printf("等待按键:%d \n", key);
	}

	//需要松开按键信号
	void Editor::NeedOffKeySignal(int key)
	{
		/*	lock_guard<mutex> lock(waitOnKeyLock);

			if (onkey[key] == 0)
				return;

			needOffkey[key]++;
			needOffKeyCount++;
			isWait = true;
			printf("等待松开键:%d \n", key);*/
	}

	//按键信号
	void Editor::OnKeySignal(int key)
	{
		GetCurTimeLateNoteKeys(2);

		lock_guard<mutex> lock(waitOnKeyLock);

		printf("按下按键:%d \n", key);
		onkey[key]++;
		onKeyCount++;

		if (needOnkey[key] > 0) //存在需要对应按下的按键
		{
			onkey[key]--;
			onKeyCount--;

			//
			needOnkey[key]--;
			needOnKeyCount--;
			if (needOnKeyCount == 0 && needOffKeyCount == 0)
				isWait = false;
		}
		else if (!lateHavKeys[key])  //当前时间之后没有这个按键事件
		{
			isWait = true;
			printf("按错键,等待中\n");
		}

	}

	//松开按键信号
	void Editor::OffKeySignal(int key)
	{
		lock_guard<mutex> lock(waitOnKeyLock);

		if (onkey[key] > 0)
		{
			printf("松开按键:%d \n", key);
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