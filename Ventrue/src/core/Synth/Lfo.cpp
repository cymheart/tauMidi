#include"Lfo.h"

namespace ventrue
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

        return  amp * (float)FastSin((double)freq * sec * 2 * M_PI);
    }
}