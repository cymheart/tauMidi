#include"VirInstrument.h"
#include"KeySounder.h"
#include"RegionSounder.h"
#include"Ventrue.h"
#include"Channel.h"


namespace ventrue
{
	VirInstrument::VirInstrument(Ventrue* ventrue, Channel* channel, Preset* preset)
	{
		this->ventrue = ventrue;
		this->channel = channel;
		this->preset = preset;
		keySounders = new KeySounderList;
		onKeySounders = new vector<KeySounder*>;

		//
		effects = new EffectList();
		effects->Set(leftChannelSamples, rightChannelSamples, ventrue->frameSampleCount);

		//
		if (ventrue->GetChannelOutputMode() == ChannelOutputMode::Mono) {
			EffectDelay* delay = new EffectDelay();
			effects->AppendEffect(delay);
		}

		//
		midiTrackRecord = new MidiTrackRecord();
		channel->SetMidiRecord(midiTrackRecord);


	}

	VirInstrument::~VirInstrument()
	{
		channel = nullptr;
		preset = nullptr;

		DEL(effects);
		DEL(midiTrackRecord);

		if (!FindKeySounderFromKeySounders(lastKeySounder))
			DEL(lastKeySounder);

		DEL_OBJS_LIST(keySounders, KeySounderList);
		DEL(onKeySounders);
	}


	//增加效果器
	void VirInstrument::AddEffect(VentrueEffect* effect)
	{
		effects->AppendEffect(effect);
	}


	//清除通道样本缓存
	void VirInstrument::ClearChannelSamples()
	{
		memset(leftChannelSamples, 0, sizeof(float) * ventrue->frameSampleCount);
		memset(rightChannelSamples, 0, sizeof(float) * ventrue->frameSampleCount);
	}

