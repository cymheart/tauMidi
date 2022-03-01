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
		writeQue->SetReleaseCallback(ReleaseProcess);
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

	int TaskProcesser::Start(bool isRunNewThread)
	{
		if (!isStop)
			return 0;

		startTime = GetCurrentTimeMsec();

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
		return new TaskTimer(this, timerCB, data, isRepeat);
	}

	//生成定时器
	TaskTimer* TaskProcesser::CreateTimer(TimerCallBack timerCB, void* data, int durationMS, bool isRepeat)
	{
		return new TaskTimer(this, timerCB, data, durationMS, isRepeat);
	}

	void TaskProcesser::PostBlockFilterSingle(int filterNumber)
	{
		Task* task = PopTask(TaskMsg::TMSG_TASK_BLOCK_FILTER_SINGLE);
		task->processCallBack = nullptr;
		task->data = (void*)filterNumber;
		task->filterNum = -1;
		PostTask(task);
	}

	void TaskProcesser::PostUnBlockFilter()
	{
		Task* task = PopTask(TaskMsg::TMSG_TASK_NOT_BLOCK_FILTER);
		task->filterNum = -1;
		PostTask(task);
	}

	Task* TaskProcesser::PostTask(TaskCallBack taskCallBack)
	{
		return PostTask(taskCallBack, nullptr, 0, 0);
	}

	Task* TaskProcesser::PostTask(TaskCallBack taskCallBack, int delay)
	{
		return PostTask(taskCallBack, nullptr, delay, 0);
	}

	Task* TaskProcesser::PostTask(TaskCallBack taskCallBack, void* data)
	{
		return PostTask(taskCallBack, data, 0, 0);
	}

	Task* TaskProcesser::PostTask(TaskCallBack taskCallBack, void* data, int delay)
	{
		return PostTask(taskCallBack, data, delay, 0);
	}

	Task* TaskProcesser::PostTaskByFilter(TaskCallBack taskCallBack, int filterNumber)
	{
		return PostTask(taskCallBack, nullptr, 0, filterNumber);
	}

	Task* TaskProcesser::PostTaskByFilter(TaskCallBack taskCallBack, int delay, int filterNumber)
	{
		return PostTask(taskCallBack, nullptr, delay, filterNumber);
	}

	Task* TaskProcesser::PostTaskByFilter(TaskCallBack taskCallBack, void* data, int filterNumber)
	{
		return PostTask(taskCallBack, data, 0, filterNumber);
	}


	Task* TaskProcesser::PostTask(TaskCallBack taskCallBack, void* data, int delay, int filterNumber)
	{
		Task* task = new Task(TaskMsg::TMSG_DATA);
		task->processCallBack = taskCallBack;
		task->data = data;
		task->filterNum = filterNumber;
		PostTask(task, delay);

		return task;
	}


	Task* TaskProcesser::PostTask(Task* task) {
		return PostTask(task, 0);
	}

	Task* TaskProcesser::PostTask(Task* task, int delay)
	{
		if (isLock)
			Lock();

		if (task->msg == TaskMsg::TMSG_TASK_REMOVE)
			task->SetPriority(TASK_MIN_PRIORITY);

		int64_t curtTime = GetCurrentTimeMsec();
		task->delay = (int64_t)delay;
		task->createTimeMS = curtTime;
		task->executeTimeMS = curtTime - startTime + task->delay;
		writeQue->Add(task);


		if (isLock)
		{
			UnLock();
			Notify();
		}

		return task;
	}

	int TaskProcesser::Stop()
	{
		if (isStop)
			return 0;

		isStop = true;
		Continue();

		Task* task = PopTask(TMSG_QUIT);
		PostTask(task, 0);


		if (!isLock) {
			readQue->Release();
			writeQue->Release();
			return 0;
		}

		quitSem.wait();

		readQue->Release();
		writeQue->Release();
		return 0;
	}

	int TaskProcesser::Pause()
	{
		Task* task = PopTask(TMSG_PAUSE);
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
		case  TMSG_TIMER_STOP:
		{
			TaskTimer* timer = ((TimerTask*)task)->timer;
			if (timer->runTask != nullptr) {
				Task* tsk = timer->runTask;
				if (!writeQue->Remove(tsk))
					readQue->Remove(tsk);
			}

		}
		break;

		case TMSG_TASK_REMOVE:
		{
			if (task->cmpCallBack == nullptr)
				break;

			cmpRemoveTask = task;
			readQue->Traversal();
		}
		break;

		case TMSG_TASK_BLOCK_FILTER_SINGLE:
		{
			taskBlockFilterCount = 1;
			taskBlockFilterNumbers[0] = (uintptr_t)task->data;
			readQue->SetBlockFilter(taskBlockFilterNumbers, taskBlockFilterCount);
		}
		break;

		case TMSG_TASK_BLOCK_FILTER:
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

		case TMSG_TASK_NOT_BLOCK_FILTER:
		{
			taskBlockFilterCount = 0;
			readQue->SetBlockFilter(nullptr, taskBlockFilterCount);

		}
		break;

		case TMSG_QUIT:
			return 1;

		case TMSG_PAUSE:
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

		int64_t waitTime;
		int64_t tm;
		int64_t curTime;

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
							Wait(waitTime);
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
				int64_t diffMs = perFrameDuration - tm;
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
		Task::Release(task);

		return ret;
	}

	int TaskProcesser::_RemoveTaskProcess(Task* task)
	{
		if (cmpRemoveTask == nullptr)
			return 1;

		if (cmpRemoveTask->cmpCallBack(cmpRemoveTask, task) == true)
			task->isRemove = true;
		return 1;

	}

	int TaskProcesser::_ReleaseProcess(Task* task)
	{
		Task::Release(task);
		return 0;
	}

}
