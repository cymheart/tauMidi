#include"Synther.h"
#include"Synth/Tau.h"
#include"Synth/SyntherEvent.h"
#include"Synth//VirInstrument.h"
#include"Synth/Tau.h"

namespace tau
{

	//缓存处理
	void Synther::CacheProcess()
	{
		cacheLocker.lock();

		int stepModeCount = 0;
		for (int i = 0; i < slaveSynthers.size(); i++) {

			slaveSynthers[i]->CacheReadFallSamples(this);
			slaveSynthers[i]->CacheReadToMain(this);

			if (slaveSynthers[i]->state == CacheState::EnterSteped)
				stepModeCount++;
		}

		if (!slaveSynthers.empty()) {
			cachePlayState = slaveSynthers[0]->cachePlayState;
			curtCachePlaySec = slaveSynthers[0]->curtCachePlaySec;
		}

		if (stepModeCount != 0 &&
			stepModeCount == slaveSynthers.size())
		{
			isEnableCache = false;
		}

		cacheLocker.unlock();
	}

	void Synther::CacheRender()
	{
		mainSynther->cacheLocker.lock();

		if (isReqDelete)
		{
			state = CacheState::Removing;
			mainSynther->cacheLocker.unlock();
			return;
		}

		CacheWrite();

		int64_t cachedSize = cacheBuffer->GetNeedReadSize();

		switch (state)
		{
		case CacheState::CachingAndRead:

			if (!CanCache())
			{
				CreateRiseCacheGain();
				state = CacheState::CacheReadTail;

			}
			else if (cachedSize <= minCacheSize)
			{
				CreateFallCacheSamples();
				state = CacheState::CachingNotRead;
			}
			else if (cachedSize <= maxCacheSize) {
				ReqRender();
			}
			else
				state = CacheState::OnlyRead;
			break;


		case CacheState::CachingNotRead:
			if (!CanCache())
			{
				CreateRiseCacheGain();
				state = CacheState::CacheReadTail;
			}
			else if (cachedSize <= reReadCacheSize) {
				ReqRender();
			}
			else {
				state = CacheState::CachingAndRead;
				CreateRiseCacheGain();
				ReqRender();
			}
			break;

		case CacheState::CachingPauseRead:
			if (cachedSize <= maxCacheSize)
				ReqRender();
			else
				state = CacheState::PauseWaitRead;
			break;


		case CacheState::CacheStoping:
			cacheBuffer->Clear();
			state = CacheState::CacheStop;
			break;
		}


		mainSynther->cacheLocker.unlock();
	}

	void Synther::CacheReadToMain(Synther* mainSynther)
	{
		if (isReqDelete)
		{
			if (state != CacheState::CacheStoping &&
				state != CacheState::CachingAndRead &&
				state != CacheState::CachingNotRead &&
				state != CacheState::CacheStoping)
			{
				state = CacheState::Remove;
			}
		}

		switch (state)
		{
		case CacheState::CacheStop:
			if (!CanCache()) {
				isSoundEnd = true;
				break;
			}

			dstCacheGain = cacheGain = 1;
			state = CacheState::CachingNotRead;
			ReqRender();
			break;


		case CacheState::CachingAndRead:
			CacheRead(mainSynther);
			break;


		case CacheState::OnlyRead:

			if (cacheBuffer->GetNeedReadSize() <= reCacheSize) {
				state = CacheState::CachingAndRead;
				ReqRender();
			}

			CacheRead(mainSynther);
			break;

		case CacheState::CacheReadTail:

			if (cacheBuffer->GetNeedReadSize() <= 0)
			{
				state = CacheState::CacheStop;
				cachePlayState = EditorState::STOP;
				break;
			}

			CacheRead(mainSynther);
			break;


		case CacheState::LeaveStep:
			curtCachePlaySec = GetPlaySec();
			isEnableCache = true;
			cacheBuffer->Clear();
			cacheGain = dstCacheGain = 0;
			state = CacheState::CachingNotRead;
			ReqRender();

			//
			if (cmdWait)
				cmdWait->set();
			break;


		case CacheState::Removing:
			state = CacheState::Remove;
			break;
		}

	}


