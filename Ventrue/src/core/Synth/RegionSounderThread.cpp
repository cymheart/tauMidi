#include"RegionSounderThread.h"
#include"RegionSounder.h"
#include"Ventrue.h"
#include"VirInstrument.h"
#include"scutils/Utils.h"

namespace ventrue
{
	RegionSounderThreadData::RegionSounderThreadData(Ventrue* ventrue)
	{
		this->ventrue = ventrue;
		leftChannelFrameBuffer = (float*)malloc(sizeof(float) * ventrue->GetChildFrameSampleCount());
		rightChannelFrameBuffer = (float*)malloc(sizeof(float) * ventrue->GetChildFrameSampleCount());
	}

	RegionSounderThreadData::~RegionSounderThreadData()
	{
		free(leftChannelFrameBuffer);
		free(rightChannelFrameBuffer);
	}


	void RegionSounderThreadData::SetFrameBuffer()
	{
		regionSounder->SetFrameBuffer(leftChannelFrameBuffer, rightChannelFrameBuffer);
	}


	// 唤醒线程
	void RegionSounderThreadData::Wake(RegionSounder* regionSounder)
	{
		this->regionSounder = regionSounder;
		waitSem.set();
	}

	void RegionSounderThreadData::Wake()
	{
		waitSem.set();
	}

	// 线程休眠
	void RegionSounderThreadData::Sleep()
	{
		waitSem.wait();
	}


	RegionSounderThread::RegionSounderThread()
		:threadStopedCount(0)
	{
	}

	RegionSounderThread::~RegionSounderThread()
	{
		Stop();
	}

	void RegionSounderThread::Start()
	{
		if (runState != RunState::Stoped)
			return;

		runState = RunState::Running;
		threadStopedCount = 0;
		coreCount = ScUtils_GetCPUCount();
		if (coreCount > 4) coreCount /= 2;
		freeThreadIdx = coreCount - 1;
		for (int i = 0; i < coreCount; i++)
		{
			thread t(KeySounderProcesserThread, this);
			t.detach();
		}
	}

	void RegionSounderThread::Wait()
	{
		waitSem.wait();
	}

	void RegionSounderThread::Stop()
	{
		if (runState == RunState::Stoped ||
			runState == RunState::Stoping)
			return;

		isStop = true;
		runState = RunState::Stoping;

		while (threadStopedCount < coreCount)
		{
			for (int i = 0; i < threads.size(); i++)
				threads[i]->Wake();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		for (int i = 0; i < threads.size(); i++)
		{
			DEL(threads[i]);
		}

		runState = RunState::Stoped;
	}

	void RegionSounderThread::Render(RegionSounder** regionSounders, int count)
	{
		processCount = count;
		for (int i = 0; i < count; i++)
			Render(regionSounders[i]);
	}

	void RegionSounderThread::Render(RegionSounder* regionSounder)
	{
		RegionSounderThreadData* rsthread = nullptr;
		locker.lock();

		if (freeThreadIdx == -1)
		{
			regionSounderQueue.push_back(regionSounder);
			locker.unlock();
			return;
		}

		if (threads.size() > 0) {
			rsthread = threads[freeThreadIdx];
			freeThreadIdx--;
		}

		locker.unlock();

		if (rsthread != nullptr)
			rsthread->Wake(regionSounder);
	}

	void RegionSounderThread::KeySounderProcesserThread(void* param)
	{
		RegionSounderThread& self = *(RegionSounderThread*)param;
		RegionSounderThreadData* regionSounderThreadData = new RegionSounderThreadData(self.ventrue);

		self.locker.lock();
		self.threads.push_back(regionSounderThreadData);
		self.locker.unlock();

		while (!self.isStop)
		{
			regionSounderThreadData->Sleep();
			if (self.isStop)
				break;

			if (regionSounderThreadData->regionSounder != nullptr)
			{
				regionSounderThreadData->SetFrameBuffer();
				regionSounderThreadData->regionSounder->Render();
				self.NextQueueItemProcess(regionSounderThreadData);
			}
		}

		self.threadStopedCount++;
	}

	void RegionSounderThread::NextQueueItemProcess(RegionSounderThreadData* regionSounderThreadData)
	{
		RegionSounder* regionSounder = nullptr;

		locker.lock();

		regionSounderThreadData->regionSounder->GetVirInstrument()->
			CombineRegionSounderSamples(regionSounderThreadData->regionSounder);

		processCount--;
		freeThreadIdx++;
		threads[freeThreadIdx] = regionSounderThreadData;

		if (regionSounderQueue.size() > 0)
		{
			regionSounder = regionSounderQueue.back();
			regionSounderQueue.pop_back();
			freeThreadIdx--;
		}
		else
		{
			if (processCount == 0)
				waitSem.set();

			locker.unlock();
			return;
		}

		locker.unlock();

		regionSounderThreadData->Wake(regionSounder);
	}
}
