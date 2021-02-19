#ifndef _UnitTransform_h_
#define _UnitTransform_h_

#include"VentrueTypes.h"

namespace ventrue
{
    class UnitTransform
    {
    public:
 
        // 分贝转增益   
        // <param name="db"></param>
        static float DecibelsToGain(float db);
  
        // 增益转分贝
        // <param name="gain"></param>
        static float GainToDecibels(float gain);

        // 共振峰Db值转滤波Q值
        // <param name="resonanceDb"></param>
        static float ResonanceDbToFilterQ(float resonanceDb);

        //cB转分贝
        static float CentibelsToDecibels(float cB);

        // 音分转倍率
        // <param name="cents"></param>
        static float CentsToMul(float cents);

        // 音分转赫兹(Hz)
        // <param name="cents"></param>
        static float CentsToHertz(float cents);
         
        // timecents转秒   
        // <param name="timecents"></param>     
        static float TimecentsToSecsf(float timecents);
  
        // 半音转倍率
        // 1半音之间的频率倍率为2^(1/12) = 1.059463f    
        // <param name="semitone"></param>   
        static float SemitoneToMul(float semitone);

    private:
        UnitTransform() {}
        ~UnitTransform() {}
      
    };
}

#endif