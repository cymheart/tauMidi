

#ifndef _KeySounder_h_
#define _KeySounder_h_

#include "VentrueTypes.h"
#include "Midi/MidiTypes.h"

namespace ventrue
{
	/*
	* 按键发音器
	* 按键发声器是区域发声器的集合，一个按键被按下后，可能会有多个区域响应这个按键，来处理同时发声
	* by cymheart, 2020--2021.
	*
	*/
	class KeySounder
	{
	public:
		KeySounder();
		~KeySounder();

		static KeySounder* New();

		void Clear();
		void Release();

		inline KeySounderID GetID()
		{
			return id;
		}

		inline void SetID(KeySounderID id)
		{
			this->id = id;
		}

		//获取key所在的虚拟乐器
		VirInstrument* GetVirInstrument() { return virInst; }

		// 按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst);

		// 松开按键       
		int OffKey(float velocity = 127.0f);

		// 需要松开按键
		void NeedOffKey();

		// 生成当前按键区域的独占类表   
		// exclusiveClassList : 一个包含非0的数字，遇到-1为结尾
		void CreateExclusiveClassList(int32_t* exclusiveClassList);

		// 停止对持有相同独占类的RegionSounder的处理     
		void StopExclusiveClassRegionSounderProcess(int exclusiveClass);

		// 内部相关的所有区域发声处理是否结束
		// 包括了采样处理结束，和效果音残余处理结束   
		bool IsSoundEnd();


		// 判断是否在指定的虚拟乐器上按下了指定的键
		// <param name="key">指定键值</param>
		inline bool IsOnningKey(int key)
		{
			return key == downKey && isOnningKey ? true : false;
		}

		// 判断是否按键中  
		inline bool IsOnningKey()
		{
			return isOnningKey;
		}

		// 判断是否需要释放按键  
		inline bool IsNeedOffKey()
		{
			return isNeedOffKey;
		}


		//是否保持按键状态
		bool IsHoldDownKey();


		// 获取按键
		inline int GetOnKey()
		{
			return downKey;
		}

		// 获取按键按下时间点
		inline float GetOnKeySec()
		{
			return downKeySec;
		}

		// 获取发声结束时间点
		inline float GetSoundEndSec()
		{
			return soundEndSec;
		}

		// 获取力度   
		inline float GetVelocity()
		{
			return velocity;
		}

		// 获取所有的发声区域     
		inline RegionSounderList* GetRegionSounderList()
		{
			return regionSounderList;
		}

		// 是否具有发声区域    
		bool IsHavRegionSounder();

		//设置是否为实时控制类型
		void SetRealtimeControlType(bool isRealtimeControl);

		//调制生成器参数     
		void ModulationParams();




	private:

		// 根据给定的按键，在预设区域中找到所有对应的乐器区域，并存入乐器区域激活列表 
		void CreateActiveRegionSounderList();

		RegionSounder* CreateRegionSounder(
			Sample* sample,
			Region* activeInstRegion, Region* activeInstGlobalRegion,
			Region* activePresetRegion, Region* activePresetGlobalRegion);

	public:
		Ventrue* ventrue = nullptr;

		// 是否保持在发声队列中(不管是否发声结束)
		bool IsHoldInSoundQueue = false;

	private:
		VirInstrument* virInst = nullptr;

		KeySounderID id = 0;
		int downKey = 0;
		float downKeySec = 0;
		float soundEndSec = 0;
		float velocity = 0;

		//是否正在按键
		bool isOnningKey = false;

		//是否需要松开按键
		bool isNeedOffKey = false;

		//发声是否结束
		bool isSoundEnd = false;



		RegionSounderList* regionSounderList = nullptr;

		//
		SamplesLinkToInstRegionInfo activeInstRegionLinkInfos[512];
	};
}

#endif
