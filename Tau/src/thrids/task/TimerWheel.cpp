#include"TimerWheel.h"
#include"TaskObjectPool.h"

namespace task
{
	TimerWheel::TimerWheel(TaskProcesser* processer)
		:TaskQueue(processer)
	{
		wheelSlots[0] = new WheelSlot(TVR_SIZE);

		for (int i = 1; i < 5; i++)
			wheelSlots[i] = new WheelSlot(TVN_SIZE);
	}

	TimerWheel::~TimerWheel()
	{
		Release();

		for (int i = 0; i < 5; i++)
			DEL(wheelSlots[i]);
	}

	/**步进到指定时间点,并回调处理在时间点之间的所有元素*/
	void TimerWheel::Read(int64_t curTimeMS)
	{
		int count;
		LinkedList<Task*>* taskList;
		long tm = curtTimeMS + 1;
		if (curTimeMS - tm > 20)
			tm = GetNearestExecuteTimeMS();

		for (; tm <= curTimeMS; tm++)
		{
			PointerTo(tm);

			for (int i = 4; i >= 1; i--)
			{
				if (wheelSlots[i]->IsSlotEmpty(pointer[i]))
					continue;

				taskList = wheelSlots[i]->GetSlotList(pointer[i]);
				count = taskList->Size();
				LinkedListNode<Task*>* node = taskList->GetHeadNode();
				LinkedListNode<Task*>* next = nullptr;

				for (; node != nullptr; node = next) {
					Task* task = node->elem;
					next = taskList->Remove(node);
					if (task->slotPos[i - 1] < pointer[i - 1])
						timeOutList.AddLast(node);
					else
						AddToWheelSlot(node, i - 1, task->slotPos[i - 1]);
				}

				wheelSlots[i]->AddElemCount(-count);
			}


			//
			taskList = wheelSlots[0]->GetSlotList(pointer[0]);
			count = taskList->Size();
			if (count != 0) {
				timeOutList.Merge(*taskList);
				wheelSlots[0]->AddElemCount(-count);
			}
		}

		//处理超时元素
		while (!timeOutList.Empty())
		{
			LinkedListNode<Task*>* node = timeOutList.GetHeadNode();
			LinkedListNode<Task*>* next;
			int ret = 0;
			for (; node != nullptr; ) {
				if (blockFilterCount > 0 && IsBlockFilter(node->elem->filterNum)) {
					node = node->next;
					continue;
				}

				if (node->elem != nullptr)
					ret = ReadTaskCallback(taskProcesser, node->elem);

				next = timeOutList.Remove(node);
				TaskObjectPool::GetInstance().NodePool().Push(node);
				node = next;

				if (ret == 1)
					return;
			}

			if (blockFilterCount == 0)
				continue;

			break;
		}
	}


	/**合并时间轮数据*/
	void TimerWheel::Merge(TaskQueue& mergeTaskQue)
	{
		TimerWheel& mergeTimerWheel = (TimerWheel&)mergeTaskQue;

		for (int i = 4; i >= 0; i--)
		{
			WheelSlot* mergeWheelSlot = mergeTimerWheel.GetWheelSlot(i);
			if (mergeWheelSlot->IsEmpty())
				continue;

			int count;
			LinkedList<Task*>* mergeSlotList;
			for (int j = 0; j < wheelSlots[i]->GetSlotCount(); j++)
			{
				mergeSlotList = mergeWheelSlot->GetSlotList(j);
				if (mergeSlotList == nullptr)
					break;
				else if (mergeSlotList->Empty())
					continue;

				count = mergeSlotList->Size();

				if (j < pointer[i] || (i == 0 && j == pointer[i]))
				{
					timeOutList.Merge(*mergeSlotList);
				}
				else if (j == pointer[i])
				{
					Task* task;
					LinkedListNode<Task*>* node = mergeSlotList->GetHeadNode();
					LinkedListNode<Task*>* next = nullptr;
					for (; node != nullptr; node = next) {
						task = node->elem;
						next = mergeSlotList->Remove(node);

						if (task->slotPos[i - 1] < pointer[i - 1])
							timeOutList.AddLast(node);
						else
							mergeTimerWheel.AddToWheelSlot(node, i - 1, task->slotPos[i - 1]);
					}
				}
				else
				{
					LinkedList<Task*>* slotList = wheelSlots[i]->GetSlotList(j);
					slotList->Merge(*mergeSlotList);
					wheelSlots[i]->AddElemCount(count);
				}

				mergeWheelSlot->AddElemCount(-count);

				if (mergeWheelSlot->IsEmpty())
					break;
			}

		}
	}

	/**遍历所有元素，并回调处理*/
	void TimerWheel::Traversal()
	{
		LinkedListNode<Task*>* node = timeOutList.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			if (TraversalCallback(taskProcesser, node->elem) == 1)
				return;
		}

