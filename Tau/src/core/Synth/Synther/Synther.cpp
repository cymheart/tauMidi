#include"Synther.h"
#include"Synth/Tau.h"
#include"Synth/VirInstrument.h"
#include"Synth/ZoneSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"
#include"FX/Compressor.h"
#include"Audio/AudioPa/Audio_pa.h"
#include"Audio/AudioOboe/Audio_oboe.h"
#include"Audio/AudioSDL/Audio_SDL.h"
#include"Audio/AudioRt/Audio_Rt.h"
#include"kissfft/kiss_fft.h"
#include"Synth/Tau.h"
#include"Synth/Editor/Editor.h"
#include"Synth/Editor/MidiEditor.h"
#include"Synth/Editor/Track.h"
#include <iostream>
#include <fstream>
#include"task\TaskTimer.h"


using namespace std;

namespace tau
{
	Synther::Synther(Tau* tau)
	{
		this->tau = tau;
		isFrameRenderCompleted = true;
		isReqDelete = false;

		cachePlayState = EditorState::STOP;
		SetCurtCachePlaySec(0);

		//
		cacheTaskProcesser = new TaskProcesser;
		taskProcesser = new TaskProcesser;
		ZoneSounderThreadPool = new ZoneSounderThread(this);

		//
		effects = new EffectList();
		innerEffects = new EffectList();

	}


	Synther::~Synther()
	{
		Close();

		//
		DEL(cacheTaskProcesser);
		DEL(taskProcesser);
		DEL(ZoneSounderThreadPool);
		DEL(effects);
		DEL(innerEffects);
		
		presetBankReplaceMap.clear();


		DEL(midiEditor);
	}

	void Synther::Lock()
	{
		if (!IsCacheEnable())
			tau->lockMutex.lock();
		else
			cacheLocker.lock();
	}

	void Synther::UnLock()
	{
		if (!IsCacheEnable())
			tau->lockMutex.unlock();
		else
			cacheLocker.unlock();
	}


	void Synther::Open()
	{
		if (isOpened)
			return;

		isFrameRenderCompleted = true;

		channelCount = 1;
		if (tau->channelOutputMode == ChannelOutputMode::Stereo)
			channelCount = 2;


		leftChannelSamples = new float[tau->frameSampleCount];
		rightChannelSamples = new float[tau->frameSampleCount];

		cacheReadLeftChannelSamples = new float[tau->frameSampleCount];
		cacheReadRightChannelSamples = new float[tau->frameSampleCount];

		//

		frameSec = tau->frameSampleCount * tau->invSampleProcessRate;
		int baseSize = sizeof(float) * tau->frameSampleCount * channelCount;
		synthStreamBufferSize = baseSize * 20;
		synthSampleStream = new uint8_t[synthStreamBufferSize];

		if (tau->sampleStreamCacheSec > 0)
		{
			isEnableCache = true;

			maxCacheSize = sizeof(float) * tau->sampleProcessRate * channelCount * tau->sampleStreamCacheSec;
			if (maxCacheSize < baseSize * 4)
				maxCacheSize = baseSize * 4;

			cacheBuffer = new RingBuffer(maxCacheSize + synthStreamBufferSize);

			minCacheSize = maxCacheSize / 10;
			reReadCacheSize = maxCacheSize / 3;
			reCacheSize = maxCacheSize / 2;


			//
			int sz = 3 * tau->sampleProcessRate * channelCount;
			sz = sz / tau->frameSampleCount + 2;
			sz *= tau->frameSampleCount;
			fallSamplesPool = new ArrayPool<float>(4, sz);
		}


		//
		effects->Set((float*)synthSampleStream, channelCount, frameSampleCount);
		innerEffects->Set(leftChannelSamples, rightChannelSamples, frameSampleCount);

		//
		tau->editor->ResetParams();
		curtCachePlaySec = tau->editor->GetInitStartPlaySec();
		ZoneSounderThreadPool->Start();
		taskProcesser->Start();

		if (IsCacheEnable()) {
			cacheTaskProcesser->Start();
		}

		OpenAudio();
		audio->Open();

		isOpened = true;
		//ShowCacheInfo();
	}


