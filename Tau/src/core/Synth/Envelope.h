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
	enum class EnvStage
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

	// 阶段范围信息
	struct StageRangeInfo
	{
		// x轴范围
		float xmin = 0, xmax = 0;
		// y轴范围
		float ymin = 0, ymax = 0;
		float xRangeWidth = 0, xRangeWidthInv = 0;
		float yRangeWidth = 0;

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
		void OffKey(float sec, float releaseSec = -1);

		//包络线时间线是否停止
		inline bool IsStop()
		{
			if (curtStage == EnvStage::Stop)
				return true;
			return false;
		}

		// 获取当前包络阶段
		inline EnvStage GetStage()
		{
			return curtStage;
		}

		// 设置阶段
		void SetStage(EnvStage stage)
		{
			curtStage = stage;
		}

		// 设置基准秒
		void SetBaseSec(float baseSec)
		{
			this->baseSec = baseSec;
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

		//  根据给定的时间点，设置包络线所处阶段   
		// <param name="sec">时间点</param>
		void SetCurtStage(float sec);

		void SetStageRangeInfo(StageRangeInfo& range, float xmin, float xmax, float ymin, float ymax);
		float ComputeStageValueY(EnvStage stage, float x);

	public:
		//包络类型
		EnvelopeType type = EnvelopeType::Vol;

		// 幅值
		float amp = 1;

		// 延迟时长
		// delaySec最小值如果没有设置必须默认为0，不然会有断音现象
		float delaySec = 0;

		// 起音时长
		float attackSec = 0.001f;

		// 延音所在y位置：范围[0,1]
		float sustainY = 1;

		// 释音时长
		float releaseSec = 0.001f;

		//
		short keyToHold = 0;
		short keyToDecay = 0;
		// 保持时长(Timecents)
		short holdTimecents = -12000;
		// 衰减时长(Timecents)
		short decayTimecents = -12000;



		//当前点在包络线上原始点值
		float curtValue = 0;


	protected:

		StageRangeInfo stageRangeInfo[(int)(EnvStage::Count)];

		// 当前包络阶段
		EnvStage curtStage = EnvStage::Stop;

		// 被启动的时间点
		float openSec = 0;

		// 是否按下按键
		bool onKey = false;

		// 当前按键号
		int noteKeyNum = 60;

		// 松开按键的时间点，以0为参考点
		float offKeySec = 0;

		// 基时间点
		float baseSec = 0;

		// 当前时间点
		float curtSec = 0;

		float realDecaySec = 0;
		float realHoldSec = 0;

		//是否快速释音
		bool isFastRelease = false;

		float oldSec = -1;
		float oldOutput = 0;

	};
}

#endif
