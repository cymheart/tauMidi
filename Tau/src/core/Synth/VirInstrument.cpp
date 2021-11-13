#include"Synther/Synther.h"
#include"VirInstrument.h"
#include"KeySounder.h"
#include"RegionSounder.h"
#include"Tau.h"
#include"Channel.h"
#include"Preset.h"
#include <random>
using namespace tauFX;


namespace tau
{
	VirInstrument::VirInstrument(Synther* synther, Channel* channel, Preset* preset)
	{
		this->synther = synther;
		this->tau = synther->tau;
		this->channel = channel;
		this->preset = preset;

		channel->SetVirInstrument(this);

		keySounders = new KeySounderList;
		onKeySounders = new vector<KeySounder*>;
		keyEvents = new vector<KeyEvent>;
		onKeySecHistorys = new list<float>;
		stateOps = new queue<VirInstrumentStateOp>;

		leftChannelSamples = new float[tau->frameSampleCount];
		rightChannelSamples = new float[tau->frameSampleCount];
		ClearChannelSamples();


		//
		SetEnableEffects(synther->isEnableVirInstEffects);

		//
		effects = new EffectList();
		effects->Set(leftChannelSamples, rightChannelSamples, tau->frameSampleCount);

		if (tau->GetChannelOutputMode() == ChannelOutputMode::Mono) {
			Delay* delay = new Delay();
			effects->AppendEffect(delay);
		}

		//
		innerEffects = new EffectList();
		innerEffects->Set(leftChannelSamples, rightChannelSamples, tau->frameSampleCount);

		//
		midiTrackRecord = new MidiTrackRecord();
	}

	VirInstrument::~VirInstrument()
	{
		DEL(effects);
		DEL(innerEffects);
		DEL(midiTrackRecord);

		if (!FindKeySounderFromKeySounders(lastKeySounder))
			DEL(lastKeySounder);

		DEL_OBJS_LIST(keySounders, KeySounderList);
		DEL(onKeySounders);
		DEL(keyEvents);
		DEL(onKeySecHistorys);
		DEL(stateOps);

		DEL(leftChannelSamples);
		DEL(rightChannelSamples);

		channel = nullptr;
		preset = nullptr;
	}

	//清除发音数据
	void VirInstrument::ClearSoundDatas()
	{
		if (!FindKeySounderFromKeySounders(lastKeySounder))
			DEL(lastKeySounder);

		if (keySounders != nullptr)
		{
			KeySounderList::iterator it = keySounders->begin();
			KeySounderList::iterator end = keySounders->end();
			for (; it != end; it++)
				DEL(*it);

			keySounders->clear();
		}

		if (onKeySounders)
			onKeySounders->clear();

		if (keyEvents)
			keyEvents->clear();

		if (onKeySecHistorys)
			onKeySecHistorys->clear();

		if (stateOps)
		{
			queue<VirInstrumentStateOp> empty;
			swap(empty, *stateOps);
		}

		isSoundEnd = true;

		fadeReverbDepthInfo.isFadeDepth = false;
		fadeReverbDepthInfo.curtDepth = 0;

		onKeySpeed = 0;
		isRemove = false;
		state = VirInstrumentState::ONED;
		canExecuteStateOp = false;
		gain = 1;
		startGain = 0;
		dstGain = 0;
		startGainFadeSec = 0;
		totalGainFadeTime = 0;

		ClearChannelSamples();
		memset(regionSounders, 0, sizeof(RegionSounder*) * regionSounderCount);
		lastKeySounder = nullptr;
		regionSounderCount = 0;

	}


	//打开乐器
	void VirInstrument::On(bool isFade)
	{
		if (stateOps->empty())
			canExecuteStateOp = true;
		else if (stateOps->back().opType == VirInstrumentStateOpType::ON ||
			stateOps->back().opType == VirInstrumentStateOpType::REMOVE)
			return;

		VirInstrumentStateOp op = { VirInstrumentStateOpType::ON , isFade };
		stateOps->push(op);

	}

	//关闭乐器
	void VirInstrument::Off(bool isFade)
	{
		if (stateOps->empty())
			canExecuteStateOp = true;
		else if (stateOps->back().opType == VirInstrumentStateOpType::OFF ||
			stateOps->back().opType == VirInstrumentStateOpType::REMOVE)
			return;

		VirInstrumentStateOp op = { VirInstrumentStateOpType::OFF , isFade };
		stateOps->push(op);
	}

