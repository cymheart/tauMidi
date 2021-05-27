package cymheart.ventrue;

public class VirInstrument
{
    //打开中
    public static final int ONING = 0;
    //已经打开
    public static final int  ONED = 1;
    //关闭中
    public static final int   OFFING = 2;
    //已经关闭
    public static final int  OFFED = 3;

    private VirInstrument()
    {
        setClsRef();
    }

    //获取状态
    public int GetState()
    {
        return ndkGetState(ndkVirInstrument);
    }

    private long ndkVirInstrument;
    public long GetNdkVirInstrument()
    {
        return ndkVirInstrument;
    }

    private native void setClsRef();
    private native int ndkGetState(long ndkVirInstrument);

}
