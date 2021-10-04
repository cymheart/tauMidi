package cymheart.ventrue.dsignal;

public class Bode {
    public Bode()
    {
        ndkBode = ndkCreateBode();
    }

    //增加数字滤波器到要计算的bode图中
    public void AddFilters(Filter[] filters)
    {
        for(int i=0; i<filters.length; i++)
            ndkAddFilter(ndkBode, filters[i].GetNdkFilter());
    }

    //增加一个数字滤波器到要计算的bode图中
    public void AddFilter(Filter filter)
    {
        ndkAddFilter(ndkBode, filter.GetNdkFilter());
    }

    //设置采样频率
    public void SetSampleFreq(float sfreq)
    {
        ndkSetSampleFreq(ndkBode, sfreq);
    }

    //设置频率响应采样个数
    //默认值:512个
    public void SetFreqzSampleCount(int count)
    {
        ndkSetFreqzSampleCount(ndkBode, count);
    }

    //设置bode图绘制区域的宽度
    public void SetPlotAreaWidth(float width)
    {
        ndkSetPlotAreaWidth(ndkBode, width);
    }

    //设置bode图绘制区域的高度
    public void SetPlotAreaHeight(float height)
    {
        ndkSetPlotAreaHeight(ndkBode, height);
    }

    //设置bode图绘制区域频率轴的起始Hz(单位:HZ)
    public void SetPlotFreqAxisStart(float startFreqHz)
    {
        ndkSetPlotFreqAxisStart(ndkBode, startFreqHz);
    }

    //设置bode图绘制区域频率轴的结束Hz(单位:HZ)
    public void SetPlotFreqAxisEnd(float endFreqHz)
    {
        ndkSetPlotFreqAxisEnd(ndkBode, endFreqHz);
    }

    //设置bode图绘制区域增益DB的范围
    //默认值:+15dB ~ -15dB 范围为30dB
    public void SetPlotGainDbRange(float gainRangeDB)
    {
        ndkSetPlotGainDbRange(ndkBode, gainRangeDB);
    }

    //设置频率轴标尺频率点
    public void SetRulerFreqs(float[] rulerFreqs)
    {
        ndkSetRulerFreqs(ndkBode, rulerFreqs);
    }

    //设置增益轴标尺dB点
    public void SetRulerGainDBs(float[] ruleGainDB)
    {
        ndkSetRulerGainDBs(ndkBode, ruleGainDB);
    }

    //计算生成bode图所需要的参数
    public void Compute()
    {
        ndkCompute(ndkBode);
    }

    //bode图绘制频率轴位置点集合
    public float[] GetPlotFreqAxisPos()
    {
        return ndkGetPlotFreqAxisPos(ndkBode);
    }

    //获取bode图绘制增益轴位置点集合
    public float[] GetPlotGainAxisPos()
    {
        return ndkGetPlotGainAxisPos(ndkBode);
    }


    //获取bode图绘制频率轴标尺频率点位
    public float[] GetPlotRulerFreqsPos()
    {
        return ndkGetPlotRulerFreqsPos(ndkBode);
    }

    //获取bode图绘制增益轴标尺dB点位
    public float[] GetPlotRulerGainDBsPos()
    {
        return ndkGetPlotRulerGainDBsPos(ndkBode);
    }


    private long ndkBode;
    public long GetNdkBode()
    {
        return ndkBode;
    }

    //
    private static native long ndkCreateBode();
    private static native void ndkAddFilter(long ndkBode, long ndkFilter);
    private static native void ndkSetSampleFreq(long ndkBode, float sfreq);
    private static native void ndkSetFreqzSampleCount(long ndkBode, int count);
    private static native void ndkSetPlotAreaWidth(long ndkBode, float width);
    private static native void ndkSetPlotAreaHeight(long ndkBode, float height);
    private static native void ndkSetPlotFreqAxisStart(long ndkBode, float startFreqHz);
    private static native void ndkSetPlotFreqAxisEnd(long ndkBode, float endFreqHz);
    private static native void ndkSetPlotGainDbRange(long ndkBode, float gainRangeDB);
    private static native void ndkSetRulerFreqs(long ndkBode, float[] rulerFreqs);
    private static native void ndkSetRulerGainDBs(long ndkBode, float[] ruleGainDB);
    private static native void ndkSetComputeCurveControlPoints(long ndkBode, boolean isCompute);
    private static native void ndkCompute(long ndkBode);
    private static native float[] ndkGetPlotFreqAxisPos(long ndkBode);
    private static native float[] ndkGetPlotGainAxisPos(long ndkBode);
    private static native float[] ndkGetPlotRulerFreqsPos(long ndkBode);
    private static native float[] ndkGetPlotRulerGainDBsPos(long ndkBode);

}
