#ifndef _SortTaskQueue_h_
#define _SortTaskQueue_h_


#include"TaskQueue.h"
#include"Task.h"

namespace task
{
	class SortTaskQueue :public TaskQueue
	{
	public:
		SortTaskQueue(TaskProcesser* processer = nullptr);
		~SortTaskQueue();

		int64_t GetWaitTimeMS(int64_t curTimeMS);
		void SetBlockFilter(int* filterNumbers, int count);
		bool IsBlockFilter(int num);
		bool IsEmpty();
		bool NeedProcessTimeOutList();
		bool Read(int64_t curTimeMS);
		void Merge(TaskQueue& mergeTaskQue);
		void Traversal();
		void Release();
		void ResetCurtTime(int64_t curtTimeMS);
		void Add(Task* task);
		bool Remove(Task* task);

	protected:
		void AddToTimeOutList(LinkedListNode<Task*>* node);
		static bool Compare(Task* left, Task* right);

	protected:
		LinkedList<Task*> timeOutList;
		LinkedList<Task*> timerTaskList;
		int64_t waitTimeMS = -1;
		int* blockFilterNumbers;
		int blockFilterCount = 0;
	};


}
#endif
