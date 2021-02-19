#ifndef _RegionSounderThread_h_
#define _RegionSounderThread_h_

#include "VentrueTypes.h"

namespace ventrue
{
	//by cymheart, 2020--2021.
	class RegionSounderThreadData
	{
	public:
		RegionSounderThreadData(Ventrue* ventrue);
		~RegionSounderThreadData();

		void SetFrameBuffer();

		// 唤醒线程
		void Wake(RegionSounder* regionSounder);
		void Wake();

		// 线程休眠    
		void Sleep();

	public:
		Ventrue* ventrue = nullptr;
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
		RegionSounderThread();
		~RegionSounderThread();

		void Render(RegionSounder** regionSounders, int count);
		void Wait();
		void Start();
		void Stop();


		void SetVentrue(Ventrue* ventrue)
		{
			this->ventrue = ventrue;
		}

	private:

		void Render(RegionSounder* regionSounder);
		static void KeySounderProcesserThread(void* param);
		void NextQueueItemProcess(RegionSounderThreadData* regionSounderThread);

	private:
		Ventrue* ventrue = nullptr;
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
