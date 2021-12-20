#ifndef _TaskProcesser_h_
#define _TaskProcesser_h_

#include"Task.h"
#include"TaskQueue.h"
#include <chrono>

namespace task
{
	class TaskProcesser
	{
	public:
		TaskProcesser();
		~TaskProcesser();

		void SetTaskQueueType(TaskQueueType type);

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

		void PostBlockFilterSingle(int filterNumber);
		void PostUnBlockFilter();
		int PostTask(TaskCallBack taskCallBack);
		int PostTask(TaskCallBack taskCallBack, int delay);
		int PostTask(TaskCallBack taskCallBack, void* data);
		int PostTask(TaskCallBack taskCallBack, void* data, int delay);
		int PostTaskByFilter(TaskCallBack taskCallBack, int filterNumber);
		int PostTaskByFilter(TaskCallBack taskCallBack, int delay, int filterNumber);
		int PostTaskByFilter(TaskCallBack taskCallBack, void* data, int filterNumber);

		int PostTask(TaskCallBack taskCallBack, void* data, int delay, int filterNumber);
		int PostTask(Task* task);
		int PostTask(Task* task, int delay);



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

		int taskProcessRet = 0;
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


		int64_t startTime = 0;

		thread::id threadID;
		Semaphore pauseSem;
		Semaphore quitSem;


		mutex locker;
		Semaphore sem;
	};

}

#endif
