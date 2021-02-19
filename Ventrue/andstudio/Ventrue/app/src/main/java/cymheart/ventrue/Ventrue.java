package cymheart.ventrue;

public class Ventrue
{

    static {
        System.loadLibrary("ventrue");
    }

    public Ventrue()
    {
        ndkVentrue = ndkCreateVentrue();
    }

    public void SetFrameSampleCount(int sampleCount)
    {
        ndkSetFrameSampleCount(ndkVentrue, sampleCount);
    }

   //设置声道模式(立体声，单声道设置)
    public void SetChannelCount(int channelCount)
    {
        ndkSetChannelCount(ndkVentrue, channelCount);
    }

    public void OpenAudio()
    {
        ndkOpenAudio(ndkVentrue);
    }

    public void ParseSoundFont(String formatName, String filepath)
    {
        ndkParseSoundFont(ndkVentrue, formatName, filepath);
    }


    //获取所有发声区域的数量
    public int GetTotalRegionSounderCount()
    {
        return ndkGetTotalRegionSounderCount(ndkVentrue);
    }


    private long ndkVentrue;
    public long GetNdkVentrue()
    {
        return ndkVentrue;
    }

    //
    private static native long ndkCreateVentrue();
    private static native void ndkSetFrameSampleCount(long ndkVentrue, int sampleCount);
    private static native void ndkSetChannelCount(long ndkVentrue, int channelCount);
    private static native void ndkOpenAudio(long ndkVentrue);
    private static native void ndkParseSoundFont(long ndkVentrue, String formatName, String filepath);
    private static native int ndkGetTotalRegionSounderCount(long ndkVentrue);

    public static native void ndkTest();
}
