#ifndef _TaskTimer_h_
#define _TaskTimer_h_

#include"TaskTypes.h"
#include"TaskProcesser.h"

namespace task
{
	using TimerCallBack = void (*)(void* data);

	class TimerTask :public Task
	{
	public:
		TaskTimer* timer = nullptr;
		void* data = nullptr;
	};

	class TaskTimer
	{
	public:
		TaskTimer(TaskProcesser* taskProcesser, TimerCallBack timerCB,
			void* data, int durationMS, bool isRepeat = true);

		~TaskTimer();

		void Start();
		void Stop();

	private:
		void PostTask(int tm);

		static void RunTask(Task* task);
		static void StartTask(Task* task);

	public:
		TimerTask* task;
	private:

		int durationMS;
		TaskProcesser* taskProcesser;
		void* data;
		TimerCallBack timerCB;

		bool isStop;
		bool isRepeat;

		friend class TaskPump;
	};
}

#endif
