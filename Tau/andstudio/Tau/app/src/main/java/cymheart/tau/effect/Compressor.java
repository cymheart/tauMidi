package cymheart.tau.effect;

public class Compressor extends Effect {

    public Compressor()
    {
        ndkEffect = ndkCreateCompressor();
    }

    //设置采样频率
    public void SetSampleFreq(float freq)
    {
        ndkSetSampleFreq(ndkEffect, freq);
    }

    //设置Attack时长
    public void SetAttackSec(float sec) {
        ndkSetAttackSec(ndkEffect, sec);
    }

    //设置Release时长
    public void SetReleaseSec(float sec) {
        ndkSetReleaseSec(ndkEffect, sec);
    }

    //设置比值
    public void SetRadio(float radio) {
        ndkSetRadio(ndkEffect, radio);
    }

    //设置门限
    public void SetThreshold(float threshold) {
        ndkSetThreshold(ndkEffect, threshold);
    }

    //设置拐点的软硬
    public void SetKneeWidth(float width) {
        ndkSetKneeWidth(ndkEffect, width);
    }

    //设置增益补偿
    public void SetMakeupGain(float gain) {
        ndkSetMakeupGain(ndkEffect, gain);
    }

    //设置是否自动增益补偿
    public void SetAutoMakeupGain(boolean isAuto) {
        ndkSetAutoMakeupGain(ndkEffect, isAuto);
    }

    //计算系数
    public void CalculateCoefficients() {
        ndkCalculateCoefficients(ndkEffect);
    }

    private static native long ndkCreateCompressor();
    private static native void ndkSetSampleFreq(long ndkEffect, float freq);
    private static native void ndkSetAttackSec(long ndkEffect, float sec);
    private static native void ndkSetReleaseSec(long ndkEffect, float sec);
    private static native void ndkSetRadio(long ndkEffect, float radio);
    private static native void ndkSetThreshold(long ndkEffect, float threshold);
    private static native void ndkSetKneeWidth(long ndkEffect, float width);
    private static native void ndkSetMakeupGain(long ndkEffect,float gain);
    private static native void ndkSetAutoMakeupGain(long ndkEffect, boolean isAuto);
    private static native void ndkCalculateCoefficients(long ndkEffect);

}
