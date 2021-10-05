#include"Task.h"

namespace task
{
	//Task
	Task::Task()
		: startTime(res::zero())
		, delay(0)
		, processCallBack(nullptr)
		, msg(TaskMsg::TMSG_DATA)
	{
	}

	Task::Task(TaskMsg taskMsg)
		: startTime(res::zero())
		, delay(0)
		, processCallBack(nullptr)
		, msg(taskMsg)
	{
	}

	Task::Task(TaskCallBack processDataCallBack, TaskMsg msg)
		:startTime(res::zero())
		, delay(0)
		, processCallBack(processDataCallBack)
	{
		this->msg = msg;
	}

	Task::Task(TaskCompareCallBack cmpTaskCallBack, TaskMsg msg)
		:startTime(res::zero())
		, delay(0)
		, cmpCallBack(cmpTaskCallBack)
	{
		this->msg = msg;
	}



	void Task::Release(Task* task)
	{
		task->isRemove = false;

		if (task->isSysReleaseTask)
		{
			DEL(task);
		}
		else if (task->releaseCallBack != nullptr)
		{
			task->releaseCallBack(task);
		}
	}

}