	void Synther::Close()
	{
		if (!isOpened)
			return;

		tau->lockMutex.lock();
		isReqDelete = true;
		tau->lockMutex.unlock();

		waitSem.wait();

		//
		Audio* a = audio;
		audio = nullptr;
		DEL(a);

		ZoneSounderThreadPool->Stop();
		taskProcesser->Stop();
		cacheTaskProcesser->Stop();
		isOpened = false;

		for (int i = 0; i < virInstList.size(); i++)
			DEL(virInstList[i]);

		virInstList.clear();
		virInsts.clear();
		virInstSet.clear();
		needDeleteVirInsts.clear();

		//
		maxCacheSize = 0;
		minCacheSize = 0;
		reReadCacheSize = 0;
		reCacheSize = 0;

		isReqDelete = false;


		//
		DEL_ARRAY(leftChannelSamples);
		DEL_ARRAY(rightChannelSamples);
		DEL_ARRAY(cacheReadLeftChannelSamples);
		DEL_ARRAY(cacheReadRightChannelSamples);

		DEL(synthSampleStream);
		DEL(cacheBuffer);
		DEL(fallSamplesPool);
	}


	float Synther::GetSampleProcessRate()
	{
		if (!tau)
			return 44100;

		return tau->GetSampleProcessRate();
	}


	// 开启声音播放引擎
	void Synther::OpenAudio()
	{
		if (audio != nullptr)
			return;

#ifdef _WIN32
		if (tau->audioEngineType == Audio::EngineType::SDL) {
			audio = new Audio_SDL();
		}
		else if (tau->audioEngineType == Audio::EngineType::RtAudio) {
			audio = new Audio_Rt();
			//添加压缩器效果
			tauFX::Compressor* compressor = new tauFX::Compressor();
			AddEffect(compressor);
		}
		else
		{
			audio = new Audio_pa();
			//添加压缩器效果
			tauFX::Compressor* compressor = new tauFX::Compressor();
			AddEffect(compressor);
		}
#else
		audio = new Audio_oboe();
#endif

		audio->SetChannelCount((int)tau->channelOutputMode);
		audio->SetSampleCount(tau->frameSampleCount);
		audio->SetAudioCallback(FillAudioSample, this);

		audio->SetFreq((int)tau->sampleProcessRate);
	}

	void Synther::FillAudioSample(void* udata, uint8_t* stream, int len)
	{
		((Synther*)udata)->FrameRender(stream, len);
	}


	//设置帧样本数量
	//这个值越小，声音的实时性越高（在实时演奏时，值最好在1024以下，最合适的值为512）,
	//当这个值比较小时，cpu内耗增加
	void Synther::SetFrameSampleCount(int count)
	{
		frameSampleCount = count;
		if (frameSampleCount < 256) frameSampleCount = 256;

		effects->Set((float*)synthSampleStream, channelCount, frameSampleCount);
		innerEffects->Set(leftChannelSamples, rightChannelSamples, frameSampleCount);
	}

	//设置是否使用多线程
	//使用多线程渲染处理声音
	//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
	//当childFrameSampleCount >= 256时，多线程效率会比较高
	//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
	//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
	void Synther::SetUseMulThread(bool use)
	{
		useMulThreads = use;
		if (use)
		{
			ZoneSounderThreadPool->Start();
		}
		else
		{
			ZoneSounderThreadPool->Stop();
		}
	}

	//投递任务
	void Synther::PostTask(TaskCallBack taskCallBack, void* data, int delay)
	{
		Task* task = taskProcesser->CreateTask();
		task->processCallBack = taskCallBack;
		task->data = data;
		taskProcesser->PostTask(task, delay);
	}

	//投递任务
	void Synther::PostTask(Task* task, int delay)
	{
		taskProcesser->PostTask(task, delay);
	}

