﻿#ifndef _Task_h_
#define _Task_h_

#include"TaskTypes.h"

namespace task
{
	
	class Task
	{
	public:
		Task();
		Task(TaskMsg taskMsg);
		Task(TaskCallBack processDataCallBack, TaskMsg msg = TMSG_DATA);

		//获取执行优先值，越小的值，意味着越早执行
		inline int GetPriority() { return priority; }

		//设置执行优先值，越小的值，意味着越早执行	
		inline void SetPriority(int priority)
		{
			if (priority > TASK_MAX_PRIORITY) priority = TASK_MAX_PRIORITY;
			if (priority < TASK_MAX_PRIORITY) priority = TASK_MAX_PRIORITY;
			this->priority = priority;
		}

		//设置是否系统负责释放task
		inline void SetSysReleaseTask(bool isSysReleaseTask)
		{
			this->isSysReleaseTask = isSysReleaseTask;
		}

		//释放对象
		static void Release(Task* task);

	public:
		clock::time_point startTime;
		int delay = 0;                //延迟执行时间
		TaskCallBack processCallBack = nullptr;      //处理数据回调函数
		TaskCallBack releaseCallBack = nullptr;      //释放数据回调函数
		TaskMsg msg = TaskMsg::TMSG_DATA;                  //任务标志

		//自定义携带数据
		void* data = nullptr;

		//是否系统负责释放task
		bool isSysReleaseTask = true;

		//执行的优先顺序值，越小的值，意味着越早执行
		int priority = 0;
	
	};
}
#endif