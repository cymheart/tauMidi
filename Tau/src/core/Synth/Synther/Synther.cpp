#include"Synther.h"
#include"Synth/Tau.h"
#include"Synth/VirInstrument.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"
#include"FX/Compressor.h"
#include"Audio/AudioPa/Audio_pa.h"
#include"Audio/AudioOboe/Audio_oboe.h"
#include"Audio/AudioSDL/Audio_SDL.h"
#include"Audio/AudioRt/Audio_Rt.h"

namespace tau
{
	Synther::Synther(Tau* tau)
	{
		this->tau = tau;
		isFrameRenderCompleted = true;
		computedFrameBufSyntherCount = 0;
		isSoundEnd = false;

		//
		presetBankReplaceMap = new unordered_map<uint32_t, uint32_t>;

		//
		taskProcesser = new TaskProcesser;
		regionSounderThreadPool = new RegionSounderThread(this);


		//
		effects = new EffectList();
		effects->Set(leftChannelSamples, rightChannelSamples, frameSampleCount);

		innerEffects = new EffectList();
		innerEffects->Set(leftChannelSamples, rightChannelSamples, frameSampleCount);

	}

	Synther::~Synther()
	{
		if (!isMainSynther) {
			Synther* s = (Synther*)(tau->mainEditorSynther);
			Semaphore waitSem;
			s->RemoveAssistSyntherTask(&waitSem, this);
			waitSem.wait();
		}

		DEL(audio);

		//
		regionSounderThreadPool->Stop();
		taskProcesser->Stop();


		DEL(taskProcesser);
		DEL(regionSounderThreadPool);

		for (int i = 0; i < virInstList.size(); i++)
			DEL(virInstList[i]);


		DEL(effects);
		DEL(innerEffects);
		DEL(presetBankReplaceMap);
	}

	void Synther::Open()
	{
		if (isMainSynther) {
			OpenAudio();
			audio->Open();
		}

		regionSounderThreadPool->Start();
		taskProcesser->Start();
		isOpened = true;
	}

	void Synther::Close()
	{
		if (audio != nullptr)
			audio->Close();

		regionSounderThreadPool->Stop();
		taskProcesser->Stop();
		isOpened = false;
	}

	void Synther::AddAssistSynther(Synther* assistSynther)
	{
		assistSynthers.push_back(assistSynther);
	}

	void Synther::RemoveAssistSynther(Synther* assistSynther)
	{
		vector<Synther*>::iterator iVector = find(assistSynthers.begin(), assistSynthers.end(), assistSynther);
		if (iVector != assistSynthers.end())
			assistSynthers.erase(iVector);
	}

	float Synther::GetSampleProcessRate()
	{
		if (!tau)
			return 44100;

		return tau->GetSampleProcessRate();
	}

