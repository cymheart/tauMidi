#ifndef _ZoneSounderThread_h_
#define _ZoneSounderThread_h_

#include "TauTypes.h"
#include"scutils/Semaphore.h"
#include"InsideModulators.h"
using namespace scutils;

namespace tau
{
	//by cymheart, 2020--2021.
	class ZoneSounderThreadData
	{
	public:
		ZoneSounderThreadData(Tau* tau);
		~ZoneSounderThreadData();

		void SetFrameBuffer();

		// 唤醒线程
		void Wake(ZoneSounder* zoneSounder);
		void Wake();

		// 线程休眠    
		void Sleep();

	public:
		Tau* tau = nullptr;
		ZoneSounder* zoneSounder = nullptr;
		float* leftChannelFrameBuffer = nullptr;
		float* rightChannelFrameBuffer = nullptr;

	private:
		Semaphore waitSem;

	};

	class ZoneSounderThread
	{
		enum RunState
		{
			Stoping,
			Stoped,
			Running
		};

	public:
		ZoneSounderThread(Synther* synther);
		~ZoneSounderThread();

		void Render(ZoneSounder** ZoneSounders, int count);
		void Wait();
		void Start();
		void Stop();

	private:

		void Render(ZoneSounder* ZoneSounder);
		static void KeySounderProcesserThread(void* param);
		void NextQueueItemProcess(ZoneSounderThreadData* ZoneSounderThread);

	private:
		Synther* synther = nullptr;
		Tau* tau = nullptr;
		atomic_int threadStopedCount;
		ZoneSounderQueue zoneSounderQueue;
		ZoneSounderThreadDataList threads;
		volatile int freeThreadIdx = 0;
		volatile int processCount = 0;
		mutex locker;
		Semaphore waitSem;
		RunState runState = RunState::Stoped;
		bool isStop = false;
		int coreCount = 0;

	};
}

#endif