	void Synther::CachePlay()
	{
		lock_guard<mutex> lock(mainSynther->cacheLocker);

		cachePlayState = EditorState::PLAY;

		switch (state)
		{
		case CacheState::CacheStoping:
			cacheGain = 0;
			state = CacheState::CachingNotRead;
			break;

		case CacheState::CachingPauseRead:
			cacheGain = 0;
			state = CacheState::CachingNotRead;
			break;

		case CacheState::PauseWaitRead:
			cacheGain = 0;
			CreateRiseCacheGain();
			state = CacheState::OnlyRead;
			break;
		}

	}

	void Synther::CachePause()
	{
		lock_guard<mutex> lock(mainSynther->cacheLocker);

		cachePlayState = EditorState::PAUSE;

		switch (state)
		{
		case CacheState::CacheStoping:
		case CacheState::CachingPauseRead:
			break;

		case CacheState::CachingAndRead:
		case CacheState::CachingNotRead:
			CreateFallCacheSamples();
			state = CacheState::CachingPauseRead;
			break;

		default:
			CreateFallCacheSamples();
			state = CacheState::CachingPauseRead;
			ReqRender();
			break;
		}
	}

	void Synther::CacheStop(bool isReset)
	{
		lock_guard<mutex> lock(mainSynther->cacheLocker);

		if (cachePlayState != EditorState::PAUSE)
		{
			CreateFallCacheSamples();
		}

		cachePlayState = EditorState::STOP;
		curtCachePlaySec = 0;

		for (int i = 0; i < virInstList.size(); i++)
			virInstList[i]->ClearSoundDatas();

		//移除需要删除的乐器
		if (isReset)
			RemoveNeedDeleteVirInsts(true);

		switch (state)
		{
		case CacheState::CacheStoping:
			break;

		case CacheState::CachingAndRead:
		case CacheState::CachingNotRead:
		case CacheState::CachingPauseRead:
			state = CacheState::CacheStoping;
			break;

		default:
			cacheBuffer->Clear();
			state = CacheState::CacheStop;
			break;
		}

	}


	bool Synther::CacheGoto(double sec, bool isMustReset)
	{
		lock_guard<mutex> lock(mainSynther->cacheLocker);

		if (state == CacheState::CacheStop)
			return false;

		int offsetCacheReadPosLen = -1;
		if (!isMustReset && sec >= curtCachePlaySec)
		{
			int64_t size = cacheBuffer->GetNeedReadSize() - minCacheSize;
			int m = size / (sizeof(float) * channelCount);
			float cacheEndSec = curtCachePlaySec + m * tau->invSampleProcessRate;
			if (sec <= cacheEndSec)
			{
				float diffSec = sec - curtCachePlaySec;
				offsetCacheReadPosLen = diffSec * tau->sampleProcessRate * sizeof(float) * channelCount;
				int n = tau->frameSampleCount * 2 * sizeof(float) * channelCount;
				offsetCacheReadPosLen /= n;
				offsetCacheReadPosLen *= n;
			}
		}

		//
		bool isReset = false;
		curtCachePlaySec = sec;

		//
#define _CacheClear() \
		if(offsetCacheReadPosLen < 0) {cacheBuffer->Clear();isReset = true;}\
		else {cacheBuffer->OffsetReadPos(offsetCacheReadPosLen);}\

		//
		switch (state)
		{
		case CacheState::CachingAndRead:
		case CacheState::CacheStoping:
			CreateFallCacheSamples();
			_CacheClear();
			cacheGain = 0;
			state = CacheState::CachingNotRead;
			break;

		case CacheState::CachingNotRead:
		case CacheState::CachingPauseRead:
			_CacheClear();
			break;

		case CacheState::PauseWaitRead:
			_CacheClear();
			state = CacheState::CachingPauseRead;
			ReqRender();
			break;

		default:
			CreateFallCacheSamples();
			_CacheClear();
			cacheGain = 0;
			state = CacheState::CachingNotRead;
			ReqRender();
			break;
		}

		return isReset;
	}


