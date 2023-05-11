#include"KeySounder.h"
#include"ZoneSounder.h"
#include"Preset.h"
#include"Instrument.h"
#include"Tau.h"
#include"VirInstrument.h"
#include"Synther/Synther.h"
#include"TauPool.h"

namespace tau
{
	KeySounder::KeySounder()
	{
	}

	KeySounder::~KeySounder()
	{
		for (int i = 0; i < zoneSounders.size(); i++)
			delete zoneSounders[i];
	}

	KeySounder* KeySounder::New()
	{
		KeySounder* keySounder = TauPool::GetInstance().KeySounderPool().Pop();
		keySounder->Clear();
		return keySounder;
	}

	void KeySounder::Clear()
	{
		downKey = 0;
		IsHoldInSoundQueue = false;
		isOnningKey = false;
		isNeedOffKey = false;
		isSoundEnd = false;
		virInst = nullptr;
		synther = nullptr;
		isForceOffKey = false;
	}

	void KeySounder::Release()
	{
		for (int i = 0; i < zoneSounders.size(); i++)
			zoneSounders[i]->Release();
		zoneSounders.clear();
		TauPool::GetInstance().KeySounderPool().Push(this);
	}

	//按下按键
	void KeySounder::OnKey(int key, float velocity, VirInstrument* virInst)
	{
		this->virInst = virInst;
		synther = virInst->synther;
		isOnningKey = true;
		downKey = key;
		downKeySec = synther->GetCurtSec();
		this->velocity = velocity;
		CreateActiveZoneSounders();

		for (int i = 0; i < zoneSounders.size(); i++)
			zoneSounders[i]->OnKey(key, velocity);

	}

	// 松开按键
	int KeySounder::OffKey(float velocity)
	{
		if (isOnningKey == false)
			return downKey;

		for (int i = 0; i < zoneSounders.size(); i++)
			zoneSounders[i]->OffKey(velocity);

		isOnningKey = false;
		return downKey;
	}

	// 需要松开按键
	void KeySounder::NeedOffKey()
	{
		isNeedOffKey = true;
		for (int i = 0; i < zoneSounders.size(); i++)
			zoneSounders[i]->NeedOffKey();
	}

	// 生成当前按键区域的独占类表
	// <returns>exclusiveClassList : 一个包含非0的数字，遇到-1为结尾</returns>
	void KeySounder::CreateExclusiveClassList(int32_t* exclusiveClassList)
	{
		int32_t idx = 0;
		int32_t exclusiveClass;
		bool isExist;

		for (int i = 0; i < zoneSounders.size(); i++)
		{
			isExist = false;
			exclusiveClass = zoneSounders[i]->GetGenExclusiveClass().amount;

			for (int j = 0; j < idx; j++)
			{
				if (exclusiveClassList[j] == exclusiveClass)
				{
					isExist = true;
					break;
				}
			}

			if (!isExist && exclusiveClass != 0)
			{
				exclusiveClassList[idx++] = exclusiveClass;
			}
		}

		exclusiveClassList[idx] = -1;
	}

	// 停止对持有相同独占类的ZoneSounder的处理
	void KeySounder::StopExclusiveClassZoneSounderProcess(int exclusiveClass)
	{
		for (int i = 0; i < zoneSounders.size(); i++)
		{
			if (zoneSounders[i]->IsSoundEnd() == true)
				continue;

			int val = zoneSounders[i]->GetGenExclusiveClass().amount;
			if (val == exclusiveClass)
				zoneSounders[i]->EndSound();
		}
	}

	// 内部相关的所有区域发声处理是否结束
	// 包括了采样处理结束，和效果音残余处理结束
	bool KeySounder::IsSoundEnd()
	{
		if (isSoundEnd)
			return true;

		for (int i = 0; i < zoneSounders.size(); i++)
		{
			if (!zoneSounders[i]->IsSoundEnd())
				return false;
		}

		isSoundEnd = true;
		soundEndSec = synther->GetCurtSec();
		return true;
	}

	void KeySounder::EndSound()
	{
		if (isSoundEnd)
			return;

		for (int i = 0; i < zoneSounders.size(); i++)
			zoneSounders[i]->EndSound();
	
		isSoundEnd = true;
		soundEndSec = synther->GetCurtSec();
	}


	// 是否具有发声区域
	bool KeySounder::IsHavZoneSounder()
	{
		return !zoneSounders.empty();
	}

	// 是否保持按键状态
	bool KeySounder::IsHoldDownKey()
	{
		for (int i = 0; i < zoneSounders.size(); i++) {
			if (zoneSounders[i]->IsSoundEnd())
				continue;

			return zoneSounders[i]->IsHoldDownKey();
		}

		return false;
	}

