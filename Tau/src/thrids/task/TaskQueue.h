#ifndef _TaskQueue_h_
#define _TaskQueue_h_

#include"TaskTypes.h"

namespace task
{
	class TaskQueue
	{
	public:
		TaskQueue(TaskProcesser* processer = nullptr)
			:taskProcesser(processer)
		{

		}

		void SetReadTaskCallback(ProcessTaskCallBack cb)
		{
			ReadTaskCallback = cb;
		}

		void SetReleaseCallback(ProcessTaskCallBack cb)
		{
			ReleaseCallback = cb;
		}

		void SetTraversalCallback(ProcessTaskCallBack cb)
		{
			TraversalCallback = cb;
		}


		virtual int64_t GetWaitTimeMS(int64_t curTimeMS) = 0;

		virtual bool IsEmpty() = 0;
		virtual bool NeedProcessTimeOutList() = 0;

		/**设置阻塞过滤参数
		 * @param filterNumbers 需要过滤的对应号码表
		 * @param count 过滤数组长度
		 * */
		virtual  void SetBlockFilter(int* filterNumbers, int count) = 0;

		virtual void Add(Task* task) = 0;
		virtual bool Remove(Task* task) = 0;

		virtual bool Read(int64_t curTimeMS) = 0;

		virtual void Merge(TaskQueue& mergeTaskQue) = 0;

		/**遍历所有元素，并回调处理*/
		virtual void Traversal() = 0;

		/**释放所有元素*/
		virtual void Release() = 0;

		virtual void ResetCurtTime(int64_t curtTimeMS) = 0;

	protected:
		TaskProcesser* taskProcesser = nullptr;
		ProcessTaskCallBack ReadTaskCallback = nullptr;
		ProcessTaskCallBack ReleaseCallback = nullptr;
		ProcessTaskCallBack TraversalCallback = nullptr;

	};

}
#endif