	//移除乐器
	void VirInstrument::Remove(bool isFade)
	{
		if (isRemove)
			return;

		isRemove = true;

		if (stateOps->empty())
			canExecuteStateOp = true;
		else if (stateOps->back().opType == VirInstrumentStateOpType::REMOVE)
			return;

		VirInstrumentStateOp op = { VirInstrumentStateOpType::OFF , isFade };
		if (stateOps->empty() || stateOps->back().opType != VirInstrumentStateOpType::OFF)
			stateOps->push(op);

		op = { VirInstrumentStateOpType::REMOVE , isFade };
		stateOps->push(op);
	}

	//状态操作
	void VirInstrument::StateOp()
	{
		VirInstrumentStateOp op = stateOps->front();

		switch (op.opType)
		{
		case VirInstrumentStateOpType::ON:
			OnExecute(op.isFade);
			break;
		case VirInstrumentStateOpType::OFF:
			OffExecute(op.isFade);
			break;
		case VirInstrumentStateOpType::REMOVE:
			synther->DelVirInstrumentTask(this);
			break;
		default:
			break;
		}

		stateOps->pop();

		if (op.opType == VirInstrumentStateOpType::REMOVE)
		{
			queue<VirInstrumentStateOp> empty;
			swap(empty, *stateOps);
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
		startGainFadeSec = synther->sec;
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
		startGainFadeSec = synther->sec;
		totalGainFadeTime = 0.2f;

		//
		if (virInstStateChangedCB != nullptr)
			virInstStateChangedCB(this);
	}



	//增加效果器
	void VirInstrument::AddEffect(TauEffect* effect)
	{
		effect->SetSynther(synther);
		effects->AppendEffect(effect);
	}


	//清除通道样本缓存
	void VirInstrument::ClearChannelSamples()
	{
		memset(leftChannelSamples, 0, sizeof(float) * tau->frameSampleCount);
		memset(rightChannelSamples, 0, sizeof(float) * tau->frameSampleCount);
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

	//调制生成器参数
	void VirInstrument::ModulationParams(int key)
	{
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			KeySounder& keySounder = *(*it);
			if (keySounder.GetOnKey() != key)
				continue;
			keySounder.ModulationParams();
		}
	}


	//调制输入按键生成器参数
	void VirInstrument::ModulationInputKeyParams()
	{
		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			KeySounder& keySounder = *(*it);
			keySounder.ModulationParams();
		}
	}


