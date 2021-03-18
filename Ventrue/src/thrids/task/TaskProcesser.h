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



		//设置主线程Looper
		inline void SetMainThreadTaskProcesser(TaskProcesser* taskProcesser) {
			mainThreadTaskProcesser = taskProcesser;
		}

		// 设置是否送到主线程处理
		inline void SetPostTaskToMainThreadProcess(bool isPostToMainThread) {
			isPostTaskToMainThreadProcess = isPostToMainThread;
		}

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

		int PostTask(TaskCallBack taskCallBack);
		int PostTask(TaskCallBack taskCallBack, int delay);
		int PostTask(TaskCallBack taskCallBack, void* data);
		int PostTask(TaskCallBack taskCallBack, void* data, int delay);
		int PostTask(Task* task);
		int PostTask(Task* task, int delay);



	private:
		int PostTaskDirect(Task* task, int delay, bool isFromSelfThread);
		int PostCommonTask(Task* task, int delay, bool isFromSelfThread);
		int PostTimerTask(Task* task, int delay, bool isFromSelfThread);
		void Run();

		//遍历定时任务列表
		int ReadTimerList(TaskList& timerReads, clock::time_point curTime);
		int ReadCommonList(TaskList& reads);
		int ProcessTask(Task* task);

		//
		static void ThreadRunCallBack(void* param);
	private:
		class TimerCompare
		{
		public:
			TimerCompare() :curTime(res::zero()) {}
			bool operator()(Task* left, Task* right)
			{
				int tmA = (int)chrono::duration_cast<res>(curTime - left->startTime).count();
				int tmB = (int)chrono::duration_cast<res>(curTime - right->startTime).count();
				int delayA = (int)(left->delay - tmA);
				int delayB = (int)(right->delay - tmB);
				return delayA < delayB;
			}

		private:
			clock::time_point curTime;
			friend class TaskProcesser;
		};

	private:

		//主线程任务处理器
		TaskProcesser* mainThreadTaskProcesser = nullptr;

		//是否为固定帧率模式
		bool isFixedFps = false;

		//是否投递任务到主线程进行处理
		bool isPostTaskToMainThreadProcess = false;

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

		TaskList* readList = nullptr;
		TaskList* writeList = nullptr;
		TaskList* timerReadList = nullptr;
		TaskList* timerWriteList = nullptr;

		TaskList* readListSelfThread = nullptr;
		TaskList* writeListSelfThread = nullptr;
		TaskList* timerReadListSelfThread = nullptr;
		TaskList* timerWriteListSelfThread = nullptr;
		long minDelay = MAX_DELAY_TIME;

		thread::id threadID;
		TaskQueue* taskQue = nullptr;
		TimerCompare* timerCompare = nullptr;
		Semaphore pauseSem;
		Semaphore quitSem;

		clock::time_point startTime;
		clock::time_point endTime;
	};

}

#endif
