
#ifndef _WheelSlot_h_
#define _WheelSlot_h_

#include"TaskProcesser.h"

namespace task
{

	/**时间轮槽*/
	class WheelSlot
	{

	public:
		WheelSlot(int size);
		~WheelSlot();

		inline bool IsEmpty()
		{
			return elemCount == 0;
		}

		inline void AddElemCount(int count)
		{
			elemCount += count;
		}

		/**获取槽数量*/
		inline int GetSlotCount()
		{
			return slots.size();
		}

		/**清空数据*/
		void Clear();

		/**判断指定槽是否为空，即指定槽列表数据是否为空*/
		bool IsSlotEmpty(int slotIdx);

		/**清空指定槽数据*/
		void ClearSlot(int slotIdx);

		/**获取指定槽数据列表*/
		LinkedList<Task*>* GetSlotList(int slotIdx);

		/**增加元素到指定槽的列表中*/
		void AddToSlot(Task* task, int slotIdx);

		/**增加元素node到指定槽的列表中*/
		void AddToSlot(LinkedListNode<Task*>* node, int slotIdx);

		/**获取最近等待执行时间的元素*/
		Task* GetNearestExecuteTask();

	protected:

		/**增加元素node到指定数据列表中*/
		void AddToList(LinkedListNode<Task*>* node, LinkedList<Task*>& taskList);


	protected:
		int elemCount = 0;
		vector<LinkedList<Task*>*> slots;
	};
}

#endif