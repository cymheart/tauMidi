#include"TaskProcesser.h"
#include"TaskTimer.h"


namespace task
{
	TaskProcesser::TaskProcesser()
	{
		timerCompare = new TimerCompare();
		taskQue = new TaskQueue();

		readList = taskQue->readList;
		writeList = taskQue->writeList;
		timerReadList = taskQue->timerReadList;
		timerWriteList = taskQue->timerWriteList;

		//
		readListSelfThread = taskQue->readListSelfThread;
		writeListSelfThread = taskQue->writeListSelfThread;
		timerReadListSelfThread = taskQue->timerReadListSelfThread;
		timerWriteListSelfThread = taskQue->timerWriteListSelfThread;
	}

	TaskProcesser::~TaskProcesser()
	{
		Stop();
		DEL(timerCompare);
		DEL(taskQue);
	}

	//打开嵌入模式
	void TaskProcesser::OpenEmbedMode()
	{
		SetPostTaskToMainThreadProcess(false);
		SetFixedFrameRate(false);
		SetLockMode(false);
		SetEmbedMode(true);
	}

	//打开单线程固定帧率模式
	void TaskProcesser::OpenSingleThreadFixedFrameRateMode()
	{
		SetPostTaskToMainThreadProcess(false);
		SetFixedFrameRate(true);
		SetLockMode(false);
		SetEmbedMode(false);
	}

	//打开线程固定帧率模式
	void TaskProcesser::OpenThreadFixedFrameRateMode()
	{
		SetPostTaskToMainThreadProcess(false);
		SetFixedFrameRate(true);
		SetLockMode(true);
		SetEmbedMode(false);
	}


	int TaskProcesser::Start(bool isRunNewThread)
	{
		if (!isStop)
			return 0;

		startTime = clock::now();
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

		return 0;
	}

	void TaskProcesser::ThreadRunCallBack(void* param)
	{
		TaskProcesser* taskProcesser = (TaskProcesser*)param;
		taskProcesser->Run();
	}


	int TaskProcesser::PostTask(TaskCallBack taskCallBack)
	{
		return PostTask(taskCallBack, nullptr, 0);
	}

	int TaskProcesser::PostTask(TaskCallBack taskCallBack, int delay)
	{
		return PostTask(taskCallBack, nullptr, delay);
	}

	int TaskProcesser::PostTask(TaskCallBack taskCallBack, void* data)
	{
		return PostTask(taskCallBack, data, 0);
	}

	int TaskProcesser::PostTask(TaskCallBack taskCallBack, void* data, int delay)
	{
		Task* task = new Task(TaskMsg::TMSG_DATA);
		task->processCallBack = taskCallBack;
		task->data = data;
		PostTask(task, delay);

		return 0;
	}


	int TaskProcesser::PostTask(Task* task) {
		return PostTask(task, 0);
	}

	int TaskProcesser::PostTask(Task* task, int delay)
	{
		if (isFixedFps)
		{
			thread::id curtFuncAtThread = this_thread::get_id();
			if (curtFuncAtThread == threadID)
				return PostTaskDirect(task, delay, true);
		}

		int ret = 0;
		if (isLock)
			taskQue->Lock();

		ret = PostTaskDirect(task, delay, false);

		if (isLock)
		{
			taskQue->UnLock();

			//设置任务处理可读信号
			if (!isFixedFps)
				taskQue->Notify();
		}

		return ret;
	}

	int TaskProcesser::PostTaskDirect(Task* task, int delay, bool isFromSelfThread)
	{
		int ret;
		switch (task->msg)
		{
		case TMSG_TIMER_START:
			ret = PostTimerTask(task, -2000, isFromSelfThread);
			break;

		case TMSG_TIMER_RUN:
			ret = PostTimerTask(task, delay, isFromSelfThread);
			break;

		case TMSG_TIMER_STOP:
			ret = PostCommonTask(task, -1000, isFromSelfThread);
			break;

		default:
			ret = PostCommonTask(task, delay, isFromSelfThread);
		}

		return ret;
	}


