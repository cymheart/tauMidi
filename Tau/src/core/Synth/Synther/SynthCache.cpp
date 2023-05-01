#include"Synther.h"
#include"Synth/Tau.h"
#include"Synth/SyntherEvent.h"
#include"Synth/VirInstrument.h"
#include"Synth/Tau.h"
#include"Synth/Editor/Editor.h"
#include"Synth/Editor/MidiEditor.h"

namespace tau
{
	void Synther::ClearCacheBuffer()
	{
		if (cacheBuffer != nullptr)
			cacheBuffer->Clear();

		if (fallSamples.empty())
			return;

		vector<FadeSamplesInfo>::iterator it = fallSamples.begin();
		for (; it != fallSamples.end(); it++)
		{
			FadeSamplesInfo& info = *it;
			float* samples = info.samples;
			fallSamplesPool->Push(samples);
		}

		fallSamples.clear();
	}

	bool Synther::CanCache()
	{
		//仅当cacheState == play时，才可以缓存
		if (!isCacheWriteSoundEnd ||
			(midiEditor->GetState() == EditorState::PLAY))
			return true;

		isCacheWriteSoundEnd = true;
		return false;
	}

	void Synther::CacheOutput()
	{
		cacheLocker.lock();

		memset(synthSampleStream, 0, sizeof(float) * frameSampleCount * 2);

		if ((isSoundEndRemove || isReqDelete) && isSoundEnd) {
			cacheState = CacheState::CacheStop;
			ClearCacheBuffer();
			ReqRender();
			cacheLocker.unlock();
			return;
		}

		CacheReadByState();
		CacheReadFallSamples();

		if (tau->editor != nullptr)
			tau->editor->cacheState = cacheState;

		//进入步进模式后,合成器关闭缓存
		if (cacheState == CacheState::EnterStep)
			isEnableCache = false;

		cacheLocker.unlock();

	}

	void Synther::CacheReadByState()
	{
		if (isReqDelete)
		{
			ReqRender();
			cacheState = CacheState::Remove;
			return;
		}


		switch (cacheState)
		{
		case CacheState::CacheStop:
			if (!CanCache()) {
				ReqRender();
				break;
			}

			ClearCacheBuffer();
			dstCacheGain = cacheGain = 1;
			cacheState = CacheState::CachingNotRead;
			ReqRender();
			break;


		case CacheState::CachingAndRead:
			CacheRead();
			break;


		case CacheState::OnlyRead:

			if (cacheBuffer->GetNeedReadSize() <= reCacheSize) {
				cacheState = CacheState::CachingAndRead;
				ReqRender();
			}

			CacheRead();
			break;

		case CacheState::CacheReadTail:

			if (cacheBuffer->GetNeedReadSize() <= 0)
			{
				cacheBuffer->Clear();
				cacheState = CacheState::CacheStop;
				SetCachePlayState(EditorState::ENDPAUSE);
				break;
			}

			CacheRead();
			break;


		case CacheState::LeaveStep:
			SetCurtCachePlaySec(GetPlaySec());
			isEnableCache = true;
			cacheBuffer->Clear();
			cacheGain = dstCacheGain = 0;
			cacheState = CacheState::CachingNotRead;
			ReqRender();
			break;


		case CacheState::Removing:
			cacheState = CacheState::Remove;
			break;

		default:
			break;

		}
	}


	//合成渐隐的尾音样本到主发声通道中
	void Synther::CacheReadFallSamples()
	{
		if (fallSamples.empty())
			return;

		float* out = (float*)synthSampleStream;

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

					*out++ += samples[n] * gain;
					*out++ += samples[frameSampleCount + n] * gain;
				}
				break;