	//调制生成器参数
	void VirInstrument::ModulationParams()
	{
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			KeySounder& keySounder = *(*it);
			keySounder.ModulationParams();
		}
	}

	/// <summary>
	/// 录制为midi
	/// </summary>
	/// <param name="bpm">录制的BPM</param>
	/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
	void VirInstrument::RecordMidi(float bpm, float tickForQuarterNote)
	{
		if (midiTrackRecord->IsRecord())
			return;

		midiTrackRecord->Stop();
		midiTrackRecord->Clear();
		midiTrackRecord->SetBPM(bpm);
		midiTrackRecord->SetTickForQuarterNote(tickForQuarterNote);
		midiTrackRecord->Start();

		//
		midiTrackRecord->RecordSetController(MidiControllerType::BankSelectMSB, channel->GetBankSelectMSB(), channel->GetChannelNum());
		midiTrackRecord->RecordSetController(MidiControllerType::BankSelectLSB, channel->GetBankSelectLSB(), channel->GetChannelNum());
		midiTrackRecord->RecordSetProgramNum(channel->GetProgramNum(), channel->GetChannelNum());
	}

	//停止录制midi
	void VirInstrument::StopRecordMidi()
	{
		midiTrackRecord->Stop();
	}

	// 获取录制的midi轨道
	MidiTrack* VirInstrument::TakeMidiTrack(float baseTickForQuarterNote)
	{
		return midiTrackRecord->TakeMidiTrack(baseTickForQuarterNote);
	}


	//所有区域发音是否结束
	//这个检测不能检测到效果器作用是否结束
	bool VirInstrument::IsAllKeySoundEnd()
	{
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			if (!(*it)->IsSoundEnd())
				return false;
		}
		return true;
	}


	//按键
	KeySounder* VirInstrument::OnKey(int key, float velocity)
	{
		KeySounder* keySounder = KeySounder::New();
		_OnKey(keySounder, key, velocity);

		//使用单音模式时，其它保持按键状态并在发音当中的keySounder将被快速释音，
		//同时使其保持在按键状态队列当中，只有这样在释放当前发音按键的时候，其他保持按键状态的key才能恢复发音
		if (useMonoMode)
		{
			for (int i = 0; i < onKeySounders->size(); i++)
			{
				if ((*onKeySounders)[i] == keySounder)
					continue;
				(*onKeySounders)[i]->IsHoldInSoundQueue = true;
				(*onKeySounders)[i]->OffKey();
			}
		}

		//录制
		midiTrackRecord->RecordOnKey(key, velocity, channel->GetChannelNum());

		return keySounder;
	}

	//按键动作送入RegionSounder中执行处理
	void VirInstrument::_OnKey(KeySounder* keySounder, int key, float velocity)
	{
		isSoundEnd = false;
		keySounder->OnKey(key, velocity, this);
		keySounder->CreateExclusiveClassList(exclusiveClasses);

		//设置具有相同独占类的区域将不再处理样本
		StopExclusiveClassRegionSounderProcess(exclusiveClasses);

		onKeySounders->push_back(keySounder);
		keySounders->push_back(keySounder);
	}


	//松开按键,一次只能关闭一个按键
	void VirInstrument::OffKey(int key, float velocity)
	{
		KeySounder* keySounder = nullptr;
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			if ((*it)->IsOnningKey(key)) {
				keySounder = *it;
				break;
			}
		}

		OffKey(keySounder, velocity);
	}

	// 执行松开指定发音按键
	void VirInstrument::OffKey(KeySounderID keySounderID, float velocity)
	{
		KeySounder* keySounder = GetOnKeyStateSounder(keySounderID);
		OffKey(keySounder, velocity);
	}

	// 松开指定发音按键
	void VirInstrument::OffKey(KeySounder* keySounder, float velocity)
	{
		_OffKey(keySounder, velocity, true);
	}

	// 松开指定发音按键
	void VirInstrument::_OffKey(KeySounder* keySounder, float velocity, bool isRecord)
	{
		if (keySounder == nullptr)
			return;

		//录制
		if (isRecord)
			midiTrackRecord->RecordOffKey(keySounder->GetOnKey(), velocity, channel->GetChannelNum());

		//
		if ((!keySounder->IsOnningKey() &&
			!keySounder->IsHoldInSoundQueue))
			return;

		if (keySounder->IsHoldDownKey() && !useMonoMode)
		{
			keySounder->NeedOffKey();
			return;
		}

		KeySounder* lastKeySounder = GetLastOnKeyStateSounder();
		if (lastKeySounder == nullptr)
			return;

		keySounder->IsHoldInSoundQueue = false;
		keySounder->OffKey(velocity);
		RemoveOnKeyStateSounder(keySounder);

		//单音模式中会保持按下的按键按键历史队列当中，当释放一个发音按键后
		//倒数第二个保持按键状态的按键将重新发音
		if (useMonoMode && lastKeySounder == keySounder)
		{
			MonoModeReSoundLastOnKey();
		}
	}


	// 在Mono模式下重新发音最后一个按键
	void VirInstrument::MonoModeReSoundLastOnKey()
	{
		KeySounder* lastOnKeySounder = GetLastOnKeyStateSounder();
		if (lastOnKeySounder == nullptr)
			return;

		//按键将不会再保留，在下一次清理中将被移除 
		lastOnKeySounder->IsHoldInSoundQueue = false;

		//如果滑音状态下，由于会生成最后一个保持按键的重新发音的KeySounder,此时声音会重新滑向这个发音键，状态不冲突
		//一个新的发音生成之后，才会重队列中执行移除按键的操作
		KeySounder* keySounder = KeySounder::New();
		keySounder->SetID(lastOnKeySounder->GetID());
		lastOnKeySounder->SetID(0);
		lastOnKeySounder->OffKey();
		_RemoveOnKeyStateSounder(lastOnKeySounder);

		_OnKey(keySounder,
			lastOnKeySounder->GetOnKey(),
			lastOnKeySounder->GetVelocity());

	}

	//移除指定按下状态的按键发音器
	bool VirInstrument::_RemoveOnKeyStateSounder(KeySounder* keySounder)
	{
		vector<KeySounder*>::iterator it = onKeySounders->begin();
		vector<KeySounder*>::iterator end = onKeySounders->end();
		for (; it != end; it++)
		{
			if (*it == keySounder)
			{
				onKeySounders->erase(it);
				return true;
			}
		}

		return false;
	}

	//移除指定按下状态的按键发音器
	void VirInstrument::RemoveOnKeyStateSounder(KeySounder* keySounder)
	{
		_RemoveOnKeyStateSounder(keySounder);

		//最后一个保持按键状态的keySounder,总是不被直接从内存池中移除，
	   //而是放入lastKeySounder，供总是保持滑音状态使用，因为“保持滑音状态”
	   //需要最后一个发音作参考，来生成滑音,正常状态的滑音是靠两个同时处于按键状态的keySounder先后来生成的
	   //当开起一直保持功能后，生成滑音只需要最近一个按键的发音信息即可，而无所谓是否按下还是已经按下过
		if (onKeySounders->size() == 0) {

			if (lastKeySounder != nullptr) {
				if (!FindKeySounderFromKeySounders(lastKeySounder))
					lastKeySounder->Release();
			}

			lastKeySounder = keySounder;
		}
	}


	// 获取最后在按状态的按键发音器(不包括最后松开的按键)
	KeySounder* VirInstrument::GetLastOnKeyStateSounder()
	{
		if (onKeySounders->size() == 0)
			return nullptr;
		return (*onKeySounders)[onKeySounders->size() - 1];
	}

	// 获取指定的实际按下的按键的KeySounder
	KeySounder* VirInstrument::GetOnKeyStateSounder(KeySounderID keySounderID)
	{
		for (int i = 0; i < onKeySounders->size(); i++)
		{
			if ((*onKeySounders)[i]->GetID() == keySounderID)
				return (*onKeySounders)[i];
		}

		return nullptr;
	}

	//从KeySounders中查找KeySounder
	bool VirInstrument::FindKeySounderFromKeySounders(KeySounder* keySounder)
	{
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			if (*it == keySounder)
				return true;
		}

		return false;
	}


	// 找寻最后一个按键发声区域中具有同样乐器区域的regionSounder
	RegionSounder* VirInstrument::FindLastSameRegion(Region* region)
	{
		KeySounder* keySounder = GetLastOnKeyStateSounder();
		if (keySounder == nullptr)
		{
			if (alwaysUsePortamento)
				keySounder = GetLastKeySounder();
			else
				return nullptr;

			if (keySounder == nullptr)
				return nullptr;
		}

		RegionSounderList& regionSounderList = *(keySounder->GetRegionSounderList());
		size_t size = regionSounderList.size();
		for (int i = 0; i < size; i++)
		{
			if (regionSounderList[i]->instRegion == region)
				return regionSounderList[i];
		}

		return nullptr;
	}

	// 设置具有相同独占类的区域将不再处理样本
	// exclusiveClasses数组以一个小于等于0的值结尾
	void VirInstrument::StopExclusiveClassRegionSounderProcess(int* exclusiveClasses)
	{
		if (exclusiveClasses[0] < 0)
			return;

		KeySounder* keySounder;
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			keySounder = *it;
			for (int j = 0; exclusiveClasses[j] > 0; j++)
				keySounder->StopExclusiveClassRegionSounderProcess(exclusiveClasses[j]);
		}
	}

	//为渲染准备所有正在发声的区域
	int VirInstrument::CreateRegionSounderForRender(RegionSounder** totalRegionSounder, int startSaveIdx)
	{
		if (IsSoundEnd())
			return 0;

		float reverbDepth, chorusDepth;
		float maxReverbDepth = 0, maxChorusDepth = 0;
		int idx = startSaveIdx;
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();


		for (; it != end; it++)
		{
			RegionSounderList* regionSounderList = (*it)->GetRegionSounderList();
			size_t size = regionSounderList->size();
			for (int i = 0; i < size; i++)
			{
				RegionSounder* regionSounder = (*regionSounderList)[i];

				if (regionSounder->IsSoundEnd())
					continue;

				totalRegionSounder[idx++] = regionSounder;

				//获取区域混音深度数值
				reverbDepth = regionSounder->GetGenReverbEffectsSend() * 0.01f;
				if (reverbDepth > maxReverbDepth)
					maxReverbDepth = reverbDepth;

				//获取区域和声深度数值
				chorusDepth = regionSounder->GetGenChorusEffectsSend() * 0.01f;
				if (chorusDepth > maxChorusDepth)
					maxChorusDepth = chorusDepth;
			}
		}

		//存在发音时，设置调整混音，和声数值
		if (idx > startSaveIdx) {
			SetRegionReverbDepth(maxReverbDepth);
			SetRegionChorusDepth(maxChorusDepth);
		}

		return idx - startSaveIdx;
	}

	//设置区域混音深度
	void VirInstrument::SetRegionReverbDepth(float value)
	{
		regionReverbDepth = value;
		if (regionReverb != nullptr)
			regionReverb->SetEnable(false);

		if (regionReverbDepth != 0)
		{
			if (!regionReverb)
			{
				regionReverb = new EffectReverb();
				effects->AppendEffect(regionReverb);
			}

			regionReverb->SetEnable(true);

			if (regionReverbDepth != regionReverb->GetEffectMix())
			{
				regionReverb->SetRoomSize(0.8f);
				regionReverb->SetWidth(0.2f);
				regionReverb->SetDamping(0.5f);
				regionReverb->SetEffectMix(regionReverbDepth);
			}
		}
	}

	//设置区域和声深度
	void VirInstrument::SetRegionChorusDepth(float value)
	{
		regionChorusDepth = value;
		if (regionChorus != nullptr)
			regionChorus->SetEnable(false);

		if (regionChorusDepth != 0)
		{
			if (!regionChorus) {
				regionChorus = new EffectChorus();
				effects->AppendEffect(regionChorus);
			}

			regionChorus->SetEnable(true);

			if (regionChorusDepth != regionChorus->GetEffectMix())
			{
				regionChorus->SetModDepth(0.8f);
				regionChorus->SetModFrequency(0.008f);
				regionChorus->SetEffectMix(regionChorusDepth);
			}
		}
	}

	//合并区域已处理发音样本
	void VirInstrument::CombineRegionSounderSamples(RegionSounder* regionSounder)
	{
		if (regionSounder->IsSoundEnd() || regionSounder->virInst != this)
			return;

		ChannelOutputMode outputMode = ventrue->GetChannelOutputMode();
		float* regionLeftChannelSamples = regionSounder->GetLeftChannelSamples();
		float* regionRightChannelSamples = regionSounder->GetRightChannelSamples();
		int framePos = ventrue->childFramePos;
		for (int i = 0; i < ventrue->childFrameSampleCount; i++)
		{
			leftChannelSamples[framePos + i] += regionLeftChannelSamples[i];
			rightChannelSamples[framePos + i] += regionRightChannelSamples[i];
		}


		//此时处理不能听到发声的滞留区域，使其结束发音
		regionSounder->EndBlockSound();

	}


	//应用效果器到乐器的声道buffer
	void VirInstrument::ApplyEffectsToChannelBuffer()
	{
		if (isEnableEffect)
			effects->Process();

		//检测发音是否结束
		isSoundEnd = false;
		if (IsAllKeySoundEnd())
		{
			//检测由效果器带来的尾音是否结束
			int offset = (int)(ventrue->frameSampleCount * 0.02f);
			for (int i = 0; i < ventrue->frameSampleCount; i += offset)
			{
				if (fabsf(leftChannelSamples[i]) > 0.0001f ||
					fabsf(rightChannelSamples[i]) > 0.0001f)
					return;
			}

			isSoundEnd = true;
		}
	}


	// 移除已完成所有区域发声处理(采样处理)的KeySounder      
	void VirInstrument::RemoveProcessEndedKeySounder()
	{
		int offIdx = 0;
		KeySounderList::iterator it = keySounders->begin();
		for (; it != keySounders->end(); )
		{
			KeySounder* keySounder = *it;
			if (keySounder->IsSoundEnd())
			{
				if (keySounder->IsOnningKey())
				{
					offKeySounder[offIdx++] = keySounder;
					it++;
					continue;
				}

				if (keySounder != lastKeySounder && !keySounder->IsHoldInSoundQueue)
					keySounder->Release();

				it = keySounders->erase(it);
			}
			else
			{
				it++;
			}
		}

		//
		for (int i = 0; i < offIdx; i++)
		{
			OffKey(offKeySounder[i]);
		}

	}

}
