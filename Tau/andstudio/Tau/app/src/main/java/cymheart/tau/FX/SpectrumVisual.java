package cymheart.tau.FX;

public class SpectrumVisual extends Effect{

    public SpectrumVisual()
    {
        ndkEffect = ndkCreateSpectrumVisual();
    }

    //频谱可视初始化
    public void Init(
            int number_of_bars, int rate, int channels,
            int start_freq, int end_freq,
            int low_cut_off, int high_cut_off,
            int skip_frame)
    {
        ndkInit(ndkEffect,
                number_of_bars, rate, channels,
                start_freq, end_freq,
                low_cut_off, high_cut_off, skip_frame);
    }

    //获取采样流的频谱的频带条
    public int GetAmpBars(double[] ampOfBars, int[] freqOfBars){
        return ndkGetAmpBars(ndkEffect, ampOfBars, freqOfBars);
    }

    //生成smoothBars频带条
    //smoothAmps: 输出的数据
    //minDB: 对频率的幅值响应限制再小于值60dB都为0值
    //interpCount:对两个频率幅值之间的插值个数
    //smoothCoffe:对整个数据的平滑系数，默认值:0.6
    //channel：需要对那个通道运算，0:左通道， 1:右通道，2:两个通道取最大值
    public int CreateSmoothAmpBars(
            double[] smoothAmps,
            float minDB, int interpCount, float smoothCoffe,
            int channel)
    {
        return ndkCreateSmoothAmpBars(ndkEffect,
                smoothAmps, minDB, interpCount, smoothCoffe, channel);
    }

    private static native long ndkCreateSpectrumVisual();

    //频谱可视初始化
    private static native void ndkInit(
            long ndkEffect,
            int number_of_bars, int rate, int channels,
            int start_freq, int end_freq,
            int low_cut_off, int high_cut_off,
            int skip_frame);


    //获取采样流的频谱的频带条
    private static native int ndkGetAmpBars(long ndkEffect, double[] ampOfBars, int[] freqOfBars);


    //生成smoothBars频带条
    //smoothAmps: 输出的数据
    //minDB: 对频率的幅值响应限制再小于值60dB都为0值
    //interpCount:对两个频率幅值之间的插值个数
    //smoothCoffe:对整个数据的平滑系数，默认值:0.6
    //channel：需要对那个通道运算，0:左通道， 1:右通道，2:两个通道取最大值
    private static native int ndkCreateSmoothAmpBars(
            long ndkEffect,
            double[] smoothAmps,
            float minDB, int interpCount, float smoothCoffe,
            int channel);

}
