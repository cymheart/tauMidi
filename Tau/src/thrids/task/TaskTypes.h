#ifndef _TaskTypes_h_
#define _TaskTypes_h_

#include"scutils/Utils.h"
#include"scutils/Semaphore.h"
#include"scutils/SingletonDefine.h"
#include"scutils/ObjectPool.h"
#include"scutils/LinkedList.h"
#include"scutils/LinkedListNode.h"
#include<chrono>

using namespace scutils;

namespace task
{

	class TaskTimer;
	class Task;
	class TaskProcesser;
	using TaskCallBack = void (*)(Task* task);
	using MsgCallBack = void (*)(int32_t msg, void* data);
	using TaskCompareCallBack = bool (*)(Task* curtTask, Task* cmpTask);
	using TimerCallBack = void (*)(void* data);
	using ProcessTaskCallBack = int (*)(TaskProcesser* taskProcesser, Task* task);


#define TASK_MAX_PRIORITY 100000
#define TASK_MIN_PRIORITY -1

	enum TaskQueueType
	{
		SortTaskQue,
		TimerWheelQue
	};

	enum TaskMsg
	{
		TMSG_DATA,
		TMSG_QUIT,
		TMSG_TIMER_START,
		TMSG_TIMER_RESTART,
		TMSG_TIMER_RUN,
		TMSG_TIMER_STOP,
		TMSG_TASK_REMOVE,
		TMSG_PAUSE,
		/**任务阻塞过滤单个*/
		TMSG_TASK_BLOCK_FILTER_SINGLE,
		/**任务阻塞过滤*/
		TMSG_TASK_BLOCK_FILTER,
		/**任务不阻塞过滤*/
		TMSG_TASK_NOT_BLOCK_FILTER,
	};



}

#endif
