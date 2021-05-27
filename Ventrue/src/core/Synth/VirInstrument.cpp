#include"VirInstrument.h"
#include"KeySounder.h"
#include"RegionSounder.h"
#include"Ventrue.h"
#include"Channel.h"
#include"Preset.h"
#include"Track.h"
#include <random>
#include"VentrueCmd.h"


namespace ventrue
{
	VirInstrument::VirInstrument(Ventrue* ventrue, Channel* channel, Preset* preset)
	{
		this->ventrue = ventrue;
		this->channel = channel;
		this->preset = preset;
		keySounders = new KeySounderList;
		onKeySounders = new vector<KeySounder*>;
		onkeyEventMap = new unordered_map<int, list<KeyEvent>>();
		offkeyEventMap = new unordered_map<int, list<KeyEvent>>();
		onKeySecHistorys = new list<float>;
		stateOps = new vector<VirInstrumentStateOp>;

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
		DEL(effects);
		DEL(midiTrackRecord);

		if (!FindKeySounderFromKeySounders(lastKeySounder))
			DEL(lastKeySounder);

		DEL_OBJS_LIST(keySounders, KeySounderList);
		DEL(onKeySounders);
		DEL(onkeyEventMap);
		DEL(offkeyEventMap);
		DEL(onKeySecHistorys);
		DEL(stateOps);

		channel = nullptr;
		preset = nullptr;
	}

	//打开乐器
	void VirInstrument::On(bool isFade)
	{
		if (stateOps->empty())
			canExecuteStateOp = true;
		else if (stateOps->back().opType == VirInstrumentStateOpType::ON)
			return;

		VirInstrumentStateOp op = { VirInstrumentStateOpType::ON , isFade };
		stateOps->push_back(op);

	}

	//关闭乐器
	void VirInstrument::Off(bool isFade)
	{
		if (stateOps->empty())
			canExecuteStateOp = true;
		else if (stateOps->back().opType == VirInstrumentStateOpType::OFF)
			return;

		VirInstrumentStateOp op = { VirInstrumentStateOpType::OFF , isFade };
		stateOps->push_back(op);
	}

	//移除乐器
	void VirInstrument::Remove(bool isFade)
	{
		isRemove = true;

		if (stateOps->empty())
			canExecuteStateOp = true;

		VirInstrumentStateOp op = { VirInstrumentStateOpType::OFF , isFade };
		stateOps->push_back(op);
		op = { VirInstrumentStateOpType::REMOVE , isFade };
		stateOps->push_back(op);
	}

	//状态操作
	void VirInstrument::StateOp(VirInstrumentStateOp op)
	{
		switch (op.opType)
		{
		case VirInstrumentStateOpType::ON:
			OnExecute(op.isFade);
			break;
		case VirInstrumentStateOpType::OFF:
			OffExecute(op.isFade);
			break;
		case VirInstrumentStateOpType::REMOVE:
			ventrue->GetCmd()->DelVirInstrument(this);
			break;
		default:
			break;
		}
	}

	//执行打开乐器
	void VirInstrument::OnExecute(bool isFade)
	{
		if (state == VirInstrumentState::ONING ||
			state == VirInstrumentState::ONED)
			return;


		state = VirInstrumentState::ONING;
		startGain = !isFade ? 1 : gain;
		dstGain = 1;
		startGainFadeSec = ventrue->sec;
		totalGainFadeTime = 0.2f;

		//
		if (virInstStateChangedCB != nullptr)
			virInstStateChangedCB(this);
	}

