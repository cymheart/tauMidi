#ifndef _TaskProcesser_h_
#define _TaskProcesser_h_

#include"Task.h"
#include"TaskQueue.h"
#include <chrono>
#include<thread>


namespace task
{
	class TaskProcesser
	{
	public:
		TaskProcesser();
		~TaskProcesser();

		void SetTaskQueueType(TaskQueueType type);

		std::thread::id& GetThreadID() {
			return threadID;
		}

		Task* CreateTask();

		void PushTask(Task* task);
		Task* PopTask(TaskMsg msg = TaskMsg::TMSG_DATA);

		//打开嵌入模式
		void OpenEmbedMode();

		//打开单线程固定帧率模式
		void OpenSingleThreadFixedFrameRateMode();

		//打开线程固定帧率模式
		void OpenThreadFixedFrameRateMode();


		//设置锁模式
		inline void SetLockMode(bool isLock) {
			this->isLock = isLock;
		}

		//启用协程
		void EnableCoroutine();

		void* GetMainFiberHandle()
		{
			return mainFiberHandle;
		}

		//返回一个当前时间的毫秒值
		//float GetCurtTimeMillis();


		//设置否是嵌入到其它循环回调之中
		inline void SetEmbedMode(bool isEmbed)
		{
			this->isEmbed = isEmbed;
		}

		//设置是否系统释放task
		inline void SetSysReleaseTask(bool _isSysReleaseTask)
		{
			isSysRelaseTask = _isSysReleaseTask;
		}


		//设置是否固定帧率
		inline void SetFixedFrameRate(bool _isFixeFps) {
			isFixedFps = _isFixeFps;
		}

		//设置帧率
		inline void SetFrameRate(float _frameRate) {
			frameRate = _frameRate;
			perFrameDuration = 1000.0f / frameRate;
		}

		//获取帧率
		inline float GetFrameRate() {
			return frameRate;
		}

		int Start(bool isRunNewThread = true);
		int Pause();
		int Continue();
		int Stop();

		//生成固定帧率定时器 
		TaskTimer* CreateTimer(TimerCallBack timerCB, void* data, bool isRepeat);
		//生成定时器 
		TaskTimer* CreateTimer(TimerCallBack timerCB, void* data, int durationMS, bool isRepeat);
		//回收定时器 
		void RecycleTimer(TaskTimer* timer);

		//
		void PostBlockFilterSingle(int filterNumber);
		void PostUnBlockFilter();
		void PostRemoveTask(TaskCompareCallBack cmpCB, void* data, int delay);
		void PostTask(Task* task, int delay = 0);

	private:

		void Lock();
		void UnLock();
		void Wait();
		void Wait(uint32_t timeOut);
		void Notify();

		void Run();

		int ProcessTask(Task* curtTask);

		static int ReadTaskList(TaskProcesser* taskProcesser, Task* task)
		{
			return taskProcesser->_ReadTaskList(task);
		}
		static int RemoveTaskProcess(TaskProcesser* taskProcesser, Task* task)
		{
			return taskProcesser->_RemoveTaskProcess(task);
		}
		static int ReleaseProcess(TaskProcesser* taskProcesser, Task* task)
		{
			return taskProcesser->_ReleaseProcess(task);
		}
		int _ReadTaskList(Task* task);
		int _RemoveTaskProcess(Task* task);
		int _ReleaseProcess(Task* task);

		//
		static void ThreadRunCallBack(void* param);

	private:

		TaskQueueType taskQueType = TaskQueueType::SortTaskQue;
		TaskQueue* readQue;
		TaskQueue* writeQue;

		int taskBlockFilterNumbers[100];
		int taskBlockFilterCount = 0;

		Task* cmpRemoveTask = nullptr;

		//是否为固定帧率模式
		bool isFixedFps = false;

		//帧率
		float frameRate = 60;

		//每帧花费时间(ms)
		float perFrameDuration = 1000.0f / frameRate;

		//是否对数据进行加锁读写
		bool isLock = true;

		//是否是嵌入到其它循环回调之中
		bool isEmbed = false;

		//是否由系统释放Task
		bool isSysRelaseTask = true;

		bool isStop = true;

		//是否启用协程
		bool enableCoroutine = false;
		//协程句柄
		void* mainFiberHandle = nullptr;

		int64_t startTime = 0;

		std::thread::id threadID;
		Semaphore pauseSem;
		Semaphore quitSem;


		mutex locker;
		Semaphore sem;
	};

}

#endif
