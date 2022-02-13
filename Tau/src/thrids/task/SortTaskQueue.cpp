#include"SortTaskQueue.h"
#include"Task.h"
#include"TaskObjectPool.h"

namespace task
{
	SortTaskQueue::SortTaskQueue(TaskProcesser* processer)
		:TaskQueue(processer)
	{

	}

	SortTaskQueue::~SortTaskQueue()
	{
		Release();
	}

	int64_t SortTaskQueue::GetWaitTimeMS(int64_t curTimeMS)
	{
		return waitTimeMS;
	}

	void SortTaskQueue::SetBlockFilter(int* filterNumbers, int count)
	{
		blockFilterNumbers = filterNumbers;
		blockFilterCount = count;
	}

	bool SortTaskQueue::IsBlockFilter(int num)
	{
		for (int i = 0; i < blockFilterCount; i++) {
			if (blockFilterNumbers[i] == num)
				return true;
		}
		return false;
	}


	bool SortTaskQueue::IsEmpty()
	{
		return timeOutList.Empty() && timerTaskList.Empty();
	}

	bool SortTaskQueue::NeedProcessTimeOutList()
	{
		if (timeOutList.Empty())
			return false;
		else if (blockFilterCount == 0)
			return true;

		LinkedListNode<Task*>* node = timeOutList.GetHeadNode();
		if (IsBlockFilter(node->elem->filterNum))
			return false;
		return true;
	}


	bool SortTaskQueue::Read(int64_t curTimeMS)
	{
		waitTimeMS = -1;
		LinkedListNode<Task*>* node = timerTaskList.GetHeadNode();
		LinkedListNode<Task*>* next;
		for (; node != nullptr; ) {
			Task* task = node->elem;
			if (curTimeMS < task->executeTimeMS) {
				waitTimeMS = task->executeTimeMS - curTimeMS;
				break;
			}

			next = timerTaskList.Remove(node);
			AddToTimeOutList(node);
			node = next;
		}

		//
		int ret;
		while (!timeOutList.Empty()) {

			node = timeOutList.GetHeadNode();
			for (; node != nullptr; ) {

				if (blockFilterCount > 0 && IsBlockFilter(node->elem->filterNum)) {
					node = node->next;
					continue;
				}

				ret = ReadTaskCallback(taskProcesser, node->elem);
				next = timeOutList.Remove(node);
				TaskObjectPool::GetInstance().NodePool().Push(node);
				node = next;

				if (ret == 1)
					return false;
			}

			if (blockFilterCount == 0)
				continue;

			break;
		}

		return true;
	}


	void SortTaskQueue::Merge(TaskQueue& mergeTaskQue)
	{
		SortTaskQueue& tq = (SortTaskQueue&)mergeTaskQue;
		timerTaskList.Merge(tq.timerTaskList);
		LinkedList<Task*>::Sort(&timerTaskList, timerCompare);
		timeOutList.Merge(tq.timeOutList);
	}

	void SortTaskQueue::Traversal()
	{
		LinkedListNode<Task*>* node = timeOutList.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			if (TraversalCallback(taskProcesser, node->elem) == 1)
				return;
		}

		node = timerTaskList.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			if (TraversalCallback(taskProcesser, node->elem) == 1)
				return;
		}
	}

	void SortTaskQueue::Release()
	{
		LinkedListNode<Task*>* node = timeOutList.GetHeadNode();
		LinkedListNode<Task*>* next = nullptr;
		for (; node != nullptr; node = next) {
			ReleaseCallback(taskProcesser, node->elem);
			next = node->next;
			TaskObjectPool::GetInstance().NodePool().Push(node);
		}
		timeOutList.Clear();


		node = timerTaskList.GetHeadNode();
		for (; node != nullptr; node = next) {
			ReleaseCallback(taskProcesser, node->elem);
			next = node->next;
			TaskObjectPool::GetInstance().NodePool().Push(node);
		}
		timeOutList.Clear();
	}

	void SortTaskQueue::ResetCurtTime(int64_t curtTimeMS)
	{
		Task* task;
		LinkedListNode<Task*>* node = timerTaskList.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			task = node->elem;
			task->executeTimeMS = curtTimeMS + task->delay;
		}
	}


	void SortTaskQueue::Add(Task* task)
	{
		LinkedListNode<Task*>* node = TaskObjectPool::GetInstance().NodePool().Pop();
		node->elem = task;
		if (task->delay > 0)
			timerTaskList.AddLast(node);
		else
			AddToTimeOutList(node);
	}

	bool SortTaskQueue::Remove(Task* task)
	{
		LinkedListNode<Task*>* node = timerTaskList.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			if (node->elem == task)
			{
				timerTaskList.Remove(node);
				TaskObjectPool::GetInstance().NodePool().Push(node);
				return true;
			}
		}

		node = timeOutList.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			if (node->elem == task)
			{
				timeOutList.Remove(node);
				TaskObjectPool::GetInstance().NodePool().Push(node);
				return true;
			}
		}

		return false;
	}

	void SortTaskQueue::AddToTimeOutList(LinkedListNode<Task*>* node)
	{
		if (node->elem->GetPriority() == TASK_MAX_PRIORITY) {
			timeOutList.AddLast(node);
		}
		else if (node->elem->GetPriority() == TASK_MIN_PRIORITY) {
			timeOutList.AddFirst(node);
		}
		else
		{
			LinkedListNode<Task*>* tmpNode = timeOutList.GetHeadNode();
			for (; tmpNode != nullptr; tmpNode = tmpNode->next)
			{
				if (node->elem->GetPriority() < tmpNode->elem->GetPriority())
					break;
			}

			if (tmpNode == nullptr)
				timeOutList.AddLast(node);
			else
				timeOutList.AddBefore(tmpNode, node);
		}
	}

}