	//执行关闭乐器
	void VirInstrument::OffExecute(bool isFade)
	{
		if (state == VirInstrumentState::OFFING ||
			state == VirInstrumentState::OFFED)
			return;

		state = VirInstrumentState::OFFING;
		startGain = !isFade ? 0 : gain;
		dstGain = 0;
		startGainFadeSec = ventrue->sec;
		totalGainFadeTime = 0.2f;

		//
		if (virInstStateChangedCB != nullptr)
			virInstStateChangedCB(this);
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

	//计算按键速度
	void VirInstrument::ComputeOnKeySpeed()
	{
		onKeySecHistorys->push_back(ventrue->sec);
		float tm = 0;
		int idx = 0;
		float size = onKeySecHistorys->size();

		list<float>::iterator it = onKeySecHistorys->begin();
		list<float>::iterator end = onKeySecHistorys->end();
		for (; it != end; it++, idx++)
		{
			tm = ventrue->sec - *it;
			if (tm >= 0.1f)
				break;
		}

		if (tm < 0.1f)
			return;

		onKeySecHistorys->erase(onKeySecHistorys->begin());
		onKeySpeed = (size - idx) / tm / 10.0f;

		//printf("乐器%s:onKeySpeed:%.2f n/s\n", preset->name.c_str(), onKeySpeed);
	}

	//是否可以忽略按键
	bool VirInstrument::CanIgroneOnKey(int key, float velocity, int tickCount, bool isRealTime)
	{
		//对同时产生的大量发音按键进行忽略
		//忽略算法:如果当前所有区域发音数量超过极限值的一半
		//并且当前非实时按键发音....
		if (ventrue->totalRegionSounderCount > ventrue->limitRegionSounderCount * 0.5f)
		{
			if (!isRealTime && tickCount <= 5)
			{
				//录制	
				if (channel != nullptr)
					midiTrackRecord->RecordOffKey(key, velocity, channel->GetChannelNum());

				return true;
			}

			//计算按键速度
			ComputeOnKeySpeed();

			if (onKeySpeed > 800)
			{
				//录制	
				if (channel != nullptr)
					midiTrackRecord->RecordOffKey(key, velocity, channel->GetChannelNum());

				return true;
			}
		}
		else
		{
			//计算按键速度
			ComputeOnKeySpeed();
		}

		return false;
	}


	//按键
	void VirInstrument::OnKey(int key, float velocity, int tickCount, bool isRealTime)
	{
		if (state == VirInstrumentState::OFFED)
			return;

		//判断是否可以忽略按键
		if (CanIgroneOnKey(key, velocity, tickCount, isRealTime))
			return;

		//
		KeyEvent keyEvent;
		keyEvent.isOnKey = true;
		keyEvent.key = key;
		keyEvent.velocity = velocity;
		keyEvent.isRealTime = isRealTime;
		(*onkeyEventMap)[key].push_back(keyEvent);
	}

	//松开按键
	void VirInstrument::OffKey(int key, float velocity, bool isRealTime)
	{
		if (state == VirInstrumentState::OFFED)
			return;

		KeyEvent keyEvent;
		keyEvent.isOnKey = false;
		keyEvent.key = key;
		keyEvent.velocity = velocity;
		keyEvent.isRealTime = isRealTime;
		(*offkeyEventMap)[key].push_back(keyEvent);
	}

	//松开所有按键
	void VirInstrument::OffAllKeys(bool isRealTime)
	{
		if (state == VirInstrumentState::OFFED)
			return;

		onkeyEventMap->clear();

		KeySounder* keySounder = nullptr;
		for (int i = 0; i < onKeySounders->size(); i++)
		{
			(*onKeySounders)[i]->SetForceOffKey(true);
			KeyEvent keyEvent;
			keyEvent.isOnKey = false;
			keyEvent.key = (*onKeySounders)[i]->GetOnKey();
			keyEvent.velocity = 127;
			keyEvent.isRealTime = isRealTime;
			(*offkeyEventMap)[keyEvent.key].push_back(keyEvent);
		}
	}

	//执行按键
	KeySounder* VirInstrument::OnKeyExecute(int key, float velocity)
	{
		//录制
		if (channel != nullptr)
			midiTrackRecord->RecordOnKey(key, velocity, channel->GetChannelNum());

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

		return keySounder;
	}

	//执行松开按键
	void VirInstrument::OffKeyExecute(int key, float velocity)
	{
		//录制	
		if (channel != nullptr)
			midiTrackRecord->RecordOffKey(key, velocity, channel->GetChannelNum());

		//
		KeySounder* keySounder = nullptr;
		for (int i = 0; i < onKeySounders->size(); i++)
		{
			//如果在已按键状态表中查到一个对应的key的keySounder，
			//同时还需要判断这个keySounder有没有被请求松开过，如果没有被请求松开过，
			//才可以对应此刻的松开按键
			if ((*onKeySounders)[i]->IsOnningKey(key) && !(*onKeySounders)[i]->IsNeedOffKey()) {
				keySounder = (*onKeySounders)[i];
				break;
			}
		}

		_OffKey(keySounder, velocity);
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


	// 松开指定发音按键
	void VirInstrument::_OffKey(KeySounder* keySounder, float velocity)
	{
		if (keySounder == nullptr)
			return;

		//
		if ((!keySounder->IsOnningKey() &&
			!keySounder->IsHoldInSoundQueue))
			return;

		//如果keySounder发声没有结束，同时又是保持按键状态，
		//将对此keySounder设置一个需要松开按键请求，而不立即松开按键
		//引擎将在合适的时机（发声结束时），真正松开这个按键
		if (!keySounder->IsSoundEnd() &&
			keySounder->IsHoldDownKey() &&
			!keySounder->IsForceOffKey() &&
			!useMonoMode &&
			state != VirInstrumentState::OFFED)
		{
			keySounder->NeedOffKey();
			return;
		}

		KeySounder* lastOnKeySounder = GetLastOnKeyStateSounder();
		if (lastOnKeySounder == nullptr)
			return;

		keySounder->IsHoldInSoundQueue = false;
		keySounder->OffKey(velocity);

		RemoveOnKeyStateSounder(keySounder);

		//单音模式中会保持按下的按键按键历史队列当中，当释放历史队列中最后一个发音按键后
		//倒数第二个保持按键状态的按键将重新发音
		if (useMonoMode && lastOnKeySounder == keySounder)
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


	//生成发声keySounders
	void VirInstrument::CreateKeySounders()
	{
		//printf("乐器%s:onKeyEventCount:%d\n", preset->name.c_str(), onkeyEventMap->size());

		if (!onkeyEventMap->empty())
		{
			KeySounder* keySounder;
			for (auto iter = onkeyEventMap->begin(); iter != onkeyEventMap->end(); ++iter)
			{
				list<KeyEvent>& keyEventList = iter->second;
				list<KeyEvent>::iterator it = keyEventList.begin();
				list<KeyEvent>::iterator end = keyEventList.end();
				for (; it != end; it++)
				{
					KeyEvent& keyEvent = *it;
					keySounder = OnKeyExecute(keyEvent.key, keyEvent.velocity);
					if (keySounder)
						keySounder->SetRealtimeControlType(keyEvent.isRealTime);

					//
					//PrintOnKeyInfo(keyEvent.key, keyEvent.velocity, keyEvent.isRealTime);
				}
			}

			onkeyEventMap->clear();
		}

		//
		if (!offkeyEventMap->empty())
		{
			for (auto iter = offkeyEventMap->begin(); iter != offkeyEventMap->end(); ++iter)
			{
				list<KeyEvent>& keyEventList = iter->second;
				list<KeyEvent>::iterator it = keyEventList.begin();
				list<KeyEvent>::iterator end = keyEventList.end();
				for (; it != end; it++)
				{
					KeyEvent& keyEvent = *it;
					OffKeyExecute(keyEvent.key, keyEvent.velocity);
				}
			}

			offkeyEventMap->clear();
		}
	}

	void VirInstrument::PrintOnKeyInfo(int key, float velocity, bool isRealTime)
	{
		//
		int trackNum = 0;
		Track* track = channel->GetTrack();
		if (track) trackNum = track->GetNum();
		float sec = floor(ventrue->sec * 100) / 100;

		if (isRealTime)
		{
			cout << "时间" << sec
				<< "<<<" << GetPreset()->name.c_str() << ">>> "
				<< "  乐器号" << channel->GetProgramNum()
				<< "  按键" << key
				<< "  通道" << channel->GetChannelNum()
				<< "  实时 " << endl;
		}
		else
		{
			cout << "时间" << sec
				<< "<<<" << GetPreset()->name.c_str() << ">>> "
				<< "  乐器号" << channel->GetProgramNum()
				<< "  按键" << key
				<< "  轨道" << trackNum
				<< "  通道" << channel->GetChannelNum() << endl;
		}
	}

	//为渲染准备所有正在发声的区域
	int VirInstrument::CreateRegionSounderForRender(RegionSounder** totalRegionSounder, int startSaveIdx)
	{
		//平滑过渡效果深度设置值
		if (isFadeEffectDepth)
		{
			float scale = (ventrue->sec - startFadeEffectDepthSec) / 0.5f;
			if (scale >= 1)
			{
				isFadeEffectDepth = false;
				scale = 1;
			}

			SetRegionReverbDepth(startRegionReverbDepth + (dstRegionReverbDepth - startRegionReverbDepth) * scale);
			SetRegionChorusDepth(startRegionChorusDepth + (dstRegionChorusDepth - startRegionChorusDepth) * scale);
		}

		//
		if (canExecuteStateOp && !stateOps->empty())
		{
			StateOp((*stateOps)[0]);

			if ((*stateOps)[0].opType != VirInstrumentStateOpType::REMOVE)
				stateOps->erase(stateOps->begin());
			else
				stateOps->clear();

			canExecuteStateOp = false;
		}

		//
		regionSounderCount = 0;

		if (IsSoundEnd())
			return 0;

		float reverbDepth, chorusDepth;
		float maxReverbDepth = 0, maxChorusDepth = 0;
		int idx = startSaveIdx;
		KeySounderList::reverse_iterator it = keySounders->rbegin();
		KeySounderList::reverse_iterator rend = keySounders->rend();

		//printf("%s发声按键总数:%d\n", preset->name.c_str(), keySounders->size());

		for (; it != rend; it++)
		{
			RegionSounderList* regionSounderList = (*it)->GetRegionSounderList();
			for (int i = regionSounderList->size() - 1; i >= 0; i--)
			{
				RegionSounder* regionSounder = (*regionSounderList)[i];
				if (regionSounder->IsSoundEnd()) {
					continue;
				}

				totalRegionSounder[idx++] = regionSounder;
				regionSounders[regionSounderCount++] = regionSounder;

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

		//
		//
		if (state == VirInstrumentState::OFFING ||
			state == VirInstrumentState::ONING)
		{
			float scale = (ventrue->sec - startGainFadeSec) / totalGainFadeTime;
			if (scale >= 1)
			{
				if (state == VirInstrumentState::OFFING)
					state = VirInstrumentState::OFFED;
				else if (state == VirInstrumentState::ONING)
					state = VirInstrumentState::ONED;

				canExecuteStateOp = true;
				scale = 1;

				if (virInstStateChangedCB != nullptr)
					virInstStateChangedCB(this);

			}

			gain = startGain + (dstGain - startGain) * scale;

		}

		//存在发音时，设置调整混音，和声数值
		if (idx > startSaveIdx)
		{
			startRegionReverbDepth = regionReverbDepth;
			dstRegionReverbDepth = maxReverbDepth;

			startRegionChorusDepth = regionChorusDepth;
			dstRegionChorusDepth = maxChorusDepth;

			startFadeEffectDepthSec = ventrue->sec;

			if (abs(dstRegionReverbDepth - startRegionReverbDepth) < 0.001 &&
				abs(dstRegionChorusDepth - startRegionChorusDepth) < 0.001)
			{
				isFadeEffectDepth = false;
			}
			else
			{
				isFadeEffectDepth = true;
			}
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

			if (abs(regionReverbDepth - regionReverb->GetEffectMix()) > 0.001f)
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

			if (abs(regionChorusDepth - regionChorus->GetEffectMix()) > 0.001f)
			{
				regionChorus->SetModDepth(1);
				regionChorus->SetModFrequency(0.02f);
				regionChorus->SetEffectMix(regionChorusDepth);
			}
		}
	}

	//合并区域已处理发音样本
	void VirInstrument::CombineRegionSounderSamples(RegionSounder* regionSounder)
	{
		if (IsSoundEnd() || regionSounder->IsSoundEnd() || regionSounder->virInst != this)
			return;

		ChannelOutputMode outputMode = ventrue->GetChannelOutputMode();
		float* regionLeftChannelSamples = regionSounder->GetLeftChannelSamples();
		float* regionRightChannelSamples = regionSounder->GetRightChannelSamples();
		int framePos = ventrue->childFramePos;
		for (int i = 0; i < ventrue->childFrameSampleCount; i++)
		{
			leftChannelSamples[framePos + i] += regionLeftChannelSamples[i] * gain;
			rightChannelSamples[framePos + i] += regionRightChannelSamples[i] * gain;
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
			if (IsSoundEnd() || keySounder->IsSoundEnd())
			{
				if (keySounder->IsOnningKey())
				{
					//如果此时keySounder需要松开按键 且不是单音模式
					//意味着如果有踏板保持控制，说明踏板保持是关闭的，此时需要松开这个按键
					if (state == VirInstrumentState::OFFED ||
						(keySounder->IsNeedOffKey() && !useMonoMode))
					{
						offKeySounder[offIdx++] = keySounder;
					}

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

		//处理需要松开的按键
		for (int i = 0; i < offIdx; i++)
		{
			_OffKey(offKeySounder[i], 127);
		}

	}

}
