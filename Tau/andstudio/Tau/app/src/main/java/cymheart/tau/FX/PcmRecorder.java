package cymheart.tau.FX;

public class PcmRecorder extends Effect{
    public PcmRecorder()
    {
        ndkEffect = ndkCreatePcmRecorder();
    }

    public void ClearRecordPCM()
    {
        ndkClearRecordPCM(ndkEffect);
    }

    public void StartRecordPCM()
    {
        ndkStartRecordPCM(ndkEffect);
    }

    public void StopRecordPCM()
    {
        ndkStopRecordPCM(ndkEffect);
    }

    public void SaveRecordPCM(String path)
    {
        ndkSaveRecordPCM(ndkEffect, path);
    }

    public void SaveRecordPCMToWav(String path, int sampleRate, int numChannels)
    {
        ndkSaveRecordPCMToWav(ndkEffect, path, sampleRate, numChannels);
    }

    public void SaveRecordPCMToMp3(String path, int sampleRate, int numChannels)
    {
        ndkSaveRecordPCMToMp3(ndkEffect, path, sampleRate, numChannels);
    }

    private static native long ndkCreatePcmRecorder();
    private static native void ndkClearRecordPCM(long ndkEffect);
    private static native void ndkStartRecordPCM(long ndkEffect);
    private static native void ndkStopRecordPCM(long ndkEffect);
    private static native void ndkSaveRecordPCM(long ndkEffect, String path);
    private static native void ndkSaveRecordPCMToWav(long ndkEffect, String path, int sampleRate, int numChannels);
    private static native void ndkSaveRecordPCMToMp3(long ndkEffect, String path, int sampleRate, int numChannels);

}
