#include"ZoneSounderThread.h"
#include"ZoneSounder.h"
#include"Tau.h"
#include"VirInstrument.h"
#include"scutils/Utils.h"
#include"Synther/Synther.h"

namespace tau
{
	ZoneSounderThreadData::ZoneSounderThreadData(Tau* tau)
	{
		this->tau = tau;
		leftChannelFrameBuffer = (float*)malloc(sizeof(float) * tau->GetChildFrameSampleCount());
		rightChannelFrameBuffer = (float*)malloc(sizeof(float) * tau->GetChildFrameSampleCount());
	}

	ZoneSounderThreadData::~ZoneSounderThreadData()
	{
		free(leftChannelFrameBuffer);
		free(rightChannelFrameBuffer);
	}


	void ZoneSounderThreadData::SetFrameBuffer()
	{
		zoneSounder->SetFrameBuffer(leftChannelFrameBuffer, rightChannelFrameBuffer);
	}

	// 唤醒线程
	void ZoneSounderThreadData::Wake(ZoneSounder* zoneSounder)
	{
		this->zoneSounder = zoneSounder;
		waitSem.set();
	}

	void ZoneSounderThreadData::Wake()
	{
		waitSem.set();
	}

	// 线程休眠
	void ZoneSounderThreadData::Sleep()
	{
		waitSem.wait();
	}


	ZoneSounderThread::ZoneSounderThread(Synther* synther)
		:threadStopedCount(0)
	{
		this->synther = synther;
		tau = synther->tau;
	}

	ZoneSounderThread::~ZoneSounderThread()
	{
		Stop();
	}

	void ZoneSounderThread::Start()
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

	void ZoneSounderThread::Wait()
	{
		waitSem.wait();
	}

	void ZoneSounderThread::Stop()
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

	void ZoneSounderThread::Render(ZoneSounder** zoneSounders, int count)
	{
		processCount = count;
		for (int i = 0; i < count; i++)
			Render(zoneSounders[i]);
	}

	void ZoneSounderThread::Render(ZoneSounder* zoneSounder)
	{
		ZoneSounderThreadData* rsthread = nullptr;
		locker.lock();

		if (freeThreadIdx == -1)
		{
			zoneSounderQueue.push_back(zoneSounder);
			locker.unlock();
			return;
		}

		if (threads.size() > 0) {
			rsthread = threads[freeThreadIdx];
			freeThreadIdx--;
		}

		locker.unlock();

		if (rsthread != nullptr)
			rsthread->Wake(zoneSounder);
	}

	void ZoneSounderThread::KeySounderProcesserThread(void* param)
	{
		ZoneSounderThread& self = *(ZoneSounderThread*)param;
		ZoneSounderThreadData* zoneSounderThreadData = new ZoneSounderThreadData(self.tau);

		self.locker.lock();
		self.threads.push_back(zoneSounderThreadData);
		self.locker.unlock();

		while (!self.isStop)
		{
			zoneSounderThreadData->Sleep();
			if (self.isStop)
				break;

			if (zoneSounderThreadData->zoneSounder != nullptr)
			{
				zoneSounderThreadData->SetFrameBuffer();
				zoneSounderThreadData->zoneSounder->Render();
				self.NextQueueItemProcess(zoneSounderThreadData);
			}
		}

		self.threadStopedCount++;
	}

	void ZoneSounderThread::NextQueueItemProcess(ZoneSounderThreadData* zoneSounderThreadData)
	{
		ZoneSounder* zoneSounder = nullptr;

		locker.lock();

		zoneSounderThreadData->zoneSounder->GetVirInstrument()->
			CombineZoneSounderSamples(zoneSounderThreadData->zoneSounder);

		processCount--;
		freeThreadIdx++;
		threads[freeThreadIdx] = zoneSounderThreadData;

		if (zoneSounderQueue.size() > 0)
		{
			zoneSounder = zoneSounderQueue.back();
			zoneSounderQueue.pop_back();
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

		zoneSounderThreadData->Wake(zoneSounder);
	}
}
