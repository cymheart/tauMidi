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

		TaskTimer() {}
		TaskTimer(TaskProcesser* taskProcesser, TimerCallBack timerCB,
			void* data, int durationMS, bool isRepeat = true);

		~TaskTimer();


		void SetDuration(int ms)
		{
			durationMS = ms;
		}

		void Start();
		void Stop();
		void ReStart();
		void Remove();


	private:


		void Init(TaskProcesser* taskProcesser,
			TimerCallBack timerCB,
			void* data, bool isRepeat);

		void Init(TaskProcesser* taskProcesser,
			TimerCallBack timerCB,
			void* data, int durationMS, bool isRepeat);

		void Clear();

		void PostTask(int tm);

		static void RunTask(Task* task);
		static void StartTask(Task* task);
		static void StopTask(Task* task);

	private:
		TimerTask* runTask = nullptr;
		int durationMS = 0;
		TaskProcesser* taskProcesser = nullptr;
		void* data = nullptr;
		TimerCallBack timerCB = nullptr;

		bool isStop = true;
		bool isRepeat = true;
		TaskMsg state = TaskMsg::TMSG_TIMER_STOP;

		friend class TaskProcesser;
	};
}

#endif
