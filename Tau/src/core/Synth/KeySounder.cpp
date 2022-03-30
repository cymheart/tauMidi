#include"KeySounder.h"
#include"RegionSounder.h"
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
		regionSounderList = new RegionSounderList;
	}

	KeySounder::~KeySounder()
	{
		DEL_OBJS_VECTOR(regionSounderList);
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
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			(*regionSounderList)[i]->Release();
		}
		regionSounderList->clear();

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
		CreateActiveRegionSounderList();

		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
			(*regionSounderList)[i]->OnKey(key, velocity);

	}

	// 松开按键
	int KeySounder::OffKey(float velocity)
	{
		if (isOnningKey == false)
			return downKey;

		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
			(*regionSounderList)[i]->OffKey(velocity);

		isOnningKey = false;
		return downKey;
	}

	// 需要松开按键
	void KeySounder::NeedOffKey()
	{
		isNeedOffKey = true;

		//
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
			(*regionSounderList)[i]->NeedOffKey();
	}

	// 生成当前按键区域的独占类表
	// <returns>exclusiveClassList : 一个包含非0的数字，遇到-1为结尾</returns>
	void KeySounder::CreateExclusiveClassList(int32_t* exclusiveClassList)
	{
		int32_t idx = 0;
		int32_t exclusiveClass;
		bool isExist;

		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			isExist = false;
			exclusiveClass = (int)(*regionSounderList)[i]->GetGenExclusiveClass();

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

	// 停止对持有相同独占类的RegionSounder的处理
	void KeySounder::StopExclusiveClassRegionSounderProcess(int exclusiveClass)
	{
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			if ((*regionSounderList)[i]->IsSoundEnd() == true)
				continue;

			int val = (int)(*regionSounderList)[i]->GetGenExclusiveClass();
			if (val == exclusiveClass)
			{
				(*regionSounderList)[i]->EndSound();
			}
		}
	}

	// 内部相关的所有区域发声处理是否结束
	// 包括了采样处理结束，和效果音残余处理结束
	bool KeySounder::IsSoundEnd()
	{
		if (isSoundEnd)
			return true;

		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			if (!(*regionSounderList)[i]->IsSoundEnd())
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

		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			(*regionSounderList)[i]->EndSound();
		}

		isSoundEnd = true;
		soundEndSec = synther->GetCurtSec();
	}


	// 是否具有发声区域
	bool KeySounder::IsHavRegionSounder()
	{
		if (regionSounderList->size() == 0)
			return false;
		return true;
	}

	// 是否保持按键状态
	bool KeySounder::IsHoldDownKey()
	{
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++) {
			if ((*regionSounderList)[i]->IsSoundEnd() == true)
				continue;

			return (*regionSounderList)[i]->IsHoldDownKey();
		}

		return false;
	}

	//设置是否为实时控制类型
	void KeySounder::SetRealtimeControlType(bool isRealtimeControl)
	{
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
			(*regionSounderList)[i]->isRealtimeControl = isRealtimeControl;
	}

	//调制生成器参数
	void KeySounder::ModulationParams()
	{
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			if ((*regionSounderList)[i]->IsSoundEnd())
				continue;

			(*regionSounderList)[i]->ModulationParams();
		}
	}

	//调制输入按键生成器参数
	void KeySounder::ModulationInputKeyParams()
	{
		size_t size = regionSounderList->size();
		for (int i = 0; i < size; i++)
		{
			if ((*regionSounderList)[i]->IsSoundEnd())
				continue;

			(*regionSounderList)[i]->ModulationInputKeyParams();
		}
	}

	// 根据给定的按键，在预设区域中找到所有对应的乐器区域，并存入乐器区域激活列表
	void KeySounder::CreateActiveRegionSounderList()
	{
		Preset* preset = virInst->GetPreset();
		InstLinkToPresetRegionInfoList* presetRegionLinkInfoList = preset->GetPresetRegionLinkInfoList();
		Region* activeInstRegion;
		Instrument* inst;
		Region* presetRegion;
		RegionSounder* regionSounder;
		size_t size = presetRegionLinkInfoList->size();
		RangeFloat keyRange;
		RangeFloat velRange;


		for (int i = 0; i < size; i++)
		{
			presetRegion = (*presetRegionLinkInfoList)[i].region;
			keyRange = presetRegion->GetKeyRange();
			velRange = presetRegion->GetVelRange();

			if (!(downKey >= keyRange.min && downKey <= keyRange.max &&
				velocity >= velRange.min && velocity <= velRange.max))
			{
				continue;
			}

			inst = (*presetRegionLinkInfoList)[i].linkInst;
			int sz = inst->GetHavKeyInstRegionLinkInfos(downKey, velocity, activeInstRegionLinkInfos);

			for (int j = 0; j < sz; j++)
			{
				activeInstRegion = activeInstRegionLinkInfos[j].region;

				regionSounder = CreateRegionSounder(
					activeInstRegionLinkInfos[j].linkSample,
					activeInstRegion, inst->GetGlobalRegion(),
					(*presetRegionLinkInfoList)[i].region, preset->GetGlobalRegion());

				regionSounderList->push_back(regionSounder);
			}
		}
	}

	RegionSounder* KeySounder::CreateRegionSounder(
		Sample* sample,
		Region* activeInstRegion, Region* activeInstGlobalRegion,
		Region* activePresetRegion, Region* activePresetGlobalRegion)
	{
		RegionSounder* regionSounder = RegionSounder::New();
		regionSounder->synther = synther;
		regionSounder->tau = synther->tau;
		regionSounder->virInst = virInst;
		regionSounder->keySounder = this;
		regionSounder->instRegion = activeInstRegion;
		regionSounder->instGlobalRegion = activeInstGlobalRegion;
		regionSounder->presetRegion = activePresetRegion;
		regionSounder->presetGlobalRegion = activePresetGlobalRegion;
		regionSounder->SetSample(sample);
		regionSounder->Init();

		return regionSounder;
	}
}