	// 录制为midi
	void VirInstrument::RecordMidi()
	{
		if (!isEnableRecordFunction || midiTrackRecord->IsRecord())
			return;

		midiTrackRecord->Stop();
		midiTrackRecord->Clear();
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
	MidiTrack* VirInstrument::TakeMidiTrack(float baseTickForQuarterNote, vector<RecordTempo>* tempos)
	{
		return midiTrackRecord->TakeMidiTrack(baseTickForQuarterNote, *tempos);
	}


	//所有区域发音是否结束
	//这个检测不能检测到效果器作用是否结束
	bool VirInstrument::IsAllKeySoundEnd()
	{
		if (state == VirInstrumentState::OFFED)
			return true;

		KeySounderList::iterator it = keySounders->begin();
		KeySounderList::iterator end = keySounders->end();
		for (; it != end; it++)
		{
			if (!(*it)->IsSoundEnd())
				return false;
		}
		return true;
	}

	//bend
	void VirInstrument::SetPitchBend(int value)
	{
		if (channel == nullptr)
			return;

		channel->SetPitchBend(value);
		ModulationParams();

		//录制
		if (isEnableRecordFunction && midiTrackRecord)
			midiTrackRecord->RecordSetPitchBend(value, channel->GetChannelNum());
	}

	//PolyPressure
	void VirInstrument::SetPolyPressure(int key, int pressure)
	{
		if (channel == nullptr)
			return;

		channel->SetPolyPressure(pressure);
		ModulationParams(key);

		//录制
		if (isEnableRecordFunction && midiTrackRecord)
			midiTrackRecord->RecordSetPolyPressure(key, pressure, channel->GetChannelNum());
	}


	//改变乐器
	void VirInstrument::ChangeProgram(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		int key = synther->GetReplaceInstrumentKey(bankSelectMSB, bankSelectLSB, instrumentNum);
		Preset* newPreset = tau->GetInstrumentPreset(key);

		if (newPreset == nullptr)
		{
			newPreset = tau->GetInstrumentPreset(0, 0, instrumentNum);
			if (newPreset == nullptr)
				return;
		}

		if (newPreset == preset)
			return;

		preset = newPreset;
		channel->SelectProgram(preset->bankSelectMSB, preset->bankSelectLSB, preset->instrumentNum);

		//录制
		if (isEnableRecordFunction && midiTrackRecord) {
			midiTrackRecord->RecordSetController(MidiControllerType::BankSelectMSB, preset->bankSelectMSB, channel->GetChannelNum());
			midiTrackRecord->RecordSetController(MidiControllerType::BankSelectLSB, preset->bankSelectLSB, channel->GetChannelNum());
			midiTrackRecord->RecordSetProgramNum(preset->instrumentNum, channel->GetChannelNum());
		}
	}


	//设置乐器号
	void VirInstrument::SetProgramNum(int num)
	{
		if (channel == nullptr)
			return;

		int msb = channel->GetBankSelectMSB();
		int lsb = channel->GetBankSelectLSB();

		int key = synther->GetReplaceInstrumentKey(msb, lsb, num);
		Preset* newPreset = tau->GetInstrumentPreset(key);
		if (newPreset == nullptr)
		{
			newPreset = tau->GetInstrumentPreset(0, 0, num);
			if (newPreset == nullptr)
				return;
		}

		if (newPreset == preset)
			return;

		preset = newPreset;
		channel->SetProgramNum(num);

		//录制
		if (isEnableRecordFunction && midiTrackRecord)
			midiTrackRecord->RecordSetProgramNum(num, channel->GetChannelNum());
	}

	//设置控制器
	void VirInstrument::SetController(MidiControllerType ctrlType, int value)
	{
		if (channel == nullptr)
			return;

		channel->SetControllerValue(ctrlType, value);
		ModulationParams();

		//录制
		if (isEnableRecordFunction && midiTrackRecord)
			midiTrackRecord->RecordSetController(ctrlType, value, channel->GetChannelNum());
	}


	//计算按键速度
	void VirInstrument::ComputeOnKeySpeed()
	{
		onKeySecHistorys->push_back(synther->sec);
		float tm = 0;
		int idx = 0;
		float size = (float)onKeySecHistorys->size();

		list<float>::iterator it = onKeySecHistorys->begin();
		list<float>::iterator end = onKeySecHistorys->end();
		for (; it != end; it++, idx++)
		{
			tm = synther->sec - *it;
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
		if (synther->totalRegionSounderCount > tau->limitRegionSounderCount * 0.5f)
		{
			if (!isRealTime && tickCount <= 5)
			{
				//录制	
				if (isEnableRecordFunction && channel != nullptr)
					midiTrackRecord->RecordOffKey(key, velocity, channel->GetChannelNum());

				return true;
			}

			//计算按键速度
			ComputeOnKeySpeed();

			if (onKeySpeed > tau->limitOnKeySpeed)
			{
				//录制	
				if (isEnableRecordFunction && channel != nullptr)
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
	void VirInstrument::OnKey(int key, float velocity, int tickCount)
	{
		if (state == VirInstrumentState::OFFED)
			return;

		//判断是否可以忽略按键
		if (CanIgroneOnKey(key, velocity, tickCount, isRealtime))
			return;


		//如果是实时乐器，按键将不会重复按下，先前按下的按键将会被释放
		if (isRealtime)
		{
			for (int i = 0; i < onKeySounders->size(); i++)
			{
				if ((*onKeySounders)[i]->GetOnKey() == key)
					OffKey(key, 127);
			}
		}

		//
		KeyEvent keyEvent;
		keyEvent.isOnKey = true;
		keyEvent.key = key;
		keyEvent.velocity = velocity;
		keyEvent.isRealTime = isRealtime;
		keyEvents->push_back(keyEvent);
	}

	//松开按键
	void VirInstrument::OffKey(int key, float velocity)
	{
		if (state == VirInstrumentState::OFFED)
			return;

		KeyEvent keyEvent;
		keyEvent.isOnKey = false;
		keyEvent.key = key;
		keyEvent.velocity = velocity;
		keyEvent.isRealTime = isRealtime;
		keyEvents->push_back(keyEvent);
	}



	//松开所有按键
	void VirInstrument::OffAllKeys()
	{
		if (state == VirInstrumentState::OFFED)
			return;

		keyEvents->clear();

		KeySounder* keySounder = nullptr;
		for (int i = 0; i < onKeySounders->size(); i++)
		{
			(*onKeySounders)[i]->SetForceOffKey(true);
			KeyEvent keyEvent;
			keyEvent.isOnKey = false;
			keyEvent.key = (*onKeySounders)[i]->GetOnKey();
			keyEvent.velocity = 127;
			keyEvent.isRealTime = isRealtime;
			keyEvents->push_back(keyEvent);
		}
	}

	//结束所有按键发音
	void VirInstrument::EndOnKeySounds()
	{
		KeySounder* keySounder = nullptr;
		for (int i = 0; i < onKeySounders->size(); i++)
		{
			(*onKeySounders)[i]->EndSound();
		}
	}

	//执行按键
	KeySounder* VirInstrument::OnKeyExecute(int key, float velocity)
	{
		//录制
		if (isEnableRecordFunction && channel != nullptr)
			midiTrackRecord->RecordOnKey(key, velocity, channel->GetChannelNum());

		KeySounder* keySounder = KeySounder::New();
		_OnKey(keySounder, key, velocity);

		//
		channel->SetNoteOnKey(key, velocity);
		if (tau->UseCommonModulator())
			ModulationInputKeyParams();

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
		if (isEnableRecordFunction && channel != nullptr)
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

	//从正在发音的KeySounders中查找KeySounder
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
		KeySounder* keySounder;
		for (int i = 0; i < keyEvents->size(); i++)
		{
			KeyEvent& keyEvent = (*keyEvents)[i];
			if (keyEvent.isOnKey)
			{
				keySounder = OnKeyExecute(keyEvent.key, keyEvent.velocity);
				if (keySounder)
					keySounder->SetRealtimeControlType(keyEvent.isRealTime);
				//
				PrintOnKeyInfo(keyEvent.key, keyEvent.velocity, keyEvent.isRealTime);
			}
			else
			{
				bool isFindSameOnKey = false;
				if (isRealtime)
				{
					for (int j = i - 1; j >= 0; j--)
					{
						if ((*keyEvents)[i].key == keyEvent.key && (*keyEvents)[i].isOnKey)
						{
							isFindSameOnKey = true;
							break;
						}
					}
				}

				//当在同一批任务实时按键事件中，发现了松开事件，将对松开按键事件延迟10ms执行，以使实时按键可以发音
				if (!isFindSameOnKey)
					OffKeyExecute(keyEvent.key, keyEvent.velocity);
				else
					synther->tau->OffKey(keyEvent.key, keyEvent.velocity, this, 10);
			}
		}


		keyEvents->clear();
	}

	void VirInstrument::PrintOnKeyInfo(int key, float velocity, bool isRealTime)
	{
		//
		int trackNum = 0;
		float sec = floor(synther->sec * 100) / 100;

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

		if (isEnableInnerEffects)
		{
			//平滑过渡混音深度设置值
			FadeReverbDepth();
		}

		//
		if (canExecuteStateOp && !stateOps->empty())
		{
			StateOp();
			canExecuteStateOp = false;
		}


		if (state == VirInstrumentState::OFFING ||
			state == VirInstrumentState::ONING)
		{
			float scale = (synther->sec - startGainFadeSec) / totalGainFadeTime;
			if (scale >= 1)
			{
				if (state == VirInstrumentState::OFFING)
					state = VirInstrumentState::OFFED;
				else if (state == VirInstrumentState::ONING)
					state = VirInstrumentState::ONED;

				scale = 1;
				//canExecuteStateOp = true;

				if (virInstStateChangedCB != nullptr)
					virInstStateChangedCB(this);
			}

			gain = startGain + (dstGain - startGain) * scale;
		}

		//
		regionSounderCount = 0;
		if (IsSoundEnd()) {
			canExecuteStateOp = true;
			return 0;
		}

		float reverbDepth;
		curtReverbDepth = 0;
		int idx = startSaveIdx;
		KeySounderList::reverse_iterator it = keySounders->rbegin();
		KeySounderList::reverse_iterator rend = keySounders->rend();

		//printf("%s发声按键总数:%d\n", preset->name.c_str(), keySounders->size());

		for (; it != rend; it++)
		{
			RegionSounderList* regionSounderList = (*it)->GetRegionSounderList();
			for (int i = (int)(regionSounderList->size() - 1); i >= 0; i--)
			{
				RegionSounder* regionSounder = (*regionSounderList)[i];
				if (regionSounder->IsSoundEnd()) {
					continue;
				}

				totalRegionSounder[idx++] = regionSounder;
				regionSounders[regionSounderCount++] = regionSounder;

				//获取区域混音深度数值
				reverbDepth = regionSounder->GetGenReverbEffectsSend() * 0.01f;
				if (reverbDepth > curtReverbDepth)
					curtReverbDepth = reverbDepth;

			}
		}


		//存在发音时，设置调整混音，和声数值
		if (idx > startSaveIdx && isEnableInnerEffects)
		{
			SettingFadeEffectDepthInfo(curtReverbDepth, fadeReverbDepthInfo);
		}

		return idx - startSaveIdx;
	}


	//设置过渡效果深度信息
	void VirInstrument::SettingFadeEffectDepthInfo(float curtEffectDepth, FadeEffectDepthInfo& fadeEffectDepthInfo)
	{
		if (abs(curtEffectDepth - fadeEffectDepthInfo.dstDepth) > 0.001)
		{
			fadeEffectDepthInfo.startDepth = fadeEffectDepthInfo.curtDepth;
			fadeEffectDepthInfo.dstDepth = curtEffectDepth;

			if (abs(fadeEffectDepthInfo.dstDepth - fadeEffectDepthInfo.startDepth) < 0.001)
				fadeEffectDepthInfo.isFadeDepth = false;
			else {
				fadeEffectDepthInfo.isFadeDepth = true;
				fadeEffectDepthInfo.startFadeSec = synther->sec;
			}
		}
	}

	//过渡混音深度
	void VirInstrument::FadeReverbDepth()
	{
		if (!fadeReverbDepthInfo.isFadeDepth)
			return;

		float scale = (synther->sec - fadeReverbDepthInfo.startFadeSec) / 0.1f;
		if (scale >= 1) {
			fadeReverbDepthInfo.isFadeDepth = false;
			scale = 1;
		}

		SetRegionReverbDepth(fadeReverbDepthInfo.startDepth + (fadeReverbDepthInfo.dstDepth - fadeReverbDepthInfo.startDepth) * scale);
	}


	//设置区域混音深度
	void VirInstrument::SetRegionReverbDepth(float value)
	{
		fadeReverbDepthInfo.curtDepth = value;
		if (regionReverb != nullptr)
			regionReverb->SetEnable(false);

		if (fadeReverbDepthInfo.curtDepth != 0)
		{
			if (!regionReverb)
			{
				regionReverb = new Reverb();
				innerEffects->AppendEffect(regionReverb);
			}

			regionReverb->SetEnable(true);

			if (abs(fadeReverbDepthInfo.curtDepth - regionReverb->GetEffectMix()) > 0.001f)
			{
				regionReverb->SetRoomSize(0.8f);
				regionReverb->SetWidth(0.5f);
				regionReverb->SetDamping(0.3f);
				regionReverb->SetEffectMix(fadeReverbDepthInfo.curtDepth);
			}
		}
	}


	//合并区域已处理发音样本
	void VirInstrument::CombineRegionSounderSamples(RegionSounder* regionSounder)
	{
		if (IsSoundEnd() || regionSounder->IsSoundEnd() || regionSounder->virInst != this)
			return;

		ChannelOutputMode outputMode = tau->GetChannelOutputMode();
		float* regionLeftChannelSamples = regionSounder->GetLeftChannelSamples();
		float* regionRightChannelSamples = regionSounder->GetRightChannelSamples();
		int framePos = synther->childFramePos;
		for (int i = 0; i < tau->childFrameSampleCount; i++)
		{
			leftChannelSamples[framePos + i] += regionLeftChannelSamples[i] * gain;
			rightChannelSamples[framePos + i] += regionRightChannelSamples[i] * gain;
		}

		//当区域按键时间超过0.1s后（有的发音样本起始音会很长时间处于0值，所以需要一个延迟时间再判断）
		//此时处理不能听到发声的滞留区域，使其结束发音
		KeySounder* keySounder = regionSounder->GetKeySounder();
		if (synther->sec - keySounder->GetOnKeySec() > 0.1f)
			regionSounder->EndBlockSound();

	}


	//应用效果器到乐器的声道buffer
	void VirInstrument::ApplyEffectsToChannelBuffer()
	{
		if (isEnableInnerEffects)
			innerEffects->Process();

		if (isEnableEffect)
			effects->Process();

		//检测发音是否结束
		isSoundEnd = false;
		if (IsAllKeySoundEnd())
		{
			//检测由效果器带来的尾音是否结束
			int offset = (int)(tau->frameSampleCount * 0.02f);
			for (int i = 0; i < tau->frameSampleCount; i += offset)
			{
				//此处值需要非常小，不然会产生杂音
				if (fabsf(leftChannelSamples[i]) > 0.001f ||
					fabsf(rightChannelSamples[i]) > 0.001f)
					return;
			}

			isSoundEnd = true;
			canExecuteStateOp = true;
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
