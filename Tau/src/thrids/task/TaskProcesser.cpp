#include"TaskProcesser.h"
#include"TaskTimer.h"
#include"SortTaskQueue.h"
#include"TimerWheel.h"
#include"TaskObjectPool.h"

namespace task
{
	TaskProcesser::TaskProcesser()
	{
		SetTaskQueueType(taskQueType);
	}

	TaskProcesser::~TaskProcesser()
	{
		Stop();
		DEL(readQue);
		DEL(writeQue);
	}

	void TaskProcesser::SetTaskQueueType(TaskQueueType type)
	{
		taskQueType = type;

		if (taskQueType == SortTaskQue) {
			readQue = new SortTaskQueue(this);
			writeQue = new SortTaskQueue(this);
		}
		else
		{
			readQue = new TimerWheel(this);
			writeQue = new TimerWheel(this);
		}

		readQue->SetReadTaskCallback(ReadTaskList);
		readQue->SetTraversalCallback(RemoveTaskProcess);
		readQue->SetReleaseCallback(ReleaseProcess);

		//
		writeQue->SetReleaseCallback(ReleaseProcess);
		writeQue->SetTraversalCallback(RemoveTaskProcess);
	}

	void TaskProcesser::Lock()
	{
		locker.lock();
	}

	void TaskProcesser::UnLock()
	{
		locker.unlock();
	}

	void TaskProcesser::Wait()
	{
		sem.wait();
	}

	void TaskProcesser::Wait(uint32_t timeOut)
	{
		sem.wait_for(timeOut);
	}

	void TaskProcesser::Notify()
	{
		sem.set();
	}

	void TaskProcesser::PushTask(Task* task)
	{
		task->Clear();
		TaskObjectPool::GetInstance().TaskPool().Push(task);
	}

	Task* TaskProcesser::PopTask(TaskMsg msg)
	{
		Task* task = TaskObjectPool::GetInstance().TaskPool().Pop();
		task->msg = msg;
		return task;
	}

	Task* TaskProcesser::CreateTask()
	{
		return PopTask(TaskMsg::TMSG_DATA);
	}

	//打开嵌入模式
	void TaskProcesser::OpenEmbedMode()
	{
		SetFixedFrameRate(false);
		SetLockMode(false);
		SetEmbedMode(true);
	}

	//打开单线程固定帧率模式
	void TaskProcesser::OpenSingleThreadFixedFrameRateMode()
	{
		SetFixedFrameRate(true);
		SetLockMode(false);
		SetEmbedMode(false);
	}

	//打开线程固定帧率模式
	void TaskProcesser::OpenThreadFixedFrameRateMode()
	{
		SetFixedFrameRate(true);
		SetLockMode(true);
		SetEmbedMode(false);
	}

	////返回一个当前时间的毫秒值
	//float TaskProcesser::GetCurtTimeMillis()
	//{
	//	return chrono::duration_cast<res>(curTime - startTime).count() * 0.001f;
	//}

	//启用协程
	void TaskProcesser::EnableCoroutine() {
		enableCoroutine = true;
	}

	int TaskProcesser::Start(bool isRunNewThread)
	{
		if (!isStop)
			return 0;

		startTime = GetCurrentTimeMsec();

		readQue->Clear();

		isStop = false;

		if (isEmbed) {
			threadID = this_thread::get_id();
			return 0;
		}

		if (isRunNewThread)
		{
			thread t(ThreadRunCallBack, this);
			threadID = t.get_id();
			t.detach();//将线程分离
		}
		else
		{
			threadID = this_thread::get_id();
		}

		writeQue->ResetCurtTime(0);

		return 0;
	}

	void TaskProcesser::ThreadRunCallBack(void* param)
	{
		TaskProcesser* taskProcesser = (TaskProcesser*)param;
		taskProcesser->Run();
	}

