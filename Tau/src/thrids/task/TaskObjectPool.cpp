#include"TaskObjectPool.h"

namespace task
{
	BUILD_SHARE(TaskObjectPool)

		
	TaskObjectPool::TaskObjectPool()
	{
		nodePool.CreatePool(20);
		timerTaskPool.CreatePool(20);
		timerPool.CreatePool(20);

		//
		taskPool.NewMethod = NewTask;
		taskPool.CreatePool(20);
		

	}

	TaskObjectPool::~TaskObjectPool()
	{

	}

	Task* TaskObjectPool::NewTask()
	{
		Task* task = new Task();
		task->SetSysReleaseTask(false);
		task->releaseCallBack = TaskRelease;
		return task;
	}


	void TaskObjectPool::TaskRelease(Task* task)
	{
		TaskObjectPool::GetInstance().TaskPool().Push(task);
	}

}
