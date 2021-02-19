#ifndef _Instrument_h_
#define _Instrument_h_

#include "VentrueTypes.h"
#include "Sample.h"
#include "Region.h"

namespace ventrue
{
    class Instrument
    {
    public:
        Instrument();
        ~Instrument();

        Region* GetGlobalRegion()
        {
            return globalRegion;
        }

        // 连接一个样本到一个instRegion
        Region* LinkSamples(Sample* sample);

        // 获取KeyNum在指定范围内的乐器区域组
        int GetHavKeyInstRegionLinkInfos(int keyNum, float velocity, SamplesLinkToInstRegionInfo* activeInstRegionLinkInfos);
       
    public:
        string name;

    private:

        Region* globalRegion;
        SamplesLinkToInstRegionInfoList* instRegionLinkInfoList;

    };
}

#endif