package cymheart.ventrue;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import cymheart.ventrue.dsignal.Filter;
import cymheart.ventrue.midi.MidiTrack;
import cymheart.ventrue.utils.Utils;

public class Ventrue
{
    static {
        System.loadLibrary("ventrue");
    }

    protected HashMap<Integer, MidiPlay> midiPlayMap = new HashMap<>();
    public void AddMidiPlay(int idx, MidiPlay midiPlay)
    {
        midiPlayMap.put(idx, midiPlay);
    }


    public Ventrue()
    {
        ndkVentrue = ndkCreateVentrue(this);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        Destory();
    }

    public void Destory()
    {
        ndkDestory(ndkVentrue, ndkSoundEndCB);
    }

    public VentrueCmd GetCmd()
    {
        return ndkGetCmd(this);
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

    //设置极限发声区域数量(默认值:600)
    //当播放有卡顿现象时，把这个值调小，会提高声音的流畅度
    public void SetLimitRegionSounderCount(int count)
    {
        ndkSetLimitRegionSounderCount(ndkVentrue, count);
    }

    public Preset[] GetPresetList()
    {
        return ndkGetPresetList(ndkVentrue);
    }

    public void SetSoundEndVirInstCallBack(Utils.Action<VirInstrument[]> cb)
    {
        ndkSoundEndCB = ndkSetSoundEndVirInstCallBack(ndkVentrue, ndkSoundEndCB, cb);
    }


    private long ndkVentrue;
    public long GetNdkVentrue()
    {
        return ndkVentrue;
    }

    private long ndkSoundEndCB;

    //
    private static native long ndkCreateVentrue(Ventrue ventrue);
    private static native void ndkDestory(long ndkVentrue, long ndkSoundEndCB);
    private static native VentrueCmd ndkGetCmd(Ventrue ventrue);
    private static native void ndkSetFrameSampleCount(long ndkVentrue, int sampleCount);
    private static native void ndkSetChannelCount(long ndkVentrue, int channelCount);
    private static native void ndkOpenAudio(long ndkVentrue);
    private static native void ndkParseSoundFont(long ndkVentrue, String formatName, String filepath);
    private static native int ndkGetTotalRegionSounderCount(long ndkVentrue);
    private static native void ndkSetLimitRegionSounderCount(long ndkVentrue, int count);
    private static native Preset[] ndkGetPresetList(long ndkVentrue);

    private native long ndkSetSoundEndVirInstCallBack(long ndkVentrue, long ndkSoundEndCB, Utils.Action<VirInstrument[]> cb);

    public static native void ndkTest();
}
