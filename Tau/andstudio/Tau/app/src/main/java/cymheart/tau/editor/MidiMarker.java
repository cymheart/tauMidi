package cymheart.tau.editor;

public class MidiMarker {

    static final String[] majorName = {
        "C major","G major","D major","A major","E major","B major","#F major","#C major",
            "F major","bB major","bE major","bA major","bD major","bG major","bC major"
    };

    static final String[] majorFullName = {
        "C major","G major","D major","A major","E major","B major","F sharp major","C sharp major",
            "F major","B flat major","E flat major","A flat major","D flat major","G flat major","C flat major"
    };

    static final String[] minorName = {
        "A minor","E minor","B minor","#F minor","#C minor","#G minor","#D minor",
            "#A minor","D minor","G minor","C minor","F minor","bB minor","bE minor","bA minor"
    };

    static final String[] minorFullName = {
        "A minor","E minor","B minor","F sharp minor","C sharp minor","G sharp minor","D sharp minor",
            "A sharp minor","D minor","G minor","C minor","F minor","B flat minor","E flat minor","A flat minor"
    };

    protected int track = 0;
    public boolean isEnableMarkerText = false;
    public boolean IsEnableMarkerText()
    {
        return isEnableMarkerText;
    }

    protected String titleName;
    public String GetTitle()
    {
        return titleName;
    }

    //是否开启速度设置
    public boolean isEnableTempo = false;
    public boolean IsEnableTempo()
    {
        return isEnableTempo;
    }

    //标记起始的tick数
    protected int startTick = 0;
    public int GetStartTick()
    {
        return startTick;
    }

    //标记起始的时间点
    public double startSec = 0;
    public double GetStartSec()
    {
        return startSec;
    }
    public void SetStartSec(double sec){startSec = sec;}

    // 一个四分音符的微秒数
    protected float microTempo = 0;
    public float GetMicroTempo()
    {
        return microTempo;
    }

    // 一个四分音符的tick数
    protected int tickForQuarterNote = 480;
    public int GetTickForQuarterNote()
    {
        return tickForQuarterNote;
    }

    //
    //是否开启节拍设置
    protected boolean isEnableTimeSignature = false;
    public boolean IsEnableTimeSignature()
    {
        return isEnableTimeSignature;
    }

    // 拍号分子
    protected int numerator = 4;
    public int GetNumerator()
    {
        return numerator;
    }

    // 拍号分母
    // denominatorResult = pow(2, denominator);
    protected int denominator = 2;
    public int GetDenominator()
    {
        return denominator;
    }

    //
    //是否开启调号设置
    protected boolean isEnableKeySignature = false;
    public boolean IsEnableKeySignature()
    {
        return isEnableKeySignature;
    }

    /// <summary>
    /// sf指明乐曲曲调中升号、降号的数目。
    /// 也就是说，升号数目写成0x，降号数目写成8x
    /// sf = -7: 7 flats 7个降号
    /// sf = -1: 1 flat  1个降号
    /// sf = 0: key of C C调
    /// sf = 1: 1 sharp  1个升调
    /// sf = 7: 7 sharps 7个升调
    /// </summary>
    protected int sf = 0;
    public int GetSf()
    {
        return sf;
    }

    /// <summary>
    /// 大调小调
    /// 0:major (大调)
    /// 1:minor(小调)
    /// </summary>
    protected int mi = 0;
    public int GetMi()
    {
        return mi;
    }


    public int GetIdx(){
        return (sf < 0 ? 7 - sf : sf);
    }

    //获取调号名称
    public String GetKeySignatureName()
    {
        int n = GetIdx();
        return (mi == 0 ? majorName[n] : minorName[n]);
    }


    // 获取调号全名
    public String GetKeySignatureFullName()
    {
        int n = GetIdx();
        return (mi == 0 ? majorFullName[n] : minorFullName[n]);
    }

}
