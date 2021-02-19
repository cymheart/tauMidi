#include"TaskQueue.h"
#include"Task.h"

namespace task
{
	TaskQueue::TaskQueue()
	{
		readList = new TaskList();
		writeList = new TaskList();
		timerReadList = new TaskList();
		timerWriteList = new TaskList();

		readListSelfThread = new TaskList();
		writeListSelfThread = new TaskList();
		timerReadListSelfThread = new TaskList();
		timerWriteListSelfThread = new TaskList();
	}

	TaskQueue::~TaskQueue()
	{
		Clear();

		DEL(readList);
		DEL(writeList);
		DEL(timerReadList);
		DEL(timerWriteList);
	}

	void TaskQueue::Clear()
	{
		ReleaseList(readList);
		ReleaseList(writeList);
		ReleaseList(timerReadList);
		ReleaseList(timerWriteList);


		ReleaseList(readListSelfThread);
		ReleaseList(writeListSelfThread);
		ReleaseList(timerReadListSelfThread);
		ReleaseList(timerWriteListSelfThread);
	}

	void TaskQueue::ReleaseList(TaskList* evList)
	{
		TaskList::iterator iter = evList->begin();
		for (; iter != evList->end();)
		{
			if ((*iter)->isSysReleaseTask) {
				Task::Release(*iter);
			}

			iter = evList->erase(iter);
		}
	}

	//增加数据任务到自身线程数据任务列表
	void TaskQueue::AddToWriteListSelfThread(Task* task)
	{
		TaskList::iterator iter = writeListSelfThread->begin();
		for (; iter != writeListSelfThread->end(); iter++)
		{
			if (task->GetPriority() < (*iter)->GetPriority())
				break;
		}

		if (iter != writeListSelfThread->end())
			writeListSelfThread->insert(iter, task);
		else
			writeListSelfThread->push_back(task);
	}

	//增加定时任务到自身线程定时任务列表
	void TaskQueue::AddToTimerWriteListSelfThread(Task* task)
	{
		timerWriteListSelfThread->push_back(task);
	}


	//增加其他线程过来的数据任务到数据任务列表
	void TaskQueue::AddToWriteList(Task* task)
	{
		TaskList::iterator iter = writeList->begin();
		for (; iter != writeList->end(); iter++)
		{
			if (task->GetPriority() < (*iter)->GetPriority())
				break;
		}

		if (iter != writeList->end())
			writeList->insert(iter, task);
		else
			writeList->push_back(task);
	}

	//增加其它线程过来的定时任务到定时任务列表*/
	void TaskQueue::AddToTimerWriteList(Task* task)
	{
		timerWriteList->push_back(task);
	}


	void TaskQueue::Lock()
	{
		locker.lock();
	}

	void TaskQueue::UnLock()
	{
		locker.unlock();
	}

	void TaskQueue::Wait()
	{
		sem.wait();
	}

	void TaskQueue::Wait(uint32_t timeOut)
	{
		sem.wait_for(timeOut);
	}

	void TaskQueue::Notify()
	{
		sem.set();
	}
}
