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


	//进入到步进播放模式
	void Editor::EnterStepPlayMode()
	{
		if (playMode == EditorPlayMode::Wait) {
			printf("当前模式为:等待播放模式，需要离开等待播放模式! \n");
			return;
		}

		playMode = EditorPlayMode::Step;
		isWait = false;

		//
		waitSem.reset(tau->midiEditorSyntherCount - 1);
		for (int i = 0; i < tau->midiEditorSyntherCount; i++)
			tau->midiEditorSynthers[i]->EnterPlayModeTask(EditorPlayMode::Step, &waitSem);
		waitSem.wait();
	}


	//进入到等待播放模式
	void Editor::EnterWaitPlayMode()
	{
		if (playMode == EditorPlayMode::Step) {
			printf("当前模式为:步进播放模式，需要离开步进播放模式! \n");
			return;
		}

		playMode = EditorPlayMode::Wait;
		isWait = false;
		memset(onkey, 0, sizeof(int) * 128);
		memset(needOnkey, 0, sizeof(int) * 128);
		memset(needOffkey, 0, sizeof(int) * 128);
		needOnKeyCount = 0;
		needOffKeyCount = 0;

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
			memset(onkey, 0, sizeof(int) * 128);
			memset(needOnkey, 0, sizeof(int) * 128);
			memset(needOffkey, 0, sizeof(int) * 128);
			needOnKeyCount = 0;
			needOffKeyCount = 0;
			isWait = false;
		}

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
		waitOnKeyLock.lock();

		needOnkey[key]++;

		if (needOnkey[key] > 0)
			needOnKeyCount++;

		if (needOnKeyCount > 0)
		{
			isWait = true;
			printf("等待按键:%d \n", key);
		}

		waitOnKeyLock.unlock();
	}

	//需要松开按键信号
	void Editor::NeedOffKeySignal(int key)
	{
		waitOnKeyLock.lock();

		if (onkey[key] == 0) {
			waitOnKeyLock.unlock();
			return;
		}

		needOffkey[key]++;

		if (needOffkey[key] > 0)
			needOffKeyCount++;

		if (needOffKeyCount > 0) {
			isWait = true;
			printf("等待松开键:%d \n", key);
		}


		waitOnKeyLock.unlock();
	}

	//按键信号
	void Editor::OnKeySignal(int key)
	{
		waitOnKeyLock.lock();

		needOnkey[key]--;
		onkey[key]++;

		if (needOnkey[key] >= 0)
			needOnKeyCount--;

		if (needOnKeyCount <= 0 && needOffKeyCount <= 0) {
			isWait = false;
			printf("按下按键,继续:%d \n", key);
		}
		else
		{
			printf("按下按键:%d \n", key);
		}

		waitOnKeyLock.unlock();
	}

	//松开按键信号
	void Editor::OffKeySignal(int key)
	{
		waitOnKeyLock.lock();

		if (onkey[key] > 0 && needOffkey[key] == 0 && needOnkey[key] == 0)
		{
			needOnkey[key]++;
			needOnKeyCount++;
			onkey[key]--;

			if (needOnKeyCount > 0) {
				isWait = true;
				printf("等待按键:%d \n", key);
			}

			waitOnKeyLock.unlock();
			return;
		}


		if (needOffkey[key] == 0)
		{
			if (onkey[key] > 0)
				onkey[key]--;

			waitOnKeyLock.unlock();
			return;
		}

		needOffkey[key]--;

		if (onkey[key] > 0)
			onkey[key]--;

		if (needOffkey[key] >= 0)
			needOffKeyCount--;

		if (needOnKeyCount <= 0 && needOffKeyCount <= 0) {
			isWait = false;
			printf("松开按键，继续:%d \n", key);
		}
		else
		{
			printf("松开按键:%d \n", key);
		}

		waitOnKeyLock.unlock();
	}

}