	void Synther::CacheEnterStepPlayMode()
	{
		lock_guard<mutex> lock(mainSynther->cacheLocker);
		isEnableCache = false;
		isStepPlayMode = true;
		cacheGain = dstCacheGain = 1;
		cacheGainStep = 0.0001;
		state = CacheState::EnterSteped;
	}

	void Synther::CreateFallCacheSamples()
	{
		int64_t size = cacheBuffer->GetNeedReadSize();
		int sampleSize = size / (sizeof(float) * channelCount);
		int limitSize = 0.4 / tau->invSampleProcessRate;
		if (sampleSize > limitSize)
			sampleSize = limitSize;

		int n = sampleSize / frameSampleCount;
		sampleSize = n * frameSampleCount;
		if (sampleSize == 0)
			return;

		FadeSamplesInfo fadeSamplesInfo;
		fadeSamplesInfo.samples = fallSamplesPool->Pop();
		fadeSamplesInfo.gain = 1;
		fadeSamplesInfo.gainStep = 1.0 / sampleSize;
		fadeSamplesInfo.size = sampleSize * channelCount;

		cacheBuffer->PeekToDst(fadeSamplesInfo.samples, sampleSize * sizeof(float) * channelCount);
		fallSamples.push_back(fadeSamplesInfo);
	}


	//生成渐升的CacheGain
	void Synther::CreateRiseCacheGain()
	{
		int sampleSize = 0.2 / tau->invSampleProcessRate;
		cacheGainStep = 1.0 / sampleSize;
		dstCacheGain = 1.0;
	}

	void Synther::CacheWrite()
	{
		switch (tau->channelOutputMode)
		{
		case ChannelOutputMode::Stereo:
		{
			cacheBuffer->Write(leftChannelSamples, frameSampleCount * sizeof(float));
			cacheBuffer->Write(rightChannelSamples, frameSampleCount * sizeof(float));
		}
		break;

		case ChannelOutputMode::Mono:
			cacheBuffer->Write(leftChannelSamples, frameSampleCount * sizeof(float));
			break;
		}

		TestCacheWriteSoundEnd();
	}

	void Synther::CacheRead()
	{
		cacheBuffer->ReadToDst(cacheReadLeftChannelSamples, frameSampleCount * sizeof(float));
		cacheBuffer->ReadToDst(cacheReadRightChannelSamples, frameSampleCount * sizeof(float));
		TestCacheSoundEnd();
	}

	//检测由发音是否完全结束
	void Synther::TestCacheSoundEnd()
	{
		isSoundEnd = true;
		//检测由效果器带来的尾音是否结束
		int offset = (int)(frameSampleCount * 0.02f);
		for (int i = 0; i < frameSampleCount; i += offset)
		{
			// 此处值需要非常小，不然会产生杂音
			if (fabsf(cacheReadLeftChannelSamples[i]) > 0.0001f ||
				fabsf(cacheReadRightChannelSamples[i]) > 0.0001f) {
				isSoundEnd = false;
				break;
			}
		}
	}

	void Synther::CacheRead(Synther* mainSynther)
	{
		CacheRead();

		switch (tau->channelOutputMode)
		{
		case ChannelOutputMode::Stereo:
			for (int n = 0; n < frameSampleCount; n++) {
				CacheGainFade();
				mainSynther->leftChannelSamples[n] += GetLeftCacheChannelSamples(n);
				mainSynther->rightChannelSamples[n] += GetRightCacheChannelSamples(n);
			}
			break;

		case ChannelOutputMode::Mono:
			for (int n = 0; n < frameSampleCount; n++) {
				CacheGainFade();
				mainSynther->leftChannelSamples[n] += GetLeftCacheChannelSamples(n);
			}
			break;
		}

		//
		curtCachePlaySec += frameSec;
	}


