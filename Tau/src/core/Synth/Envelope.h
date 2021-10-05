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
	//by cymheart, 2020--2021.
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

		// 按键的keyToHold系数影响保持时长
		// keyToHold为百分比值，比如： 取1即为100%, 取0.25即为25%
		// 按一个八度差，音时长改变2倍
		// newHoldSec = 2^(keyToHold * nOctave) * holdSec
		// 例如:
		// 当前按键键号为40，则相对基键60，相差60 - 40 = 20个半音，即为 20/12 = 1.66667个Octave，即nOcatve = 1.66667
		// keyToHold = 0.5, holdSec = 1.3,
		// 则 newHoldSec = 2^(0.5 * 1.66667) * 1.3 = 2.31634s
		// 再例:
		// 当前按键键号为80，则相对基键60，相差60 - 80 = -20个半音，即为 -20/12 = -1.66667个Octave，即nOcatve = -1.66667
		// keyToHold = 0.5, holdSec = 1.3,
		// 则 newHoldSec = 2^(0.5 * -1.66667) * 1.3 = 0.729599s     
		void CalRealHoldSec();


		// 按键的keyToDecay系数影响衰减时长
		// keyToDecay为百分比值，比如： 取1即为100%, 取0.25即为25%
		// 按一个八度差，音时长改变2倍
		// newDecaySec = 2^(keyToDecay * nOctave) * decaySec
		// 例如:
		// 当前按键键号为40，则相对基键60，相差60 - 40 = 20个半音，即为 20/12 = 1.66667个Octave，即nOcatve = 1.66667
		// keyToDecay = 0.5, decaySec = 1.3,
		// 则 newDecaySec = 2^(0.5 * 1.66667) * 1.3 = 2.31634s
		// 再例:
		// 当前按键键号为61，则相对基键60，相差60 - 61 = -1个半音，即为 -1/12 = -0.0833333个Octave，即nOcatve = -0.0833333
		// keyToDecay = 1, decaySec = 1,
		// 则 newDecaySec = 2^(1 * -0.0833333) * 1 = 0.943874s      
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

		// 保持时长
		float holdSec = 0.001f;

		// 衰减时长
		float decaySec = 0.001f;

		// 延音所在y位置：范围[0,1]
		float sustainY = 1;

		// 释音时长
		float releaseSec = 0.001f;


		// 按键的keyToHold系数影响保持时长
		// keyToHold为百分比值，比如： 取1即为100%, 取0.25即为25%
		// 按一个八度差，音时长改变2倍
		// newHoldSec = 2^(keyToHold * nOctave) * holdSec
		// 例如:
		// 当前按键键号为40，则相对基键60，相差60 - 40 = 20个半音，即为 20/12 = 1.66667个Octave，即nOcatve = 1.66667
		// keyToHold = 0.5, holdSec = 1.3,
		// 则 newHoldSec = 2^(0.5 * 1.66667) * 1.3 = 2.31634s
		// 再例:
		// 当前按键键号为80，则相对基键60，相差60 - 80 = -20个半音，即为 -20/12 = -1.66667个Octave，即nOcatve = -1.66667
		// keyToHold = 0.5, holdSec = 1.3,
		// 则 newHoldSec = 2^(0.5 * -1.66667) * 1.3 = 0.729599s 
		float keyToHold = 0;


		// 按键的keyToDecay系数影响衰减时长
		// keyToDecay为百分比值，比如： 取1即为100%, 取0.25即为25%
		// 按一个八度差，音时长改变2倍
		// newDecaySec = 2^(keyToDecay * nOctave) * decaySec
		// 例如:
		// 当前按键键号为40，则相对基键60，相差60 - 40 = 20个半音，即为 20/12 = 1.66667个Octave，即nOcatve = 1.66667
		// keyToDecay = 0.5, decaySec = 1.3,
		// 则 newDecaySec = 2^(0.5 * 1.66667) * 1.3 = 2.31634s
		// 再例:
		// 当前按键键号为61，则相对基键60，相差60 - 61 = -1个半音，即为 -1/12 = -0.0833333个Octave，即nOcatve = -0.0833333
		// keyToDecay = 1, decaySec = 1,
		// 则 newDecaySec = 2^(1 * -0.0833333) * 1 = 0.943874s
		float keyToDecay = 0;

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

	};
}

#endif
