#ifndef _TaskQueue_h_
#define _TaskQueue_h_

#include"TaskTypes.h"

namespace task
{
	class TaskQueue
	{
	public:
		TaskQueue();
		~TaskQueue();

		void Clear();
		void ReleaseList(TaskList* evList);

		//增加数据任务到自身线程数据任务列表
		void AddToWriteListSelfThread(Task* task);

		//增加定时任务到自身线程定时任务列表
		void AddToTimerWriteListSelfThread(Task* task);

		//增加其他线程过来的数据任务到数据任务列表
		void AddToWriteList(Task* task);

		//增加其它线程过来的定时任务到定时任务列表*/
		void AddToTimerWriteList(Task* task);

		void Lock();
		void UnLock();
		void Wait();
		void Wait(uint32_t timeOut);
		void Notify();

	public:

		TaskList* readListSelfThread;
		TaskList* writeListSelfThread;
		TaskList* timerReadListSelfThread;
		TaskList* timerWriteListSelfThread;

		TaskList* readList;
		TaskList* writeList;
		TaskList* timerReadList;
		TaskList* timerWriteList;

		mutex locker;
		Semaphore sem;
	};

}
#endif