	void Synther::CacheReadFallSamples(Synther* mainSynther)
	{
		if (fallSamples.empty())
			return;

		vector<FadeSamplesInfo>::iterator it = fallSamples.begin();
		for (; it != fallSamples.end(); )
		{
			FadeSamplesInfo& info = *it;
			float* samples = info.samples;
			double gain = info.gain;
			int n = info.pos;

			int len = min((info.size - info.pos) / channelCount, frameSampleCount);

			switch (tau->channelOutputMode)
			{
			case ChannelOutputMode::Stereo:
				for (; n < info.pos + len; n++) {
					gain -= info.gainStep;
					if (gain < 0) { gain = 0; }
					mainSynther->leftChannelSamples[n - info.pos] += samples[n] * gain;
					mainSynther->rightChannelSamples[n - info.pos] += samples[frameSampleCount + n] * gain;
				}
				break;

			case ChannelOutputMode::Mono:
				for (; n < info.pos + len; n++) {
					gain -= info.gainStep;
					if (gain < 0) { gain = 0; }
					mainSynther->leftChannelSamples[n - info.pos] += samples[n] * gain;
				}
				break;
			}


			if (gain <= 0 || len == 0)
			{
				fallSamplesPool->Push(samples);
				it = fallSamples.erase(it);
			}
			else
			{
				info.gain = gain;
				info.pos = n + len;
				it++;
			}
		}
	}

	void Synther::CacheSynthSlavesBuffer()
	{
		int n = 0;
		double s;
		curtCachePlaySec = 0;
		for (int i = 0; i < slaveSynthers.size(); i++)
		{
			s = slaveSynthers[i]->GetPlaySec();
			if (s > curtCachePlaySec)
				curtCachePlaySec = s;

			slaveSynthers[i]->CacheReadFallSamples(this);
			slaveSynthers[i]->CacheSynthToMain(this);
			if (slaveSynthers[i]->cacheGain == 0)
				n++;
		}

		if (n == slaveSynthers.size())
			isEnableCache = true;
	}


	void Synther::CacheSynthToMain(Synther* mainSynther)
	{
		if (maxCacheSize > 0 && !isStepPlayMode)
		{
			cacheGain -= 0.1;
			if (cacheGain < 0) {
				cacheGain = 0;
				state = CacheState::LeaveStep;
			}
		}

		switch (tau->channelOutputMode)
		{
		case ChannelOutputMode::Stereo:
			for (int n = 0; n < frameSampleCount; n++) {
				mainSynther->leftChannelSamples[n] += leftChannelSamples[n] * cacheGain;
				mainSynther->rightChannelSamples[n] += rightChannelSamples[n] * cacheGain;
			}
			break;

		case ChannelOutputMode::Mono:
			for (int n = 0; n < frameSampleCount; n++) {
				mainSynther->leftChannelSamples[n] += leftChannelSamples[n] * cacheGain;
			}
			break;
		}
	}


	void Synther::ShowCacheInfo()
	{
		if (!isMainSynther)
			return;

		if (slaveSynthers.size() == 0 || slaveSynthers[0]->cacheBuffer == nullptr) {
			PostTask(_ShowCacheInfoTask, this, 100);
			return;
		}

		printf("Cache Size:%d  byte\n", (int)(slaveSynthers[0]->cacheBuffer->GetNeedReadSize()));
		printf("Play Time:%.2f sec\n", (float)curtCachePlaySec);

		PostTask(_ShowCacheInfoTask, this, 100);
	}

	void Synther::_ShowCacheInfoTask(Task* task)
	{
		Synther* synther = (Synther*)task->data;
		synther->ShowCacheInfo();
	}

}