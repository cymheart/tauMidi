#ifndef _ParameterEqualizer_h_
#define _ParameterEqualizer_h_

#include "CascadeBiquad.h"

namespace dsignal
{
	/**
	* 参数均衡器
	*/
	class ParameterEqualizer
	{
	public:
		ParameterEqualizer();
		void Enable(int filterIdx, bool isEnable);

	private:
		void Init();

	private:
		float sampleRate = 44100;
		CascadeBiquad biquad[9];
	};
}

#endif
