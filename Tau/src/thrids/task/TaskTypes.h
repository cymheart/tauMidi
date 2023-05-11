#ifndef _TaskTypes_h_
#define _TaskTypes_h_

#include"scutils/Utils.h"
#include"scutils/Semaphore.h"
#include"scutils/SingletonDefine.h"
#include"scutils/ObjectPool.h"
#include"scutils/LinkedList.h"
#include"scutils/LinkedListNode.h"
#include"scutils/ObjectMemory.h"
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

	enum TaskMsg :int
	{
		TMSG_DATA = 0, //数据处理
		TMSG_QUIT, //退出
		TMSG_PAUSE, //暂停
		TMSG_TIMER_START, //定时器启动
		TMSG_TIMER_RUN,  //定时器运行中
		TMSG_TIMER_STOP,  //定时器停止
		TMSG_TIMER_REMOVE, //定时器移除
		TMSG_TASK_REMOVE,	 //任务移除
		TMSG_TASK_BLOCK_FILTER_SINGLE, //任务阻塞过滤单个
		TMSG_TASK_BLOCK_FILTER,  //任务阻塞过滤
		TMSG_TASK_NOT_BLOCK_FILTER, //任务不阻塞过滤
		TMSG_CUSTOM,  //自定义
	};



}

#endif