	//设置是否为实时控制类型
	void KeySounder::SetRealtimeControlType(bool isRealtimeControl)
	{
		for (int i = 0; i < zoneSounders.size(); i++)
			zoneSounders[i]->isRealtimeControl = isRealtimeControl;
	}

	//调制生成器
	void KeySounder::Modulation()
	{
		for (int i = 0; i < zoneSounders.size(); i++)
		{
			if (!zoneSounders[i]->IsSoundEnd())
				zoneSounders[i]->Modulation();
		}
	}


	// 根据给定的按键，在预设区域中找到所有对应的乐器区域，并存入乐器区域激活列表
	void KeySounder::CreateActiveZoneSounders()
	{
		Preset* preset = virInst->GetPreset();
		if (preset == nullptr)
			return;

		Zone* presetGlobalZone = preset->GetGlobalZone();
		vector<InstLinkToPresetZoneInfo>& presetZoneLinkInfos = preset->GetPresetZoneLinkInfos();
		Zone* presetZone;
		Instrument* inst;
		Zone* instZone;
		Zone* instGlobalZone;
		ZoneSounder* zoneSounder;
		size_t size = presetZoneLinkInfos.size();
		GeneratorAmount keyRange;
		GeneratorAmount velRange;

		for (int i = 0; i < size; i++)
		{
			presetZone = presetZoneLinkInfos[i].Zone;
			GeneratorList& presetGenList = presetZone->GetGens();
			GeneratorList& presetGlobalGenList = presetGlobalZone->GetGens();
			if (!presetGenList.IsEmpty(GeneratorType::KeyRange))
				keyRange = presetGenList.GetAmount(GeneratorType::KeyRange);
			else
				keyRange = presetGlobalGenList.GetAmount(GeneratorType::KeyRange);

			//
			if (!presetGenList.IsEmpty(GeneratorType::VelRange))
				velRange = presetGenList.GetAmount(GeneratorType::VelRange);
			else
				velRange = presetGlobalGenList.GetAmount(GeneratorType::VelRange);


			if (downKey >= keyRange.rangeData.low && downKey <= keyRange.rangeData.high &&
				velocity >= velRange.rangeData.low && velocity <= velRange.rangeData.high)
			{
				inst = presetZoneLinkInfos[i].linkInst;
				instGlobalZone = inst->GetGlobalZone();
				vector<SamplesLinkToInstZoneInfo>& instZoneLinkInfos = inst->GetInstZoneLinkInfos();

				//
				int pos = 0;
				for (int i = 0; i < instZoneLinkInfos.size(); i++)
				{
					instZone = instZoneLinkInfos[i].Zone;
					GeneratorList& instGenList = instZone->GetGens();
					GeneratorList& instGlobalGenList = instGlobalZone->GetGens();
					if (!instGenList.IsEmpty(GeneratorType::KeyRange))
						keyRange = instGenList.GetAmount(GeneratorType::KeyRange);
					else
						keyRange = instGlobalGenList.GetAmount(GeneratorType::KeyRange);
					//
					if (!instGenList.IsEmpty(GeneratorType::VelRange))
						velRange = instGenList.GetAmount(GeneratorType::VelRange);
					else
						velRange = instGlobalGenList.GetAmount(GeneratorType::VelRange);

					//
					if (downKey >= keyRange.rangeData.low && downKey <= keyRange.rangeData.high &&
						velocity >= velRange.rangeData.low && velocity <= velRange.rangeData.high)
					{
						activeInstZoneLinkInfos[pos++] = instZoneLinkInfos[i];
					}
				}

				//
				for (int j = 0; j < pos; j++)
				{
					zoneSounder = ZoneSounder::New();
					zoneSounder->synther = synther;
					zoneSounder->tau = synther->tau;
					zoneSounder->virInst = virInst;
					zoneSounder->keySounder = this;
					zoneSounder->instZone = activeInstZoneLinkInfos[j].Zone;
					zoneSounder->instGlobalZone = inst->GetGlobalZone();
					zoneSounder->presetZone = presetZoneLinkInfos[i].Zone;
					zoneSounder->presetGlobalZone = preset->GetGlobalZone();
					zoneSounder->SetSample(activeInstZoneLinkInfos[j].linkSample);
					zoneSounder->SetSampleGen(activeInstZoneLinkInfos[j].linkSampleGen);
					zoneSounder->Init();
					zoneSounders.push_back(zoneSounder);
				}				
			}
		}
	}

}
