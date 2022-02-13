#ifndef _RegionSounderThread_h_
#define _RegionSounderThread_h_

#include "TauTypes.h"
#include"scutils/Semaphore.h"
using namespace scutils;

namespace tau
{
	//by cymheart, 2020--2021.
	class RegionSounderThreadData
	{
	public:
		RegionSounderThreadData(Tau* tau);
		~RegionSounderThreadData();

		void SetFrameBuffer();

		// 唤醒线程
		void Wake(RegionSounder* regionSounder);
		void Wake();

		// 线程休眠    
		void Sleep();

	public:
		Tau* tau = nullptr;
		RegionSounder* regionSounder = nullptr;
		float* leftChannelFrameBuffer = nullptr;
		float* rightChannelFrameBuffer = nullptr;


	private:
		Semaphore waitSem;

	};

	class RegionSounderThread
	{
		enum RunState
		{
			Stoping,
			Stoped,
			Running
		};

	public:
		RegionSounderThread(Synther* synther);
		~RegionSounderThread();

		void Render(RegionSounder** regionSounders, int count);
		void Wait();
		void Start();
		void Stop();

	private:

		void Render(RegionSounder* regionSounder);
		static void KeySounderProcesserThread(void* param);
		void NextQueueItemProcess(RegionSounderThreadData* regionSounderThread);

	private:
		Synther* synther = nullptr;
		Tau* tau = nullptr;
		atomic_int threadStopedCount;
		RegionSounderQueue regionSounderQueue;
		RegionSounderThreadDataList threads;
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