	//生成固定帧率定时器
	TaskTimer* TaskProcesser::CreateTimer(TimerCallBack timerCB, void* data, bool isRepeat)
	{
		TaskTimer* timer = TaskObjectPool::GetInstance().TimerPool().Pop();
		timer->Init(this, timerCB, data, isRepeat);
		return timer;
	}

	//生成定时器
	TaskTimer* TaskProcesser::CreateTimer(TimerCallBack timerCB, void* data, int durationMS, bool isRepeat)
	{
		TaskTimer* timer = TaskObjectPool::GetInstance().TimerPool().Pop();
		timer->Init(this, timerCB, data, durationMS, isRepeat);
		return timer;
	}

	//回收定时器 
	void TaskProcesser::RecycleTimer(TaskTimer* timer)
	{
		if (timer == nullptr)
			return;
		if (isStop)
			TaskObjectPool::GetInstance().TimerPool().Push(timer);
		timer->Remove();
	}


	void TaskProcesser::PostBlockFilterSingle(int filterNumber)
	{
		if (isStop)
			return;

		Task* task = PopTask(TaskMsg::TMSG_TASK_BLOCK_FILTER_SINGLE);
		task->processCallBack = nullptr;
		task->data = (void*)filterNumber;
		task->filterNum = -1;
		PostTask(task);
	}

	void TaskProcesser::PostUnBlockFilter()
	{
		if (isStop)
			return;

		Task* task = PopTask(TaskMsg::TMSG_TASK_NOT_BLOCK_FILTER);
		task->filterNum = -1;
		PostTask(task);
	}

	void TaskProcesser::PostRemoveTask(TaskCompareCallBack cmpCB, void* data, int delay)
	{
		if (isStop)
			return;

		Task* task = PopTask(TaskMsg::TMSG_TASK_REMOVE);
		task->cmpCallBack = cmpCB;
		task->data = data;
		PostTask(task, delay);

	}

	void TaskProcesser::PostTask(Task* task, int delay)
	{
		if (isStop)
			return;

		if (isLock)
			Lock();

		if (task->msg == TaskMsg::TMSG_TASK_REMOVE)
			task->SetPriority(TASK_MIN_PRIORITY);

		int64_t curtTime = GetCurrentTimeMsec();
		task->delay = (int64_t)delay;
		task->createTimeMS = curtTime;
		task->executeTimeMS = curtTime - startTime + task->delay;
		if (task->priority < 0)
			task->executeTimeMS = task->priority;

		writeQue->Add(task);


		if (isLock)
		{
			UnLock();
			Notify();
		}


	}

	int TaskProcesser::Stop()
	{
		if (isStop)
			return 0;

		Continue();
		Task* task = PopTask(TaskMsg::TMSG_QUIT);
		PostTask(task, 0);
		isStop = true;

		quitSem.wait();

		if (!isLock) {
			readQue->Release();
			writeQue->Release();
			return 0;
		}

		readQue->Release();
		writeQue->Release();
		return 0;
	}

	int TaskProcesser::Pause()
	{
		if (isStop)
			return 0;

		Task* task = PopTask(TaskMsg::TMSG_PAUSE);
		PostTask(task, 0);
		return 0;
	}

	int TaskProcesser::Continue()
	{
		if (isLock)
			pauseSem.set();
		return 0;
	}