			case ChannelOutputMode::Mono:
				for (; n < info.pos + len; n++) {
					gain -= info.gainStep;
					if (gain < 0) { gain = 0; }
					*out++ += samples[n] * gain;
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

	void Synther::CacheInput()
	{
		cacheLocker.lock();
		if (isReqDelete)
		{
			cacheState = CacheState::Removing;
			cacheLocker.unlock();
			return;
		}

		CacheWrite();

		int64_t cachedSize = cacheBuffer->GetNeedReadSize();

		switch (cacheState)
		{
		case CacheState::CachingAndRead:

			if (!CanCache())
			{
				CreateRiseCacheGain();
				cacheState = CacheState::CacheReadTail;

			}
			else if (cachedSize <= minCacheSize)
			{
				CreateFallCacheSamples();
				cacheGain = 0;
				cacheState = CacheState::CachingNotRead;
				ReqRender();
			}
			else if (cachedSize <= maxCacheSize) {
				ReqRender();
			}
			else if (cachedSize > maxCacheSize)
				cacheState = CacheState::OnlyRead;
			break;


		case CacheState::CachingNotRead:
			if (!CanCache())
			{
				CreateRiseCacheGain();
				cacheState = CacheState::CacheReadTail;
			}
			else if (cachedSize <= reReadCacheSize) {
				ReqRender();
			}
			else if (cachedSize > reReadCacheSize) {
				cacheState = CacheState::CachingAndRead;
				CreateRiseCacheGain();
				ReqRender();
			}
			break;

		case CacheState::CachingPauseRead:
			if (!CanCache())
			{
				cacheState = CacheState::PauseWaitRead;
			}
			else if (cachedSize <= maxCacheSize)
				ReqRender();
			else
				cacheState = CacheState::PauseWaitRead;
			break;


		case CacheState::CacheStoping:
			isCacheWriteSoundEnd = true;
			cacheBuffer->Clear();
			cacheState = CacheState::CacheStop;
			break;

		default:
			break;
		}

		cacheLocker.unlock();
	}



	void Synther::CachePlay()
	{
		lock_guard<mutex> lock(cacheLocker);

		SetCachePlayState(EditorState::PLAY);

		switch (cacheState)
		{
		case CacheState::CacheStoping:
			cacheGain = 0;
			cacheState = CacheState::CachingNotRead;
			break;

		case CacheState::CachingPauseRead:
			cacheGain = 0;
			cacheState = CacheState::CachingNotRead;
			break;

		case CacheState::PauseWaitRead:
			cacheGain = 0;
			CreateRiseCacheGain();
			cacheState = CacheState::OnlyRead;
			break;

		default:
			break;
		}

	}

	void Synther::CachePause()
	{
		lock_guard<mutex> lock(cacheLocker);

		if (cachePlayState == EditorState::PAUSE)
			return;

		SetCachePlayState(EditorState::PAUSE);

		switch (cacheState)
		{
			//此时当前缓存未发声，可以直接暂停
		case CacheState::CacheStoping:
		case CacheState::CachingPauseRead:
			break;

			//此时当前缓存还在发声中，需要生成消隐样本处理(使得发音可以渐渐减弱直到消失)
		case CacheState::CachingAndRead:
		case CacheState::CachingNotRead:
			CreateFallCacheSamples();
			cacheState = CacheState::CachingPauseRead;
			break;


			//其它状态下将会转入到CachingPauseRead状态
		default:
			CreateFallCacheSamples();
			cacheState = CacheState::CachingPauseRead;
			ReqRender();
			break;
		}
	}

	void Synther::CacheStop(bool isReset)
	{
		lock_guard<mutex> lock(cacheLocker);

		if (cachePlayState != EditorState::PAUSE)
			CreateFallCacheSamples();

		SetCachePlayState(EditorState::STOP);
		SetCurtCachePlaySec(tau->editor->initStartPlaySec);

		for (int i = 0; i < virInstList.size(); i++)
			virInstList[i]->ClearSoundDatas();

		//移除需要删除的乐器
		if (isReset)
			RemoveNeedDeleteVirInsts(true);

		switch (cacheState)
		{
		case CacheState::CacheStoping:
			break;

		case CacheState::CachingAndRead:
		case CacheState::CachingNotRead:
		case CacheState::CachingPauseRead:
			cacheState = CacheState::CacheStoping;
			break;

		default:
			isCacheWriteSoundEnd = true;
			cacheBuffer->Clear();
			cacheState = CacheState::CacheStop;
			break;
		}

	}


	bool Synther::CacheGoto(double sec, bool isMustReset)
	{
		lock_guard<mutex> lock(cacheLocker);

		if (cacheState == CacheState::CacheStop)
		{
			SetCurtCachePlaySec(sec);
			return true;
		}

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

				int n = tau->frameSampleCount * sizeof(float) * channelCount;
				offsetCacheReadPosLen /= n;
				offsetCacheReadPosLen *= n;
			}
		}


		//
		bool isReset = false;
		SetCurtCachePlaySec(sec);

		//
#define _CacheClear() \
		if(offsetCacheReadPosLen < 0) {cacheBuffer->Clear();isReset = true;}\
		else {cacheBuffer->OffsetReadPos(offsetCacheReadPosLen);}\


		if (sec >= GetEndSec())
		{
			SetCachePlayState(EditorState::ENDPAUSE);
			switch (cacheState)
			{
			case CacheState::CacheStoping:
				break;

			case CacheState::CachingPauseRead:
			case CacheState::CachingAndRead:
			case CacheState::CachingNotRead:
				CreateFallCacheSamples();
				cacheState = CacheState::CacheStoping;
				break;

			default:
				CreateFallCacheSamples();
				isCacheWriteSoundEnd = true;
				cacheBuffer->Clear();
				cacheState = CacheState::CacheStop;
				break;
			}

			return false;
		}


