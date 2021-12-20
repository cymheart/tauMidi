#ifndef _TaskObjectPool_h_
#define _TaskObjectPool_h_

#include "TaskTypes.h"
#include"Task.h"
#include"TaskTimer.h"

namespace task
{
	class TaskObjectPool
	{
		SINGLETON(TaskObjectPool)

	public:
		ObjectPool<LinkedListNode<Task*>>& NodePool() { return nodePool; }
		ObjectPool<Task>& TaskPool() { return taskPool; }
		ObjectPool<TimerTask>& TimerPool() { return timerPool; }

	private:
		ObjectPool<LinkedListNode<Task*>> nodePool;
		ObjectPool<Task> taskPool;
		ObjectPool<TimerTask> timerPool;
	};

}
#endif
