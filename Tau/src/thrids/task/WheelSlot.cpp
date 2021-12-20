#include"WheelSlot.h"
#include"TaskObjectPool.h"

namespace task
{
	WheelSlot::WheelSlot(int size)
	{
		for (int i = 0; i < size; i++)
			slots.push_back(new LinkedList<Task*>());
	}

	WheelSlot::~WheelSlot()
	{
		Clear();

		for (int i = 0; i < slots.size(); i++)
			DEL(slots[i]);
		slots.clear();
	}


	/**清空数据*/
	void WheelSlot::Clear()
	{
		for (int i = 0; i < slots.size(); i++)
			ClearSlot(i);
		elemCount = 0;
	}

	/**判断指定槽是否为空，即指定槽列表数据是否为空*/
	bool WheelSlot::IsSlotEmpty(int slotIdx)
	{
		return slots[slotIdx]->Empty();
	}

	/**清空指定槽数据*/
	void WheelSlot::ClearSlot(int slotIdx)
	{
		LinkedListNode<Task*>* node = slots[slotIdx]->GetHeadNode();
		LinkedListNode<Task*>* next = nullptr;
		for (; node != nullptr; node = next)
		{
			next = node->next;
			TaskObjectPool::GetInstance().NodePool().Push(node);
		}

		elemCount -= slots[slotIdx]->Size();
		slots[slotIdx]->Clear();
	}


	/**获取指定槽数据列表*/
	LinkedList<Task*>* WheelSlot::GetSlotList(int slotIdx)
	{
		if (slotIdx >= slots.size() || slotIdx < 0)
			return nullptr;

		return slots[slotIdx];
	}

	/**增加元素到指定槽的列表中*/
	void WheelSlot::AddToSlot(Task* task, int slotIdx)
	{
		LinkedListNode<Task*>* node = TaskObjectPool::GetInstance().NodePool().Pop();
		node->elem = task;
		AddToSlot(node, slotIdx);
	}

	/**增加元素node到指定槽的列表中*/
	void WheelSlot::AddToSlot(LinkedListNode<Task*>* node, int slotIdx)
	{
		AddToList(node, *slots[slotIdx]);
		elemCount++;
	}

	/**增加元素node到指定数据列表中*/
	void WheelSlot::AddToList(LinkedListNode<Task*>* node, LinkedList<Task*>& taskList)
	{
		if (node->elem->GetPriority() == TASK_MAX_PRIORITY) {
			taskList.AddLast(node);
		}
		else if (node->elem->GetPriority() == TASK_MIN_PRIORITY) {
			taskList.AddFirst(node);
		}
		else
		{
			LinkedListNode<Task*>* tmpNode = taskList.GetHeadNode();
			for (; tmpNode != nullptr; tmpNode = tmpNode->next)
			{
				if (node->elem->GetPriority() < tmpNode->elem->GetPriority())
					break;
			}

			if (tmpNode == nullptr)
				taskList.AddLast(node);
			else
				taskList.AddBefore(tmpNode, node);
		}
	}

	/**获取最近等待执行时间的元素*/
	Task* WheelSlot::GetNearestExecuteTask()
	{
		if (IsEmpty())
			return nullptr;

		Task* nearTask = nullptr;
		for (int i = 0; i < slots.size(); i++)
		{
			if (slots[i]->Empty())
				continue;

			LinkedListNode<Task*>* node = slots[i]->GetHeadNode();
			nearTask = node->elem;
			for (node = node->next; node != nullptr; node = node->next)
			{
				if (node->elem->executeTimeMS < nearTask->executeTimeMS)
					nearTask = node->elem;
			}
			break;
		}

		return nearTask;
	}

}