#ifndef _Lfo_h_
#define _Lfo_h_

#include"TauTypes.h"

namespace tau
{
	//LFO 调制（Low Frequency Oscillator Modulation） 
	//是一种通过低频振荡器周期性改变声音参数的技术，用于为音色增添动态变化效果
	//LFO调制输出振幅值
	class Lfo
	{
	public:

		inline void Clear()
		{
			freq = 10;
			delay = 0;
			amp = 1;
			openSec = 0;
			oldSec = -1;
			oldOutput = 0;
		}

		void Open(float sec);
		float SinWave(float sec);


	public:
		// LFO调制频率
		float freq = 10;

		// 延迟启动
		float delay = 0;

		// 幅值
		float amp = 1;

	private:

		// lfo被启动的时间点
		float openSec = 0;

		float oldSec = -1;
		float oldOutput = 0;
	};
}

#endif
