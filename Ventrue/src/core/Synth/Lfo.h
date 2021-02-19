#ifndef _Lfo_h_
#define _Lfo_h_

#include"VentrueTypes.h"

namespace ventrue
{
    class Lfo
    {
    public:

        inline void Clear()
        {
            freq = 0;
            delay = 0;
            amp = 1;
            openSec = 0;
        }

        void Open(float sec);
        float SinWave(float sec);

    public:
        // LFO调制频率
        float freq = 0;

        // 延迟启动
        float delay = 0;

        // 幅值
        float amp = 1;

    private:

        // lfo被启动的时间点
        float openSec = 0;
    };
}

#endif