	void Synther::ReqDelete()
	{
		isReqDelete = true;
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
			Compressor* compressor = new Compressor();
			AddEffect(compressor);
		}
		else
		{
			audio = new Audio_pa();
			//添加压缩器效果
			Compressor* compressor = new Compressor();
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

		if (childFrameSampleCount > frameSampleCount) childFrameSampleCount = frameSampleCount;
		else if (childFrameSampleCount < 1)childFrameSampleCount = 1;

		effects->Set(leftChannelSamples, rightChannelSamples, frameSampleCount);
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
			regionSounderThreadPool->Start();
		}
		else
		{
			regionSounderThreadPool->Stop();
		}
	}


	// 请求帧渲染事件
	void Synther::ReqRender()
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = RenderTask;
		isFrameRenderCompleted = false;
		taskProcesser->PostTask(ev);
	}

	void Synther::RenderTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);

		if (synther.isMainSynther)
		{
			synther.computedFrameBufSyntherCount = synther.assistSynthers.size() + 1;
			for (int i = 0; i < synther.assistSynthers.size(); i++)
				synther.assistSynthers[i]->ReqRender();
		}

		synther.Render();

	}

	// 渲染每帧音频
	void Synther::Render()
	{

	}


	// 帧渲染
	void Synther::FrameRender(uint8_t* stream, int len)
	{
		while (true)
		{
			if (isFrameRenderCompleted)
			{
				memcpy(stream, synthSampleStream, len);
				ReqRender();
				break;
			}

#ifdef _WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
	}

	//投递任务
	void Synther::PostTask(TaskCallBack taskCallBack, void* data, int delay)
	{
		taskProcesser->PostTask(taskCallBack, data, delay);
	}

	//投递任务
	void Synther::PostTask(Task* task, int delay)
	{
		taskProcesser->PostTask(task, delay);
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
		(*presetBankReplaceMap)[orgKey] = repKey;
	}

	//移除替换乐器
	void Synther::RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum)
	{
		int orgKey = orgBankMSB << 16 | orgBankLSB << 8 | orgInstNum;
		presetBankReplaceMap->erase(orgKey);
	}

	//获取替换乐器key
	int Synther::GetReplaceInstrumentKey(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		int key = bankSelectMSB << 16 | bankSelectLSB << 8 | instrumentNum;
		auto itReplace = presetBankReplaceMap->find(key);
		if (itReplace != presetBankReplaceMap->end()) {
			key = itReplace->second;
		}

		return key;
	}

	//增加效果器
	void Synther::AddEffect(TauEffect* effect)
	{
		effect->SetSynther(this);
		effects->AppendEffect(effect);
	}

	//调制虚拟乐器参数
	void Synther::ModulationVirInstParams(Channel* channel)
	{
		channel->GetVirInstrument()->ModulationParams();
	}

	//调制虚拟乐器参数
	void Synther::ModulationVirInstParams(VirInstrument* virInst)
	{
		virInst->ModulationParams();
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
		VirInstrument* virInst = GetVirInstrument(channel);

		if (virInst == nullptr)
		{
			virInst = new VirInstrument(this, channel, nullptr);
			virInst->ChangeProgram(bankSelectMSB, bankSelectLSB, instrumentNum);
			virInst->OnExecute(false);
			AppendToVirInstList(virInst);
		}
		else
		{
			virInst->ChangeProgram(bankSelectMSB, bankSelectLSB, instrumentNum);
		}

		return virInst;

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


	//根据通道移除虚拟乐器
	void Synther::RemoveVirInstrument(Channel* channel, bool isFade)
	{
		VirInstrument* virInst = GetVirInstrument(channel);
		RemoveVirInstrument(virInst, isFade);
	}


	//移除虚拟乐器
	void Synther::RemoveVirInstrument(VirInstrument* virInst, bool isFade)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		try
		{
			virInst->Remove(isFade);
		}
		catch (exception e)
		{
			string error(e.what());
			cout << "移除乐器错误原因:" << error << endl;
		}
	}


	// 删除虚拟乐器
	void Synther::DelVirInstrument(VirInstrument* virInst)
	{
		if (virInst == nullptr)
			return;

		RemoveVirInstFromList(virInst);

		//
		DEL(virInst);
	}

	//打开虚拟乐器
	void Synther::OnVirInstrument(VirInstrument* virInst, bool isFade)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->On(isFade);
	}

	//根据通道打开相关虚拟乐器
	void Synther::OnVirInstrument(Channel* channel, bool isFade)
	{
		VirInstrument* virInst = GetVirInstrument(channel);
		OnVirInstrument(virInst, isFade);
	}


	//关闭虚拟乐器
	void Synther::OffVirInstrument(VirInstrument* virInst, bool isFade)
	{
		if (!IsHavVirInstrument(virInst))
			return;

		virInst->Off(isFade);
	}

	//根据通道关闭相关虚拟乐器
	void Synther::OffVirInstrument(Channel* channel, bool isFade)
	{
		VirInstrument* virInst = GetVirInstrument(channel);
		OffVirInstrument(virInst, isFade);
	}


	//根据通道关闭相关虚拟乐器所有按键
	void Synther::OffVirInstrumentAllKeys(Channel* channel)
	{
		VirInstrument* virInst = GetVirInstrument(channel);
		if (virInst == nullptr)
			return;

		virInst->OffAllKeys();
	}


	//根据指定通道获取关连虚拟乐器
	VirInstrument* Synther::GetVirInstrument(Channel* channel)
	{
		if (channel == nullptr)
			return nullptr;

		VirInstrument* virInst = channel->GetVirInstrument();
		if (virInst == nullptr || virInst->IsRemove())
			return nullptr;

		return virInst;
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

	//清除所有乐器的通道buffer
	void Synther::ClearChannelBuffer()
	{
		memset(leftChannelSamples, 0, sizeof(float) * frameSampleCount);
		memset(rightChannelSamples, 0, sizeof(float) * frameSampleCount);

		isVirInstSoundEnd = true;
		for (int i = 0; i < virInstList.size(); i++)
		{
			VirInstrument& virInst = *virInstList[i];
			if (virInst.IsSoundEnd())
				continue;

			virInst.ClearChannelSamples();
		}
	}


	// 渲染虚拟乐器区域发声
	void Synther::RenderVirInstRegionSound()
	{
		//为渲染准备所有正在发声的区域
		totalRegionSounderCount = 0;
		for (int i = 0; i < virInstList.size(); i++)
		{
			virInstList[i]->CreateKeySounders();

			totalRegionSounderCount +=
				virInstList[i]->CreateRegionSounderForRender(
					totalRegionSounders, totalRegionSounderCount);
		}

		//快速释音超过限制的区域发声
		if (tau->limitRegionSounderCount > 64)
			FastReleaseRegionSounders();
		else
			FastReleaseRegionSounders2();


		//printf("声音总数:%d\n", totalRegionSounderCount);
		//开始渲染区域声音
		if (totalRegionSounderCount <= 0)
			return;

		//是否使用线程池并行处理按键发音数据
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧(frameSampleCount)中调用太过频繁，效率并不是太好
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		if (!tau->useMulThreads)
		{
			for (int i = 0; i < totalRegionSounderCount; i++)
			{
				totalRegionSounders[i]->SetFrameBuffer(leftChannelFrameBuf, rightChannelFrameBuf);
				totalRegionSounders[i]->Render();
				totalRegionSounders[i]->GetVirInstrument()->CombineRegionSounderSamples(totalRegionSounders[i]);
			}
		}
		else
		{
			regionSounderThreadPool->Render(totalRegionSounders, totalRegionSounderCount);
			regionSounderThreadPool->Wait();
		}

	}

	//快速释音超过限制的区域发声
	void Synther::FastReleaseRegionSounders()
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
			if (virInsts[i]->GetRegionSounderCount() > 0)
				soundInstSize++;
		}
		if (soundInstSize == 0)
			return;

		VirInstrument* inst;
		int count;
		int limitTotalCount = tau->limitRegionSounderCount;
		int limitPerElemCount = limitTotalCount / soundInstSize;
		int curtSoundInstSize = 0;
		RegionSounder** regionSounders;
		for (int i = 0; i < instSize; i++)
		{
			inst = virInsts[i];
			count = inst->GetRegionSounderCount();
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

			regionSounders = inst->GetRegionSounders();
			for (int j = limitPerElemCount; j < count; j++)
				regionSounders[j]->OffKey(127, 0.003f);

			//
			limitTotalCount -= limitPerElemCount;
			if (soundInstSize - curtSoundInstSize == 0)
				break;

			limitPerElemCount = limitTotalCount / (soundInstSize - curtSoundInstSize);

		}
	}

	//快速释音超过限制的区域发声2
	//比例算法
	void Synther::FastReleaseRegionSounders2()
	{
		int instSize = (int)virInsts.size();
		if (instSize <= 0)
			return;

		if (instSize > 1) {
			//按乐器当前区域发声数量由少到多排列乐器
			sort(virInsts.begin(), virInsts.end(), SounderCountCompare);
		}

		int limitTotalCount = tau->limitRegionSounderCount;
		VirInstrument* inst;
		float totalCount = 0;
		int count = 0;
		RegionSounder** regionSounders;

		for (int i = 0; i < instSize; i++)
		{
			inst = virInsts[i];
			count = inst->GetRegionSounderCount();
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
			inst->realRegionSounderCount = instSoundCount[i];
			regionSounders = inst->GetRegionSounders();
			for (int j = instSoundCount[i]; j < inst->GetRegionSounderCount(); j++)
			{
				regionSounders[j]->OffKey(127, 0.003f);
			}
		}
	}

	bool Synther::SounderCountCompare(VirInstrument* a, VirInstrument* b)
	{
		int regionCountA = a->GetRegionSounderCount();
		int regionCountB = b->GetRegionSounderCount();
		return regionCountA < regionCountB;//升序
	}


	//混合所有乐器中的样本到synther的声道buffer中
	void Synther::MixVirInstsSamplesToChannelBuffer()
	{
		isVirInstSoundEnd = true;
		for (int i = 0; i < virInstList.size(); i++)
		{
			VirInstrument& virInst = *virInstList[i];
			if (virInst.IsSoundEnd())
				continue;

			isSoundEnd = false;
			isVirInstSoundEnd = false;
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

	//复合各个合成器中的framebuf
	void Synther::CombineSynthersFrameBufs()
	{
		if (isMainSynther)
		{
			computedFrameBufSyntherCount--;
			if (computedFrameBufSyntherCount == 0)
			{
				_CombineSynthersFrameBufs();
			}
		}
		else
		{
			ApplyEffectsToChannelBuffer();

			Synther& mainSynther = *(Synther*)(tau->mainEditorSynther);
			mainSynther.computedFrameBufSyntherCount--;
			if (mainSynther.computedFrameBufSyntherCount == 0)
				mainSynther.CombineSynthersFrameBufsTask();
		}
	}

	void Synther::_CombineSynthersFrameBufs()
	{
		CombineAssistToMainBuffer();
		ApplyEffectsToChannelBuffer();
		CombineChannelBufferToStream();
		isFrameRenderCompleted = true;
	}


	//合并辅助合成器buffer到主buffer中
	void Synther::CombineAssistToMainBuffer()
	{
		Synther* synther;
		for (int i = 0; i < assistSynthers.size(); i++)
		{
			synther = assistSynthers[i];

			switch (tau->channelOutputMode)
			{
			case ChannelOutputMode::Stereo:
				for (int n = 0; n < frameSampleCount; n++)
				{
					leftChannelSamples[n] += synther->leftChannelSamples[n];
					rightChannelSamples[n] += synther->rightChannelSamples[n];
				}

				break;

			case ChannelOutputMode::Mono:
				for (int n = 0; n < frameSampleCount; n++)
					leftChannelSamples[n] += synther->leftChannelSamples[n];
				break;
			}
		}
	}

	//应用效果器到乐器的声道buffer
	void Synther::ApplyEffectsToChannelBuffer()
	{
		if (isSoundEnd)
		{
			for (int i = 0; i < assistSynthers.size(); i++) {
				isSoundEnd &= assistSynthers[i]->isSoundEnd;
				if (!isSoundEnd)
					break;
			}

			if (isSoundEnd)
				return;
		}

		//对声道应用效果器
		effects->Process();

		if (isVirInstSoundEnd)
		{
			for (int i = 0; i < assistSynthers.size(); i++)
			{
				isVirInstSoundEnd &= assistSynthers[i]->isVirInstSoundEnd;
				if (!isVirInstSoundEnd)
					break;
			}
		}

		if (isVirInstSoundEnd)
		{
			//检测由效果器带来的尾音是否结束
			int offset = (int)(frameSampleCount * 0.02f);
			for (int i = 0; i < frameSampleCount; i += offset)
			{
				// 此处值需要非常小，不然会产生杂音
				if (fabsf(leftChannelSamples[i]) > 0.0001f ||
					fabsf(rightChannelSamples[i]) > 0.0001f)
					return;
			}

			isSoundEnd = true;
		}
	}



	//设置过渡效果深度信息
	void Synther::SettingFadeEffectDepthInfo(float curtEffectDepth, FadeEffectDepthInfo& fadeEffectDepthInfo)
	{
		if (abs(curtEffectDepth - fadeEffectDepthInfo.dstDepth) > 0.001)
		{
			fadeEffectDepthInfo.startDepth = fadeEffectDepthInfo.curtDepth;
			fadeEffectDepthInfo.dstDepth = curtEffectDepth;

			if (abs(fadeEffectDepthInfo.dstDepth - fadeEffectDepthInfo.startDepth) < 0.001)
				fadeEffectDepthInfo.isFadeDepth = false;
			else {
				fadeEffectDepthInfo.isFadeDepth = true;
				fadeEffectDepthInfo.startFadeSec = sec;
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

			for (int i = 0; i < frameSampleCount; i++)
			{
				*out++ = leftChannelSamples[i];
				*out++ = rightChannelSamples[i];
			}
			break;

		case ChannelOutputMode::Mono:
			for (int i = 0; i < frameSampleCount; i++)
				*out++ = leftChannelSamples[i];
			break;
		}
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
