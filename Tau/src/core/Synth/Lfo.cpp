#include"Lfo.h"

namespace tau
{
	void Lfo::Open(float sec)
	{
		openSec = sec;
	}


	float Lfo::SinWave(float sec)
	{
		sec -= openSec + delay;
		if (sec < 0)
			return 0;

		if (sec == oldSec)
			return oldOutput;

		oldOutput = amp * (float)FastSin((double)freq * sec * 2 * M_PI);
		oldSec = sec;
		return oldOutput;
	}
}