	int TaskProcesser::PostCommonTask(Task* task, int delay, bool isFromSelfThread)
	{
		int ret = 0;
		task->delay = delay;

		if (delay != 0)
			task->startTime = clock::now();

		if (delay == 0) {
			if (isFromSelfThread) { taskQue->AddToWriteListSelfThread(task); }
			else { taskQue->AddToWriteList(task); }
		}
		else {
			if (isFromSelfThread) { taskQue->AddToTimerWriteListSelfThread(task); }
			else { taskQue->AddToTimerWriteList(task); }
		}

		return ret;
	}


	int TaskProcesser::PostTimerTask(Task* task, int delay, bool isFromSelfThread)
	{
		task->delay = delay;
		task->startTime = clock::now();
		if (isFromSelfThread) { taskQue->AddToTimerWriteListSelfThread(task); }
		else { taskQue->AddToTimerWriteList(task); }
		return 0;
	}

	int TaskProcesser::Stop()
	{
		if (isStop)
			return 0;

		isStop = true;
		Continue();

		Task* node = new Task(TMSG_QUIT);
		if (PostTask(node, 0) != 0)
			return -1;

		if (!isLock)
			return 0;

		quitSem.wait();

		taskQue->Clear();
		return 0;
	}

	int TaskProcesser::Pause()
	{
		Task* node = new Task(TMSG_PAUSE);
		if (PostTask(node, 0) != 0)
			return -1;

		return 0;
	}

	int TaskProcesser::Continue()
	{
		if (isLock)
			pauseSem.set();
		return 0;
	}


	void TaskProcesser::Run()
	{
		if (isStop)
			return;

		clock::time_point curTime;
		int ret;

		for (;;)
		{
			curTime = clock::now();

			//处理其它线程过来的定时任务
			ret = ReadTimerList(*timerReadList, curTime);
			if (ret == -1)
				return;

			//处理本线程过来的定时任务
			ret = ReadTimerList(*timerReadListSelfThread, curTime);
			if (ret == -1)
				return;

			//
			ret = ReadCommonList(*readList);
			if (ret == -1)
				return;

			//
			ret = ReadCommonList(*readListSelfThread);
			if (ret == -1)
				return;


			if (isLock)
			{
				taskQue->Lock();

				if (!isFixedFps)
				{
					while (writeList->empty() &&
						timerWriteList->empty())
					{
						taskQue->UnLock();

						if (minDelay == MAX_DELAY_TIME)
						{
							taskQue->Wait();
						}
						else
						{
							//printf("minDelay：%d\n", minDelay);
							//int start = SDL_GetTicks();

							taskQue->Wait(minDelay);

							//int end = SDL_GetTicks();
							//printf("wait间隔：%d\n", end - start);
							//printf("-------------------------------\n");

							taskQue->Lock();
							minDelay = MAX_DELAY_TIME;
							break;
						}

						taskQue->Lock();
					}
				}
			}

			//common task
			if (!writeList->empty()) {
				taskQue->writeList = readList;
				taskQue->readList = writeList;
				readList = taskQue->readList;
				writeList = taskQue->writeList;
			}

			//timer task
			if (!timerWriteList->empty())
			{
				minDelay = MAX_DELAY_TIME;

				if (!timerReadList->empty())
				{
					timerReadList->splice(timerReadList->end(), *timerWriteList);
				}
				else
				{
					taskQue->timerWriteList = timerReadList;
					taskQue->timerReadList = timerWriteList;
					timerReadList = taskQue->timerReadList;
					timerWriteList = taskQue->timerWriteList;
				}

				timerCompare->curTime = curTime;
				timerReadList->sort(*timerCompare);
			}


			if (isLock) {
				taskQue->UnLock();
			}

			//common task self thread
			if (!writeListSelfThread->empty()) {
				taskQue->writeListSelfThread = readListSelfThread;
				taskQue->readListSelfThread = writeListSelfThread;
				readListSelfThread = taskQue->readListSelfThread;
				writeListSelfThread = taskQue->writeListSelfThread;
			}

			//timer task self thread
			if (!timerWriteListSelfThread->empty())
			{
				minDelay = MAX_DELAY_TIME;

				if (!timerReadListSelfThread->empty())
				{
					timerReadListSelfThread->splice(timerReadListSelfThread->end(), *timerWriteListSelfThread);
				}
				else {
					taskQue->timerWriteListSelfThread = timerReadListSelfThread;
					taskQue->timerReadListSelfThread = timerWriteListSelfThread;
					timerReadListSelfThread = taskQue->timerReadListSelfThread;
					timerWriteListSelfThread = taskQue->timerWriteListSelfThread;
				}

				timerCompare->curTime = curTime;
				timerReadListSelfThread->sort(*timerCompare);
			}

			//固定帧率逻辑
			if (isFixedFps)
			{
				//当前结束时间
				endTime = clock::now();
				//计算当前结束时间应该具有的标准帧总数
				float tm = chrono::duration_cast<res>(curTime - startTime).count() * 0.001f;
				float totalFrame = tm / perFrameDuration;
				float useFrame = totalFrame - floor(totalFrame);
				float frameLeaveTime;
				if (useFrame <= 0.001f) {
					frameLeaveTime = 0;
				}
				else {
					//1 - 获取总帧数的小数部分所需要的时间，这个部分表示为当前标准帧剩余的时间量(单位: ms)
					frameLeaveTime = (1 - useFrame) * perFrameDuration;
				}

				//同步到标准总帧的下一个标准帧位置上
				if (frameLeaveTime > 0.01f) {
					this_thread::sleep_for(std::chrono::milliseconds((long)frameLeaveTime));
				}
			}

			if (isEmbed)
				return;
		}
	}


