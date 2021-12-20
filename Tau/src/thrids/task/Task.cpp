#include"Task.h"

namespace task
{
	//Task
	Task::Task()
	{
	}

	Task::Task(TaskMsg taskMsg)
		:msg(taskMsg)
	{

	}

	Task::Task(TaskCallBack processDataCallBack, TaskMsg taskMsg)
		: processCallBack(processDataCallBack)
		, msg(taskMsg)
	{

	}

	Task::Task(TaskCompareCallBack cmpTaskCallBack, TaskMsg taskMsg)
		: cmpCallBack(cmpTaskCallBack)
		, msg(taskMsg)
	{

	}

	void Task::Clear()
	{
		filterNum = 0;
		executeTimeMS = 0;
		for (int i = 0; i < 5; i++)
			slotPos[i] = 0;
		priority = TASK_MAX_PRIORITY;

		delay = 0;
		processCallBack = nullptr;
		releaseCallBack = nullptr;
		cmpCallBack = nullptr;
		msg = TaskMsg::TMSG_DATA;
		data = nullptr;

		for (int i = 0; i < 10; i++)
			exData[i] = nullptr;

		isRemove = false;
	}

	void Task::Release(Task* task)
	{
		if (task->isSysReleaseTask)
		{
			DEL(task);
		}
		else if (task->releaseCallBack != nullptr)
		{
			TaskCallBack cb = task->releaseCallBack;
			task->Clear();
			task->releaseCallBack = cb;
			task->releaseCallBack(task);
		}
	}

}
