#include"TaskTimer.h"
#include"TaskObjectPool.h"

namespace task
{
	//TimerTask	
	void TimerTask::Release(Task* task)
	{
		TaskObjectPool::GetInstance().TimerTaskPool().Push((TimerTask*)task);
	}

	//TaskTimer
	TaskTimer::TaskTimer(TaskProcesser* taskProcesser,
		TimerCallBack timerCB,
		void* data, int durationMS, bool isRepeat)
	{

		Init(taskProcesser, timerCB, data, durationMS, isRepeat);
	}

	TaskTimer::~TaskTimer()
	{
		Clear();
	}

	void TaskTimer::Clear()
	{
		timerCB = nullptr;
		data = nullptr;
		taskProcesser = nullptr;
		durationMS = 0;
		isRepeat = true;
		runTask = nullptr;
		isStop = true;
		state = TaskMsg::TMSG_TIMER_STOP;
	}

	void TaskTimer::Init(TaskProcesser* taskProcesser,
		TimerCallBack timerCB,
		void* data, bool isRepeat)
	{
		float fps = taskProcesser->GetFrameRate();
		int tm = (int)(1000 / fps);
		Init(taskProcesser, timerCB, data, tm, isRepeat);
	}

	void TaskTimer::Init(TaskProcesser* taskProcesser,
		TimerCallBack timerCB,
		void* data, int durationMS, bool isRepeat)
	{
		state = TaskMsg::TMSG_TIMER_STOP;
		runTask = nullptr;
		isStop = true;
		this->timerCB = timerCB;
		this->data = data;
		this->taskProcesser = taskProcesser;
		this->durationMS = durationMS;
		this->isRepeat = isRepeat;

		if (durationMS < 0)
		{
			float fps = taskProcesser->GetFrameRate();
			this->durationMS = (int)(1000 / fps);
		}
	}

	void TaskTimer::Start()
	{
		if (state == TaskMsg::TMSG_TIMER_REMOVE)
			return;

		state = TaskMsg::TMSG_TIMER_START;
		TimerTask* task = TaskObjectPool::GetInstance().TimerTaskPool().Pop();
		task->timer = this;
		task->msg = TaskMsg::TMSG_TIMER_START;
		task->processCallBack = StartTask;
		taskProcesser->PostTask(task);
	}

	void TaskTimer::ReStart()
	{
		Stop();
		Start();
	}

	void TaskTimer::Stop()
	{
		if (state == TaskMsg::TMSG_TIMER_REMOVE)
			return;

		state = TaskMsg::TMSG_TIMER_STOP;
		TimerTask* t = TaskObjectPool::GetInstance().TimerTaskPool().Pop();
		t->timer = this;
		t->msg = TaskMsg::TMSG_TIMER_STOP;
		t->processCallBack = StopTask;
		taskProcesser->PostTask(t);
	}

	void TaskTimer::Remove()
	{
		if (state == TaskMsg::TMSG_TIMER_REMOVE)
			return;

		state = TaskMsg::TMSG_TIMER_REMOVE;
		TimerTask* t = TaskObjectPool::GetInstance().TimerTaskPool().Pop();
		t->timer = this;
		t->msg = TaskMsg::TMSG_TIMER_REMOVE;
		t->processCallBack = StopTask;
		taskProcesser->PostTask(t);
	}

	void TaskTimer::PostTask(int tm)
	{
		runTask = TaskObjectPool::GetInstance().TimerTaskPool().Pop();
		runTask->timer = this;
		runTask->msg = TaskMsg::TMSG_TIMER_RUN;
		runTask->processCallBack = RunTask;
		taskProcesser->PostTask(runTask, tm);
	}

	void TaskTimer::RunTask(Task* task)
	{
		TimerTask* timeTask = (TimerTask*)task;
		TaskTimer& timer = *(timeTask->timer);
		timer.runTask = nullptr;

		if (timer.isStop)
			return;

		if (timer.timerCB != nullptr)
			timer.timerCB(timer.data);

		if (timer.isRepeat)
			timer.PostTask(timer.durationMS);
		else
			timer.isStop = true;
	}

	void TaskTimer::StartTask(Task* task)
	{
		TimerTask* timeTask = (TimerTask*)task;
		TaskTimer& timer = *(timeTask->timer);

		if (!timer.isStop)
			return;

		timer.isStop = false;
		timer.PostTask(timer.durationMS);
	}

	void TaskTimer::StopTask(Task* task)
	{
		TimerTask* timeTask = (TimerTask*)task;
		TaskTimer& timer = *(timeTask->timer);
		timer.isStop = true;
	}

}