		//
		LinkedList<Task*>* taskList;
		for (int i = 0; i < 5; i++)
		{
			if (wheelSlots[i]->IsEmpty())
				continue;

			for (int j = 0; j < wheelSlots[i]->GetSlotCount(); j++)
			{
				taskList = wheelSlots[i]->GetSlotList(j);
				node = taskList->GetHeadNode();
				for (; node != nullptr; node = node->next) {
					if (TraversalCallback(taskProcesser, node->elem) == 1)
						return;
				}
			}
		}
	}

	/**释放所有元素*/
	void TimerWheel::Release()
	{
		LinkedListNode<Task*>* node = timeOutList.GetHeadNode();
		LinkedListNode<Task*>* next = nullptr;
		for (; node != nullptr; node = next) {
			ReleaseCallback(taskProcesser, node->elem);
			next = node->next;
			TaskObjectPool::GetInstance().NodePool().Push(node);
		}
		timeOutList.Clear();

		//
		LinkedList<Task*>* taskList;
		for (int i = 0; i < 5; i++)
		{
			if (wheelSlots[i]->IsEmpty())
				continue;

			for (int j = 0; j < wheelSlots[i]->GetSlotCount(); j++)
			{
				taskList = wheelSlots[i]->GetSlotList(j);
				node = taskList->GetHeadNode();
				for (; node != nullptr; node = node->next) {
					ReleaseCallback(taskProcesser, node->elem);
				}
			}

			wheelSlots[i]->Clear();
		}
	}

	void TimerWheel::ResetCurtTime(int64_t curtTimeMS)
	{
		LinkedListNode<Task*>* node;
		LinkedList<Task*>* taskList;
		LinkedListNode<Task*>* next = nullptr;
		for (int i = 0; i < 5; i++)
		{
			if (wheelSlots[i]->IsEmpty())
				continue;

			int count = 0;
			for (int j = 0; j < wheelSlots[i]->GetSlotCount(); j++)
			{
				taskList = wheelSlots[i]->GetSlotList(j);
				count += taskList->Size();

				node = taskList->GetHeadNode();
				for (; node != nullptr; node = next) {
					Task* task = node->elem;
					task->executeTimeMS = curtTimeMS + task->delay;
					next = taskList->Remove(node);
					TaskObjectPool::GetInstance().NodePool().Push(node);
					Add(task);
				}
			}

			wheelSlots[i]->AddElemCount(-count);
		}

	}

	int64_t TimerWheel::GetWaitTimeMS(int64_t curTimeMS)
	{
		int64_t nearestExTime = GetNearestExecuteTimeMS();
		if (nearestExTime < 0) { return -1; }
		return nearestExTime - curTimeMS;
	}

	/**是否为空数据*/
	bool TimerWheel::IsEmpty()
	{
		return (timeOutList.Empty() && IsWheelSlotsEmpty());
	}

	bool TimerWheel::IsWheelSlotsEmpty()
	{
		return wheelSlots[0]->IsEmpty() &&
			wheelSlots[1]->IsEmpty() &&
			wheelSlots[2]->IsEmpty() &&
			wheelSlots[3]->IsEmpty() &&
			wheelSlots[4]->IsEmpty();
	}

	/**是否需要处理超时列表*/
	bool TimerWheel::NeedProcessTimeOutList()
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

	/**获取最近等待执行时间*/
	int64_t TimerWheel::GetNearestExecuteTimeMS()
	{
		if (IsWheelSlotsEmpty())
			return -1;

		int nWheel = 0;
		for (; nWheel < 5; nWheel++)
		{
			Task* task = wheelSlots[nWheel]->GetNearestExecuteTask();
			if (task != nullptr)
				return task->executeTimeMS;
		}

		return -1;
	}


	void TimerWheel::Add(Task* task)
	{
		if (task->executeTimeMS == 0)
		{
			wheelSlots[0]->AddToSlot(task, 0);
			return;
		}

		int slotIdx = 0;
		int nWheel = 5;
		while (slotIdx == 0) {
			--nWheel;
			if (nWheel == 0) { slotIdx = GetFirstWheelSlotIdx(task->executeTimeMS); }
			else { slotIdx = GetWheelSlotIdx(task->executeTimeMS, nWheel); }
		}

		GetSlotPos(task->slotPos, task->executeTimeMS);
		wheelSlots[nWheel]->AddToSlot(task, slotIdx);
	}

	bool TimerWheel::Remove(Task* task)
	{
		LinkedListNode<Task*>* node;
		LinkedList<Task*>* taskList;
		for (int i = 0; i < 5; i++)
		{
			if (wheelSlots[i]->IsEmpty())
				continue;

			for (int j = 0; j < wheelSlots[i]->GetSlotCount(); j++)
			{
				taskList = wheelSlots[i]->GetSlotList(j);
				node = taskList->GetHeadNode();
				for (; node != nullptr; node = node->next) {
					if (node->elem == task)
					{
						taskList->Remove(node);
						TaskObjectPool::GetInstance().NodePool().Push(node);
						return true;
					}
				}
			}
		}

		//
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


	void TimerWheel::AddToWheelSlot(LinkedListNode<Task*>* node, int nWheel, int slotIdx)
	{
		wheelSlots[nWheel]->AddToSlot(node, slotIdx);
	}

}