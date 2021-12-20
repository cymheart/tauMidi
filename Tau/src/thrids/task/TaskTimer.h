#ifndef _TaskTimer_h_
#define _TaskTimer_h_

#include"TaskTypes.h"
#include"TaskProcesser.h"


namespace task
{
	class TimerTask :public Task
	{
	public:
		TimerTask()
		{
			isSysReleaseTask = false;
			releaseCallBack = Release;
		}

	public:
		TaskTimer* timer = nullptr;
		void* data = nullptr;

	private:
		static void Release(Task* task);
	};


	class TaskTimer
	{
	public:
		TaskTimer(TaskProcesser* taskProcesser, TimerCallBack timerCB,
			void* data, int durationMS, bool isRepeat = true);

		~TaskTimer();

		void SetDuration(int ms)
		{
			durationMS = ms;
		}

		void Start();
		void ReStart();
		void Stop();

	private:
		void PostTask(int tm);

		static void RunTask(Task* task);
		static void StartTask(Task* task);
		static void StopTask(Task* task);

	public:
		TimerTask* task;

	private:
		int durationMS;
		TaskProcesser* taskProcesser = nullptr;
		void* data = nullptr;
		TimerCallBack timerCB = nullptr;

		bool isStop;
		bool isRepeat;
	};
}

#endif
