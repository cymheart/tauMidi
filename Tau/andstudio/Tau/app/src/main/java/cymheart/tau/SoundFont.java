package cymheart.tau;

public class SoundFont {
    public SoundFont()
    {
        ndkSoundFont = ndkCreateSoundFont();
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        Release();
    }

    public void Release()
    {
        ndkRelease(ndkSoundFont);
        ndkSoundFont = 0;
    }

    //根据格式类型,解析soundfont文件
    public void Parse(String formatName, String path)
    {
        ndkParse(ndkSoundFont, formatName, path);
    }

    //开启物理钢琴
    public void EnablePhysicsPiano(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
    {
        ndkEnablePhysicsPiano(ndkSoundFont, bankSelectMSB, bankSelectLSB, instrumentNum);
    }

    public Preset[] GetPresetList()
    {
        return ndkGetPresetList(ndkSoundFont);
    }

    private long ndkSoundFont;
    public long GetNdkSoundFont()
    {
        return ndkSoundFont;
    }

    //
    private static native long ndkCreateSoundFont();
    private static native void ndkRelease(long nkdSoundFont);
    private static native void ndkParse(long nkdSoundFont, String formatName, String path);
    private static native void ndkEnablePhysicsPiano(long nkdSoundFont, int bankSelectMSB, int bankSelectLSB, int instrumentNum);
    private static native Preset[] ndkGetPresetList(long nkdSoundFont);
}