	// 帧渲染
	void Synther::FrameRender(uint8_t* stream, int len)
	{
		//printf("----------------\n");
		//printf("Enter FrameAudio: %ld\n", GetCurrentTimeMsec());
		//
		while (audio)
		{
			// autio系统调用此函数时，并没有按固定的帧率调用
			// render和autio通过isFrameRenderCompleted来通知协调
			if (isFrameRenderCompleted)
			{
				//printf("FrameAudio: %ld, len: %d\n", GetCurrentTimeMsec(), len);

				if (!tau->isSilence)
					memcpy(stream, synthSampleStream, len);

				if (!IsCacheEnable())
					ReqRender();
				else
					ReqCacheRender();
				break;
			}


#ifdef _WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
	}



	// 请求缓存渲染事件
	void Synther::ReqCacheRender()
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = CacheRenderTask;

		if (IsCacheEnable())
			isFrameRenderCompleted = false;

		cacheTaskProcesser->PostTask(ev);
	}

	void Synther::CacheRenderTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.CacheRender();
	}


	// 请求帧渲染事件
	void Synther::ReqRender()
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = RenderTask;

		if (!IsCacheEnable())
			isFrameRenderCompleted = false;

		taskProcesser->PostTask(ev);
	}

	void Synther::RenderTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);

		//printf("Render: %ld\n", GetCurrentTimeMsec());

		synther.tau->lockMutex.lock();
		synther.Render();
		if (!synther.IsCacheEnable())
			synther.isFrameRenderCompleted = true;
		synther.tau->lockMutex.unlock();
	}


	//是否包含指定的虚拟乐器
	bool Synther::IsHavVirInstrument(VirInstrument* virInst)
	{
		if (virInst == nullptr)
			return false;

		if (virInstSet.find(virInst) != virInstSet.end())
			return true;

		return false;
	}

	//添加替换乐器
	void Synther::AppendReplaceInstrument(
		int orgBankMSB, int orgBankLSB, int orgInstNum,
		int repBankMSB, int repBankLSB, int repInstNum)
	{
		int orgKey = orgBankMSB << 16 | orgBankLSB << 8 | orgInstNum;
		int repKey = repBankMSB << 16 | repBankLSB << 8 | repInstNum;
		presetBankReplaceMap[orgKey] = repKey;
	}

	//移除替换乐器
	void Synther::RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum)
	{
		int orgKey = orgBankMSB << 16 | orgBankLSB << 8 | orgInstNum;
		presetBankReplaceMap.erase(orgKey);
	}

	//获取替换乐器key
	int Synther::GetReplaceInstrumentKey(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		int key = bankSelectMSB << 16 | bankSelectLSB << 8 | instrumentNum;
		auto itReplace = presetBankReplaceMap.find(key);
		if (itReplace != presetBankReplaceMap.end()) {
			key = itReplace->second;
		}

		return key;
	}

	//增加效果器
	void Synther::AddEffect(TauEffect* effect)
	{
		effect->SetTau(tau);
		effects->AppendEffect(effect);
	}

	//调制虚拟乐器参数
	void Synther::ModulationVirInstParams(Channel* channel)
	{
		vector<VirInstrument*>& vinsts = channel->GetVirInstruments();
		for (int i = 0; i < vinsts.size(); i++)
			vinsts[i]->Modulation();
	}

	//调制虚拟乐器参数
	void Synther::ModulationVirInstParams(VirInstrument* virInst)
	{
		virInst->Modulation();
	}


	//设置是否开启所有乐器效果器
	void Synther::SetEnableAllVirInstEffects(bool isEnable)
	{
		isEnableVirInstEffects = isEnable;
		VirInstrument* virInst = nullptr;
		for (int i = 0; i < virInstList.size(); i++)
		{
			virInst = virInstList[i];
			virInst->SetEnableEffects(isEnable);
		}
	}

	//设置乐器bend
	void Synther::SetVirInstrumentPitchBend(VirInstrument* virInst, int bend)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->SetPitchBend(bend);
	}

	//设置乐器按键压力
	void Synther::SetVirInstrumentPolyPressure(VirInstrument* virInst, int key, int pressure)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->SetPolyPressure(key, pressure);
	}


	//设置乐器midicontroller
	void Synther::SetVirInstrumentMidiControllerValue(VirInstrument* virInst, MidiControllerType midiController, int value)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->SetController(midiController, value);
	}

	// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在，将在虚拟乐器列表中自动创建它
	// 注意如果channel已经被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
	// 而不会同时在一个通道上创建超过1个的虚拟乐器
	VirInstrument* Synther::EnableVirInstrument(Channel* channel, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);

		if (virInsts == nullptr)
		{
			VirInstrument* virInst = new VirInstrument(this, channel, nullptr);
			virInst->ChangeProgram(bankSelectMSB, bankSelectLSB, instrumentNum);
			virInst->Open(true);
			AppendToVirInstList(virInst);
			return virInst;
		}

		(*virInsts)[0]->ChangeProgram(bankSelectMSB, bankSelectLSB, instrumentNum);
		for (int i = 1; i < virInsts->size(); i++)
			RemoveVirInstrument((*virInsts)[i], true);

		return (*virInsts)[0];

	}


	// 设置虚拟乐器值
	void Synther::SetVirInstrumentProgram(VirInstrument* virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->ChangeProgram(bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	//移除所有虚拟乐器
	void Synther::RemoveAllVirInstrument(bool isFade)
	{
		for (int i = 0; i < virInstList.size(); i++)
		{
			virInstList[i]->Remove(isFade);
		}
	}

	//删除所有虚拟乐器
	void Synther::DelAllVirInstrument(int type)
	{
		for (int i = 0; i < virInstList.size(); i++)
		{
			if (virInstList[i]->IsRemove()) {
				virInstList[i]->state = VirInstrumentState::REMOVED;
				continue;
			}

			Channel* channel = virInstList[i]->GetChannel();
			bool isDeviceChannel = false;
			if (channel != nullptr)
				channel->IsDeviceChannel();

			if (type == 0 && !isDeviceChannel) {
				AddNeedDelVirInstrument(virInstList[i]);
				virInstList[i]->state = VirInstrumentState::REMOVED;
			}
			else if (type == 1 && isDeviceChannel) {
				AddNeedDelVirInstrument(virInstList[i]);
				virInstList[i]->state = VirInstrumentState::REMOVED;
			}
			else if (type == -1) {
				AddNeedDelVirInstrument(virInstList[i]);
				virInstList[i]->state = VirInstrumentState::REMOVED;
			}
		}

		RemoveNeedDeleteVirInsts(true);
	}

	//停止所有虚拟乐器
	void Synther::StopAllVirInstrument(int type)
	{
		for (int i = 0; i < virInstList.size(); i++)
		{
			bool isDeviceChannel = virInstList[i]->GetChannel()->IsDeviceChannel();

			if (type == 0 && !isDeviceChannel) {
				virInstList[i]->state = VirInstrumentState::OFFED;
				virInstList[i]->gain = 0;
			}
			else if (type == 1 && isDeviceChannel) {
				virInstList[i]->state = VirInstrumentState::OFFED;
				virInstList[i]->gain = 0;
			}
			else if (type == -1) {
				virInstList[i]->state = VirInstrumentState::OFFED;
				virInstList[i]->gain = 0;
			}
		}
	}


	//根据通道移除虚拟乐器
	void Synther::RemoveVirInstrument(Channel* channel, bool isFade)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);
		if (virInsts == nullptr)
			return;

		for (int i = 0; i < virInsts->size(); i++)
			RemoveVirInstrument((*virInsts)[i], isFade);
	}

	//移除虚拟乐器
	void Synther::RemoveVirInstrument(VirInstrument* virInst, bool isFade)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->Remove(isFade);
	}


	void Synther::AddNeedDelVirInstrument(VirInstrument* virInst)
	{
		needDeleteVirInsts.push_back(virInst);
	}


	//打开虚拟乐器
	void Synther::OpenVirInstrument(VirInstrument* virInst, bool isFade)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->Open(isFade);
	}

	//根据通道打开相关虚拟乐器
	void Synther::OpenVirInstrument(Channel* channel, bool isFade)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);
		if (virInsts == nullptr)
			return;

		for (int i = 0; i < virInsts->size(); i++)
			OpenVirInstrument((*virInsts)[i], isFade);
	}


	//关闭虚拟乐器
	void Synther::CloseVirInstrument(VirInstrument* virInst, bool isFade)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->Close(isFade);
	}

	//根据通道关闭相关虚拟乐器
	void Synther::CloseVirInstrument(Channel* channel, bool isFade)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);
		if (virInsts == nullptr)
			return;

		for (int i = 0; i < virInsts->size(); i++)
			CloseVirInstrument((*virInsts)[i], isFade);
	}

	//按下按键
	void Synther::OnKey(int key, float velocity, VirInstrument* virInst, int id)
	{
		virInst->OnKey(key, velocity, -1, id);
	}


	// 按下按键
	void Synther::OnKey(int key, float velocity, Track* track, int id)
	{
		OnKey(track->GetChannel(), key, velocity, -1, id);
	}

	//根据通道按下相关虚拟乐器指定按键
	void Synther::OnKey(Channel* channel, int key, float velocity, int tickCount, int id)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);
		if (virInsts == nullptr)
			return;

		for (int i = 0; i < virInsts->size(); i++)
			(*virInsts)[i]->OnKey(key, velocity, tickCount, id);
	}

	// 释放按键 
	void Synther::OffKey(int key, float velocity, Track* track, int id)
	{
		OffKey(track->GetChannel(), key, velocity, id);
	}

	//根据通道松开相关虚拟乐器指定按键
	void Synther::OffKey(Channel* channel, int key, float velocity, int id)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);
		if (virInsts == nullptr)
			return;

		for (int i = 0; i < virInsts->size(); i++)
			(*virInsts)[i]->OffKey(key, velocity, id);
	}

	// 释放按键
	void Synther::OffKey(int key, float velocity, VirInstrument* virInst, int id)
	{
		virInst->OffKey(key, velocity, id);
	}


	//根据通道关闭相关虚拟乐器所有按键
	void Synther::OffAllKeys(Channel* channel)
	{
		vector<VirInstrument*>* virInsts = GetVirInstruments(channel);
		if (virInsts == nullptr)
			return;

		for (int i = 0; i < virInsts->size(); i++)
			(*virInsts)[i]->OffAllKeys();
	}

	//关闭所有虚拟乐器所有按键
	void Synther::OffAllKeys()
	{
		VirInstrument* virInst;
		vector<VirInstrument*>::iterator it = virInstList.begin();
		for (; it != virInstList.end(); it++)
		{
			virInst = *it;
			virInst->OffAllKeys();
		}
	}

	//关闭与指定id匹配的所有虚拟乐器所有按键
	void Synther::OffAllKeys(int id)
	{
		VirInstrument* virInst;
		vector<VirInstrument*>::iterator it = virInstList.begin();
		for (; it != virInstList.end(); it++)
		{
			virInst = *it;
			virInst->OffAllKeys(id);
		}
	}


	//根据指定通道获取关连虚拟乐器
	vector<VirInstrument*>* Synther::GetVirInstruments(Channel* channel)
	{
		if (channel == nullptr)
			return nullptr;

		vector<VirInstrument*>& vinsts = channel->GetVirInstruments();
		if (vinsts.empty() || vinsts[0]->IsRemove())
			return nullptr;

		return &vinsts;
	}

	//添加虚拟乐器到表
	void Synther::AppendToVirInstList(VirInstrument* virInst)
	{
		virInstList.push_back(virInst);
		virInsts.push_back(virInst);
		virInstSet.insert(virInst);
	}

	// 从乐器表移除虚拟乐器
	void Synther::RemoveVirInstFromList(VirInstrument* virInst)
	{
		if (virInst == nullptr)
			return;

		vector<VirInstrument*>::iterator it = virInstList.begin();
		for (; it != virInstList.end(); it++)
		{
			if (*it == virInst) {
				virInstList.erase(it);
				break;
			}
		}

		it = virInsts.begin();
		for (; it != virInsts.end(); it++)
		{
			if (*it == virInst) {
				virInsts.erase(it);
				break;
			}
		}

		virInstSet.erase(virInst);

	}

	//获取虚拟乐器列表的备份
	vector<VirInstrument*>* Synther::TakeVirInstrumentList()
	{
		vector<VirInstrument*>* cpyInsts = new vector<VirInstrument*>();
		for (int i = 0; i < virInstList.size(); i++)
		{
			cpyInsts->push_back(virInstList[i]);
		}

		return cpyInsts;
	}

	// 渲染每帧音频
	void Synther::Render()
	{
		//清除通道buffer
		ClearChannelBuffer();

		for (childFramePos = 0; childFramePos < frameSampleCount; childFramePos += tau->childFrameSampleCount)
		{
			curtSampleCount += tau->childFrameSampleCount;
			sec = tau->invSampleProcessRate * curtSampleCount;

			ProcessMidiEvents();

			//移除需要删除的乐器
			RemoveNeedDeleteVirInsts();

			//渲染虚拟乐器区域发声
			RenderVirInstZoneSound();

			//移除已完成采样的KeySounder
			RemoveProcessEndedKeySounder();

		}

		//混合所有乐器中的样本到synther的声道buffer中
		MixVirInstsSamplesToChannelBuffer();
		SynthSampleBuffer();
	}


	//清除所有乐器的通道buffer
	void Synther::ClearChannelBuffer()
	{
		memset(leftChannelSamples, 0, sizeof(float) * frameSampleCount);
		memset(rightChannelSamples, 0, sizeof(float) * frameSampleCount);

		for (int i = 0; i < virInstList.size(); i++)
		{
			VirInstrument& virInst = *virInstList[i];
			virInst.ClearChannelSamples();
		}
	}

	// 渲染虚拟乐器区域发声
	void Synther::RenderVirInstZoneSound()
	{
		//为渲染准备所有正在发声的区域
		totalZoneSounderCount = 0;
		for (int i = 0; i < virInstList.size(); i++)
		{
			virInstList[i]->CreateKeySounders();

			totalZoneSounderCount +=
				virInstList[i]->CreateZoneSounderForRender(
					totalZoneSounders, totalZoneSounderCount);
		}

		//快速释音超过限制的区域发声
		if (tau->limitZoneSounderCount > 64)
			FastReleaseZoneSounders();
		else
			FastReleaseZoneSounders2();


		//printf("声音总数:%d\n", totalZoneSounderCount);
		//开始渲染区域声音
		if (totalZoneSounderCount <= 0)
			return;

		//是否使用线程池并行处理按键发音数据
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧(frameSampleCount)中调用太过频繁，效率并不是太好
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		if (!tau->useMulThreads)
		{
			for (int i = 0; i < totalZoneSounderCount; i++)
			{
				totalZoneSounders[i]->SetFrameBuffer(leftChannelFrameBuf, rightChannelFrameBuf);
				totalZoneSounders[i]->Render();
				totalZoneSounders[i]->GetVirInstrument()->CombineZoneSounderSamples(totalZoneSounders[i]);
			}
		}
		else
		{
			ZoneSounderThreadPool->Render(totalZoneSounders, totalZoneSounderCount);
			ZoneSounderThreadPool->Wait();
		}

	}

	//移除需要删除的乐器
	void Synther::RemoveNeedDeleteVirInsts(bool isDirectRemove)
	{
		if (needDeleteVirInsts.empty())
			return;

		VirInstrument* virInst;
		vector<VirInstrument*>::iterator it = needDeleteVirInsts.begin();
		for (; it != needDeleteVirInsts.end();)
		{
			virInst = *it;
			if (isDirectRemove || virInst->GetState() == VirInstrumentState::REMOVED) {
				it = needDeleteVirInsts.erase(it);
				DelVirInstrument(virInst);
			}
			else
				it++;
		}
	}

	//快速释音超过限制的区域发声
	void Synther::FastReleaseZoneSounders()
	{
		int instSize = (int)virInsts.size();
		if (instSize <= 0)
			return;

		if (instSize > 1) {
			//按乐器当前区域发声数量由少到多排列乐器
			sort(virInsts.begin(), virInsts.end(), SounderCountCompare);
		}

		int soundInstSize = 0;
		for (int i = 0; i < instSize; i++)
		{
			if (virInsts[i]->GetZoneSounderCount() > 0)
				soundInstSize++;
		}
		if (soundInstSize == 0)
			return;

		VirInstrument* inst;
		int count;
		int limitTotalCount = tau->limitZoneSounderCount;
		int limitPerElemCount = limitTotalCount / soundInstSize;
		int curtSoundInstSize = 0;
		ZoneSounder** ZoneSounders;
		for (int i = 0; i < instSize; i++)
		{
			inst = virInsts[i];
			count = inst->GetZoneSounderCount();
			if (count == 0)
				continue;

			curtSoundInstSize++;

			if (count <= limitPerElemCount) {
				limitTotalCount -= count;
				if (soundInstSize - curtSoundInstSize == 0)
					break;
				limitPerElemCount = limitTotalCount / (soundInstSize - curtSoundInstSize);
				continue;
			}

			ZoneSounders = inst->GetZoneSounders();
			for (int j = limitPerElemCount; j < count; j++) {
				ZoneSounders[j]->SetHoldDownKey(false);
				ZoneSounders[j]->OffKey(127, 0.003f);
			}

			//
			limitTotalCount -= limitPerElemCount;
			if (soundInstSize - curtSoundInstSize == 0)
				break;

			limitPerElemCount = limitTotalCount / (soundInstSize - curtSoundInstSize);

		}
	}

	//快速释音超过限制的区域发声2
	//比例算法
	void Synther::FastReleaseZoneSounders2()
	{
		int instSize = (int)virInsts.size();
		if (instSize <= 0)
			return;

		if (instSize > 1) {
			//按乐器当前区域发声数量由少到多排列乐器
			sort(virInsts.begin(), virInsts.end(), SounderCountCompare);
		}

		int limitTotalCount = tau->limitZoneSounderCount;
		VirInstrument* inst;
		float totalCount = 0;
		int count = 0;
		ZoneSounder** ZoneSounders;

		for (int i = 0; i < instSize; i++)
		{
			inst = virInsts[i];
			count = inst->GetZoneSounderCount();
			if (count > limitTotalCount)
				count = limitTotalCount;
			instSoundCount[i] = count;
			totalCount += count;
		}

		if (totalCount < limitTotalCount)
			return;

		count = 0;
		for (int i = 0; i < instSize; i++)
		{
			inst = virInsts[i];
			if (instSoundCount[i] == 0)
				continue;

			float scale = instSoundCount[i] / totalCount;
			int m = min(instSoundCount[i], (int)round(limitTotalCount * scale));
			if (m == 0) m = 1;
			instSoundCount[i] = m;
			if (i + 1 == instSize)
				instSoundCount[i] = max(0, limitTotalCount - count);
			count += m;
		}

		//
		for (int i = 0; i < instSize; i++)
		{
			inst = virInsts[i];
			inst->realZoneSounderCount = instSoundCount[i];
			ZoneSounders = inst->GetZoneSounders();
			for (int j = instSoundCount[i]; j < inst->GetZoneSounderCount(); j++)
			{
				ZoneSounders[j]->SetHoldDownKey(false);
				ZoneSounders[j]->OffKey(127, 0.003f);
			}
		}
	}

	bool Synther::SounderCountCompare(VirInstrument* a, VirInstrument* b)
	{
		return a->GetZoneSounderCount() < b->GetZoneSounderCount();//升序
	}


	//混合所有乐器中的样本到synther的声道buffer中
	void Synther::MixVirInstsSamplesToChannelBuffer()
	{
		for (int i = 0; i < virInstList.size(); i++)
		{
			VirInstrument& virInst = *virInstList[i];
			if (virInst.IsSoundEnd())
				continue;

			isSoundEnd = false;
			virInst.ApplyEffectsToChannelBuffer();

			float* instLeftChannelSamples = virInst.GetLeftChannelSamples();
			float* instRightChannelSamples = virInst.GetRightChannelSamples();

			switch (tau->channelOutputMode)
			{
			case ChannelOutputMode::Stereo:
				for (int n = 0; n < frameSampleCount; n++)
				{
					leftChannelSamples[n] += instLeftChannelSamples[n];
					rightChannelSamples[n] += instRightChannelSamples[n];
				}

				break;

			case ChannelOutputMode::Mono:
				for (int n = 0; n < frameSampleCount; n++)
					leftChannelSamples[n] += instLeftChannelSamples[n] + instRightChannelSamples[n];

				break;
			}
		}
	}

	//合成采样buffer
	void Synther::SynthSampleBuffer()
	{
		if (IsCacheEnable()) {
			CacheInput();
		}
		else {
			CombineChannelBufferToStream();
			effects->Process();
			FadeSynthStream();
		}

		//
		if (isReqDelete)
		{
			if (isSoundEnd) {
				isReqDelete = false;
				int type = isReqDelete ? -1 : 0;
				DelAllVirInstrument(type);
				effects->Clear();
				totalZoneSounderCount = 0;

				//
				waitSem.set();
			}
		}
	}


	//合并声道buffer到数据流
	void Synther::CombineChannelBufferToStream()
	{
		//合并左右声道采样值到流
		float* out = (float*)synthSampleStream;

		switch (tau->channelOutputMode)
		{
		case ChannelOutputMode::Stereo:
		{
			for (int i = 0; i < frameSampleCount; i++)
			{
				*out++ = leftChannelSamples[i];
				*out++ = rightChannelSamples[i];
			}
		}
		break;

		case ChannelOutputMode::Mono:

			for (int i = 0; i < frameSampleCount; i++)
				*out++ = leftChannelSamples[i];

			break;
		}
	}


	//渐隐合成流的声音
	//对effects效果器产生的余音进行检测，以确保没有任何余音，才结束
	void Synther::FadeSynthStream()
	{
		if (isReqDelete && !isSoundEnd)
		{
			float* out = (float*)synthSampleStream;
			if (channelCount == 2) {
				for (int i = 0; i < channelCount * frameSampleCount; i += 2)
				{
					soundEndGain -= 0.0001;
					if (soundEndGain < 0)
						soundEndGain = 0;
					out[i] *= soundEndGain;
					out[i + 1] *= soundEndGain;
				}
			}
			else {
				for (int i = 0; i < frameSampleCount; i++)
				{
					soundEndGain -= 0.0001;
					if (soundEndGain < 0)
						soundEndGain = 0;
					out[i] *= soundEndGain;
				}
			}
		}

		if (soundEndGain == 0)
			isSoundEnd = true;
	}





	// 移除已完成所有区域发声处理(采样处理)的KeySounder      
	void Synther::RemoveProcessEndedKeySounder()
	{
		for (int i = 0; i < virInstList.size(); i++)
		{
			virInstList[i]->RemoveProcessEndedKeySounder();
		}
	}

	//////////////////////////////////////////////////////////////
	// 录制所有乐器弹奏为midi
	void Synther::RecordMidi()
	{
		RecordMidi(nullptr);
	}

	// 录制指定乐器弹奏为midi
	void Synther::RecordMidi(VirInstrument* virInst)
	{
		if (virInst != nullptr) {
			if (virInstSet.find(virInst) != virInstSet.end())
				virInst->RecordMidi();
			return;
		}

		for (int i = 0; i < virInstList.size(); i++)
		{
			virInstList[i]->RecordMidi();
		}
	}

	/// 停止所有乐器midi的录制
	void Synther::StopRecordMidi()
	{
		StopRecordMidi(nullptr);
	}

	/// <summary>
	/// 停止指定乐器midi的录制
	/// </summary>
	/// <param name="virInst">如果为null,将停止录制所有乐器</param>
	void Synther::StopRecordMidi(VirInstrument* virInst)
	{
		if (virInst != nullptr) {
			if (virInstSet.find(virInst) != virInstSet.end())
				virInst->StopRecordMidi();
			return;
		}

		for (int i = 0; i < virInstList.size(); i++)
		{
			virInstList[i]->StopRecordMidi();
		}
	}


	// 获取录制的midi轨道
	vector<MidiTrack*>* Synther::TakeRecordMidiTracks(
		VirInstrument** virInst, int size, float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos)
	{
		MidiTrack* midiTrack;
		vector<MidiTrack*>* midiTracks = new vector<MidiTrack*>();

		if (virInst != nullptr && size > 0)
		{
			for (int i = 0; i < size; i++)
			{
				if (virInstSet.find(virInst[i]) != virInstSet.end())
				{
					midiTrack = virInst[i]->TakeMidiTrack(recordMidiTickForQuarterNote, tempos);
					if (!midiTrack)
						midiTracks->push_back(midiTrack);
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < virInstList.size(); i++)
			{
				midiTrack = virInstList[i]->TakeMidiTrack(recordMidiTickForQuarterNote, tempos);
				if (midiTrack == nullptr)
					continue;

				midiTracks->push_back(midiTrack);
			}
		}

		return midiTracks;
	}

}


