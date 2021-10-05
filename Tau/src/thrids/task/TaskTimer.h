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

	class TimerTaskPool
	{
		SINGLETON(TimerTaskPool)

	public:
		TimerTask* Pop()
		{
			return pool.Pop();
		}

		void Push(TimerTask* task)
		{
			pool.Push(task);
		}

	private:
		ObjectPool<TimerTask> pool;
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
		TimerTaskPool* timerTaskPool = nullptr;
		int durationMS;
		TaskProcesser* taskProcesser = nullptr;
		void* data = nullptr;
		TimerCallBack timerCB = nullptr;

		bool isStop;
		bool isRepeat;

		friend class TaskPump;
	};
}

#endif
