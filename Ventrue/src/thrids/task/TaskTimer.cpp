#include"TaskTimer.h"

namespace task
{
	TaskTimer::TaskTimer(TaskProcesser* taskProcesser,
		TimerCallBack timerCB,
		void* data, int durationMS, bool isRepeat)
		:isStop(true)
		, task(nullptr)
	{
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

	TaskTimer::~TaskTimer()
	{
	}

	void TaskTimer::Start()
	{
		task = new TimerTask();
		task->timer = this;
		task->msg = TaskMsg::TMSG_TIMER_START;
		task->processCallBack = StartTask;
		taskProcesser->PostTask(task);
	}

	void TaskTimer::Stop()
	{
		TimerTask* t = new TimerTask();
		t->timer = this;
		t->msg = TaskMsg::TMSG_TIMER_STOP;
		taskProcesser->PostTask(t);
	}

	void TaskTimer::PostTask(int tm)
	{
		task = new TimerTask();
		task->timer = this;
		task->msg = TaskMsg::TMSG_TIMER_RUN;
		task->processCallBack = RunTask;
		taskProcesser->PostTask(task, tm);
	}

	void TaskTimer::RunTask(Task* task)
	{
		TimerTask* timeTask = (TimerTask*)task;
		TaskTimer& timer = *(timeTask->timer);

		if (timer.isStop)
			return;

		timer.task = nullptr;

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

		if (timer.isStop)
			return;

		timer.isStop = false;
		timer.PostTask(0);
	}

}
