package cymheart.tau.dsignal;

public class MorphLBHFilter extends Filter{

    public MorphLBHFilter()
    {
        ndkFilter = ndkCreateMorphLBHFilter();
    }
    public Filter[] GetFilter()
    {
        return ndkGetFilter(ndkFilter);
    }

    //设置频率点
    public void SetFreq(float freqHZ)
    {
        ndkSetFreq(ndkFilter, freqHZ);
    }


    //设置Q
    public void SetQ(float q)
    {
        ndkSetQ(ndkFilter, q);
    }

    //设置过渡位置0-1
    public void SetAlpha(float a)
    {
        ndkSetAlpha(ndkFilter, a);
    }

    //计算滤波参数
    public void Compute()
    {
        ndkCompute(ndkFilter);
    }

    private static native long ndkCreateMorphLBHFilter();
    private static native Filter[] ndkGetFilter(long ndkFilter);
    private static native void ndkSetFreq(long ndkFilter, float freqHZ);
    private static native void ndkSetQ(long ndkFilter, float q);
    private static native void ndkSetAlpha(long ndkFilter, float a);
    private static native void ndkCompute(long ndkFilter);
}
