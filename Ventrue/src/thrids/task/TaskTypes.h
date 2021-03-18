#ifndef _TaskTypes_h_
#define _TaskTypes_h_

#include"scutils/Utils.h"
#include"scutils/Semaphore.h"
#include"scutils/SingletonDefine.h"
#include"scutils/ObjectPool.h"
#include<chrono>

using namespace scutils;

namespace task
{

	class TaskTimer;
	class Task;
	using TaskCallBack = void (*)(Task* task);
	using TaskList = list<Task*>;

#define TASK_MAX_PRIORITY 99

	enum TaskMsg
	{
		TMSG_DATA,
		TMSG_QUIT,
		TMSG_TIMER_START,
		TMSG_TIMER_RESTART,
		TMSG_TIMER_RUN,
		TMSG_TIMER_STOP,
		TMSG_PAUSE
	};

}

#endif
