#ifndef _Envelope_h_
#define _Envelope_h_

#include "TauTypes.h"


namespace tau
{
	// 包络类型
	enum class EnvelopeType
	{
		//调制
		Mod,
		//音量
		Vol
	};


	// 包络线阶段
	//by cymheart, 2020--2021.
	enum class EnvStage:int
	{

		// 停止   
		Stop = -1,


		// 延迟  
		//delay最小值如果没有设置必须默认为0，不然会有断音现象
		Delay,

		// 起音 
		Attack,

		// 保持 
		Hold,

		// 衰减
		Decay,

		// 延音  
		Sustain,

		// 释音 
		Release,

		//数量
		Count
	};

	// 包络线
	//by cymheart, 2020--2025.
	class Envelope
	{
	public:
		Envelope(EnvelopeType envType = EnvelopeType::Vol);
		~Envelope();

		virtual void Clear();

		//设置包络类型
		void SetType(EnvelopeType envType)
		{
			type = envType;
		}

		// 启动
		void OnKey(int noteKey, float sec);

		// 松开按键
		// <param name="sec">松开按键的时间点，秒</param>
		// <param name="resetReleaseSec">重设按键释放时长，-1为不重新设置</param>
		void OffKey(float sec, float resetReleaseSec = -1);

		//包络线时间线是否停止
		inline bool IsStop()
		{
			return stage == EnvStage::Stop;
		}

	
		// 设置基准秒
		void SetBaseSec(float sec)
		{
			baseSec = sec;
		}

		// 获取基准秒
		float GetBaseSec()
		{
			return baseSec;
		}

		// 获取时间点
		float GetCurtSec()
		{
			return curtSec;
		}

		// 获取当前值
		float GetCurtValue()
		{
			return curtValue;
		}

		// 获取包络所处阶段
		EnvStage GetStage()
		{
			return stage;
		}

		// 根据时间点获取包络线的值
		// <param name="sec">秒</param>
		float GetEnvValue(float sec);

		// 生成包络线
		void Create();


		// 重设包络线
		// 并根据给定的当前时间点，设置包络线所处阶段  
		// <param name="sec">时间点</param>
		void Reset(float sec = -1);

	private: 
		void CalRealHoldSec();
		void CalRealDecaySec();

	public:
		//包络类型
		EnvelopeType type = EnvelopeType::Vol;

		// 延迟时长
		// delaySec最小值如果没有设置必须默认为0，不然会有断音现象
		float delaySec = 0;

		// 起音时长
		float attackSec = 0.001f;

		// 延音所在位置：
		// vol: [0, -100dB]
		// mod: [1, 0]
		float sustain = 0;

		// 释音时长
		float releaseSec = 0.001f;

		//
		short keyToHold = 0;
		short keyToDecay = 0;
		// 保持时长(Timecents)
		short holdTimecents = -12000;
		// 衰减时长(Timecents)
		short decayTimecents = -12000;


	protected:

		EnvStage stage = EnvStage::Stop;

		// 被启动的时间点
		float openSec = 0;

		// 是否按下按键
		bool onKey = false;

		// 当前按键号
		int noteKeyNum = 60;

		// 基时间点,这个时间点表示包络从那个阶段起始计算
		// 正常baseSec为0，表示包络从delay阶段开始计算
		float baseSec = 0;

		// 当前时间点
		float curtSec = 0;

		// 原始释音时长
		float orgReleaseSec = -1;

		float realDecaySec = 0;
		float realHoldSec = 0;

		//当前点在包络线上原始点值(单位:dB)
		float curtValue = 0;

		float oldSec = -1;
		float oldOutput = 0;

		//起音结束时间点
		float attackEndSec = 0; 
		//保持结束时间点
		float holdEndSec = 0;
		//衰减结束时间点
		float decayEndSec = 0;
		//释放开始时间点
		float releaseStartSec = 0;
		//释放结束时间点
		float releaseEndSec = 0;

		//释放阶段包络最大值(单位:dB)
		float releaseMaxEnvValue = 0;

		//
		float sustainMax = 0;
		float sustainMin = 0;

	};
}

#endif