	int TaskProcesser::ProcessTask(Task* task)
	{
		if (task->processCallBack != nullptr) {
			task->processCallBack(task);
		}

		switch (task->msg)
		{
		case  TaskMsg::TMSG_TIMER_STOP:
		case  TaskMsg::TMSG_TIMER_REMOVE:
		{
			TaskTimer* timer = ((TimerTask*)task)->timer;
			if (timer->runTask != nullptr) {
				Task* tsk = timer->runTask;
				timer->runTask = nullptr;
				//
				bool isRemove;
				if (isLock) {
					Lock();
					isRemove = writeQue->Remove(tsk);
					UnLock();
				}
				else {
					isRemove = writeQue->Remove(tsk);
				}
				if (!isRemove)
					readQue->Remove(tsk);

				Task::Kill(task);
				Task::Release(tsk);
			}

			if (task->msg == TaskMsg::TMSG_TIMER_REMOVE) {
				timer->Clear();
				TaskObjectPool::GetInstance().TimerPool().Push(timer);
			}
		}
		break;


		case TaskMsg::TMSG_TASK_REMOVE:
		{
			if (task->cmpCallBack == nullptr)
				break;

			cmpRemoveTask = task;
			//
			if (isLock)Lock();
			writeQue->Traversal();
			if (isLock)UnLock();
			//
			readQue->Traversal();
		}
		break;

		case TaskMsg::TMSG_TASK_BLOCK_FILTER_SINGLE:
		{
			taskBlockFilterCount = 1;
			taskBlockFilterNumbers[0] = (uintptr_t)task->data;
			readQue->SetBlockFilter(taskBlockFilterNumbers, taskBlockFilterCount);
		}
		break;

		case TaskMsg::TMSG_TASK_BLOCK_FILTER:
		{
			int* filter = (int*)task->data;
			taskBlockFilterCount = 0;
			for (int i = 0; filter[i] != -1; i++) {
				taskBlockFilterNumbers[i] = filter[i];
				taskBlockFilterCount++;
			}

			readQue->SetBlockFilter(taskBlockFilterNumbers, taskBlockFilterCount);
		}
		break;

		case TaskMsg::TMSG_TASK_NOT_BLOCK_FILTER:
		{
			taskBlockFilterCount = 0;
			readQue->SetBlockFilter(nullptr, taskBlockFilterCount);

		}
		break;

		case TaskMsg::TMSG_QUIT:
			return 1;

		case TaskMsg::TMSG_PAUSE:
			if (isLock)
				pauseSem.wait();
			break;
		}

		return 0;
	}


	void TaskProcesser::Run()
	{
		if (isStop)
			return;

#ifdef _WIN32
		if (enableCoroutine) {
			mainFiberHandle = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
		}
#endif // _WIN32

		int64_t waitTime, tm, curTime;

		for (;;)
		{
			curTime = GetCurrentTimeMsec();
			readQue->SetBlockFilter(taskBlockFilterNumbers, taskBlockFilterCount);
			tm = curTime - startTime;

			if (!readQue->Read(tm))
			{
				if (isLock)
					quitSem.set();
				return;
			}

			if (isLock)
			{
				Lock();

				if (!isFixedFps && !isEmbed) {
					while (writeQue->IsEmpty()) {

						UnLock();

						waitTime = readQue->GetWaitTimeMS(tm);
						if (waitTime < 0) {
							Wait();
						}
						else {
							Wait((uint32_t)waitTime);
							Lock();
							break;
						}

						Lock();
					}
				}
			}

			readQue->Merge(*writeQue);


			if (isLock)
				UnLock();


			//固定帧率逻辑
			if (isFixedFps)
			{
				//当前结束时间
				int64_t endTime = GetCurrentTimeMsec();
				int64_t tm = endTime - curTime;
				int64_t diffMs = (int64_t)perFrameDuration - tm;
				if (diffMs >= 1)
				{
					this_thread::sleep_for(std::chrono::milliseconds(diffMs));
				}

			}

			if (isEmbed)
				return;
		}
	}


	int TaskProcesser::_ReadTaskList(Task* task)
	{
		int ret = 0;
		if (!task->isRemove)
			ret = ProcessTask(task);
		else
			Task::Kill(task);

		Task::Release(task);
		return ret;
	}

	int TaskProcesser::_RemoveTaskProcess(Task* task)
	{
		if (cmpRemoveTask == nullptr)
			return 1;

		if (!task->isRemove &&
			cmpRemoveTask->cmpCallBack(cmpRemoveTask, task))
			task->isRemove = true;

		return 0;
	}

	int TaskProcesser::_ReleaseProcess(Task* task)
	{
		Task::Kill(task);
		Task::Release(task);
		return 0;
	}

}