	//遍历定时任务列表
	int TaskProcesser::ReadTimerList(TaskList& timerReads, clock::time_point curTime)
	{
		Task* cur = nullptr;

		if (!timerReads.empty())
		{
			int32_t curDelay = 0;
			for (TaskList::iterator iter = timerReads.begin(); iter != timerReads.end();)
			{
				cur = (*iter);
				if (cur == nullptr)
					continue;

				if (cur->isRemove)
				{
					Task::Release(cur);
					iter = timerReads.erase(iter);
					continue;
				}

				int tm = (int)(chrono::duration_cast<res>(curTime - cur->startTime).count() * 0.001f);
				if (cur->delay > 0 && tm < cur->delay)
				{
					curDelay = (int32_t)(cur->delay - tm);
					if (curDelay < minDelay)
						minDelay = curDelay;
					break;
				}

				if (cur->msg == TMSG_TIMER_STOP) {
					TaskTimer* timer = ((TimerTask*)cur)->timer;
					if (timer->task != nullptr) {
						timer->task->isRemove = true;
					}
				}
				else if (cur->msg == TMSG_TIMER_RESTART) {
					TaskTimer* timer = ((TimerTask*)cur)->timer;
					if (timer->task != nullptr) {
						timer->task->startTime = curTime;
					}
				}

				if (ProcessTask(cur) == 1)
				{
					isStop = true;
					taskQue->Clear();
					return -1;
				}

				Task::Release(cur);
				iter = timerReads.erase(iter);
			}
		}

		return 0;
	}


	/**遍历数据任务列表*/
	int TaskProcesser::ReadCommonList(TaskList& reads)
	{
		Task* cur = nullptr;
		if (!reads.empty())
		{
			for (TaskList::iterator iter = reads.begin(); iter != reads.end();)
			{
				cur = (*iter);

				if (!cur->isRemove && ProcessTask(cur) == 1)
				{
					isStop = true;
					taskQue->Clear();
					return -1;
				}

				Task::Release(cur);
				iter = reads.erase(iter);
			}
		}

		return 0;
	}

	int TaskProcesser::ProcessTask(Task* task)
	{
		if (task->processCallBack != nullptr) {
			if (isPostTaskToMainThreadProcess && mainThreadTaskProcesser != nullptr) {
				mainThreadTaskProcesser->PostTask(task);
			}
			else {
				task->processCallBack(task);
			}
		}

		switch (task->msg) {
		case TMSG_QUIT:
			if (isLock)
				quitSem.set();
			return 1;

		case TMSG_PAUSE:
			if (isLock)
				pauseSem.wait();
			break;
		}

		return 0;
	}




}
