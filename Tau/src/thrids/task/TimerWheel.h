
#ifndef _TimerWheel_h_
#define _TimerWheel_h_

#include"TaskQueue.h"
#include"Task.h"
#include"WheelSlot.h"

namespace task
{

	/**时间轮*/
	class TimerWheel : public TaskQueue
	{
	public:
		TimerWheel(TaskProcesser* processer = nullptr);
		~TimerWheel();

		void Clear();

		/**设置阻塞过滤参数
		* @param filterNumbers 需要过滤的对应号码表
		* @param count 过滤数组长度
		* */
		void SetBlockFilter(int* filterNumbers, int count)
		{
			blockFilterNumbers = filterNumbers;
			blockFilterCount = count;
		}

		bool IsBlockFilter(int num)
		{
			for (int i = 0; i < blockFilterCount; i++) {
				if (blockFilterNumbers[i] == num)
					return true;
			}
			return false;
		}

		/**获取槽的剩余轮的时间范围*/
		int GetTimeRangeSlotIdx(int nWheel)
		{
			return slotRichTimeRange[nWheel];
		}

		/**步进到指定时间点,并回调处理在时间点之间的所有元素*/
		bool Read(int64_t curTimeMS);

		/**合并时间轮数据*/
		void Merge(TaskQueue& mergeTaskQue);

		/**遍历所有元素，并回调处理*/
		void Traversal();

		/**释放所有元素*/
		void Release();

		void ResetCurtTime(int64_t curtTimeMS);

		int64_t GetWaitTimeMS(int64_t curTimeMS);

		/**是否为空数据*/
		bool IsEmpty();

		bool NeedProcessTimeOutList();

		void Add(Task* task);

		bool Remove(Task* task);

	protected:

		void GetSlotPos(int* pos, int64_t timeMS)
		{
			pos[0] = GetFirstWheelSlotIdx(timeMS);
			pos[1] = GetWheelSlotIdx(timeMS, 1);
			pos[2] = GetWheelSlotIdx(timeMS, 2);
			pos[3] = GetWheelSlotIdx(timeMS, 3);
			pos[4] = GetWheelSlotIdx(timeMS, 4);
		}

		void PointerTo(int64_t timeMS)
		{
			curtTimeMS = timeMS;
			GetSlotPos(pointer, timeMS);
		}

		bool IsWheelSlotsEmpty();

		/**获取最近等待执行时间*/
		int64_t GetNearestExecuteTimeMS();

		void AddToWheelSlot(LinkedListNode<Task*>* node, int nWheel, int slotIdx);

		inline int GetFirstWheelSlotIdx(int64_t timeMS) {
			return (int)(timeMS & TVR_MASK);
		}

		inline int GetWheelSlotIdx(int64_t timeMS, int nWheel) {
			return (int)((timeMS >> (TVR_BITS + (nWheel - 1) * TVN_BITS)) & TVN_MASK);
		}

		/**获取槽的最小时间*/
		int GetTimeSlotIdx(int nWheel, int nSlot) {
			return (nWheel == 0 ? nSlot : nSlot << ((nWheel - 1) * 6 + 8));
		}

		WheelSlot* GetWheelSlot(int idx) { return wheelSlots[idx]; }

	protected:
		// 第1个轮占的位数
		const int TVR_BITS = 8;
		// 第1个轮的长度
		const int TVR_SIZE = 1 << TVR_BITS;

		// 第n个轮的长度
		const int TVN_BITS = 6;
		// 第n个轮的长度
		const int TVN_SIZE = 1 << TVN_BITS;

		// 掩码：取模或整除用
		const int TVR_MASK = TVR_SIZE - 1;
		const int TVN_MASK = TVN_SIZE - 1;


		int slotRichTimeRange[5] = { 0, 0xff, 0x3fff, 0xfffff, 0x3ffffff };

		WheelSlot* wheelSlots[5];

		LinkedList<Task*> timeOutList;

		/**时间轮指针位置
		 * idx:时间轮idx
		 * */
		int pointer[5] = { 0 };
		int64_t curtTimeMS = -1;
		int* blockFilterNumbers;
		int blockFilterCount = 0;

	};
}
#endif