		//
		switch (cacheState)
		{
		case CacheState::CachingAndRead:
		case CacheState::CacheStoping:
			CreateFallCacheSamples();
			_CacheClear();
			cacheGain = 0;
			cacheState = CacheState::CachingNotRead;
			break;

		case CacheState::CachingNotRead:
		case CacheState::CachingPauseRead:
			_CacheClear();
			break;

		case CacheState::PauseWaitRead:
			_CacheClear();
			cacheState = CacheState::CachingPauseRead;
			ReqRender();
			break;

		default:
			CreateFallCacheSamples();
			_CacheClear();
			cacheGain = 0;
			cacheState = CacheState::CachingNotRead;
			ReqRender();
			break;
		}

		return isReset;
	}


	//关闭了缓存，将进入通常播放模式
	void Synther::CacheEnterStepPlayMode()
	{
		lock_guard<mutex> lock(cacheLocker);

		isEnableCache = false;
		isStepPlayMode = true;
		cacheGain = dstCacheGain = 1;
		cacheGainStep = 0.0001;
		cacheState = CacheState::EnterStep;
	}

	//建立剩余缓存样本的消隐样本处理(使得发音可以渐渐减弱直到消失)
	void Synther::CreateFallCacheSamples()
	{
		int64_t size = cacheBuffer->GetNeedReadSize();  //获取缓存buffer中还剩余多少数据未读取
		int sampleSize = size / (sizeof(float) * channelCount);  //根据通道个数等计算总样本数量
		int limitSampleSize = 0.4 / tau->invSampleProcessRate;   //计算以0.4sec的消隐时间需要多少样本数量
		if (sampleSize > limitSampleSize)
			sampleSize = limitSampleSize;        //需要的样本数量

		int nFrame = sampleSize / frameSampleCount;   //计算总共有多少帧样本
		sampleSize = nFrame * frameSampleCount;  //往下取整样本数量
		if (sampleSize == 0)
			return;

		FadeSamplesInfo fadeSamplesInfo;
		fadeSamplesInfo.samples = fallSamplesPool->Pop();
		fadeSamplesInfo.gain = 1;
		fadeSamplesInfo.gainStep = 1.0 / sampleSize;
		fadeSamplesInfo.size = sampleSize * channelCount;

		//读取剩余的样本到渐隐缓存中
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

	//检测由发音是否完全结束
	void Synther::TestCacheWriteSoundEnd()
	{
		//检测由效果器带来的尾音是否结束
		int offset = (int)(frameSampleCount * 0.02f);
		for (int i = 0; i < frameSampleCount; i += offset)
		{
			// 此处值需要非常小，不然会产生杂音
			if (fabsf(leftChannelSamples[i]) > 0.0001f ||
				fabsf(rightChannelSamples[i]) > 0.0001f) {
				isCacheWriteSoundEnd = false;
			}
		}

		isCacheWriteSoundEnd = true;
	}


	void Synther::CacheRead()
	{
		cacheBuffer->ReadToDst(cacheReadLeftChannelSamples, frameSampleCount * sizeof(float));
		cacheBuffer->ReadToDst(cacheReadRightChannelSamples, frameSampleCount * sizeof(float));

		float* out = (float*)synthSampleStream;

		switch (tau->channelOutputMode)
		{
		case ChannelOutputMode::Stereo:
			for (int n = 0; n < frameSampleCount; n++) {
				CacheGainFade();
				*out++ = cacheReadLeftChannelSamples[n] * cacheGain;
				*out++ = cacheReadRightChannelSamples[n] * cacheGain;
			}
			break;

		case ChannelOutputMode::Mono:
			for (int n = 0; n < frameSampleCount; n++) {
				CacheGainFade();
				*out++ = cacheReadLeftChannelSamples[n] * cacheGain;
			}
			break;
		}

		//
		SetCurtCachePlaySec(curtCachePlaySec + frameSec);
	}

	//设置当前缓存播放时间
	void Synther::SetCurtCachePlaySec(double sec)
	{
		curtCachePlaySec = sec;
		if (maxCacheSize > 0 && isEnableCache && tau->editor)
			tau->editor->curtPlaySec = curtCachePlaySec;
	}

	//设置缓存的播放状态
	void Synther::SetCachePlayState(EditorState s)
	{
		cachePlayState = s;
		if (maxCacheSize > 0 && isEnableCache && tau->editor)
			tau->editor->playState = cachePlayState;
	}


	//设置缓存的缓存状态
	void Synther::SetCacheState(CacheState cs)
	{
		cacheState = cs;
		if (maxCacheSize > 0 && isEnableCache && tau->editor)
			tau->editor->cacheState = cs;
	}

	void Synther::ShowCacheInfo()
	{
		if (cacheBuffer == nullptr) {
			PostTask(_ShowCacheInfoTask, this, 100);
			return;
		}

		printf("Cache Size:%d  byte\n", (int)(cacheBuffer->GetNeedReadSize()));
		printf("Play Time:%.2f sec\n", (float)curtCachePlaySec);

		PostTask(_ShowCacheInfoTask, this, 100);
	}

	void Synther::_ShowCacheInfoTask(Task* task)
	{
		Synther* synther = (Synther*)task->data;
		synther->ShowCacheInfo();
	}

}