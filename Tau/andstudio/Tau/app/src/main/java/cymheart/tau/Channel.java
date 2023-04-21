package cymheart.tau;

public class Channel {

    //通道号
    protected int channelNum = 0;
    public int GetChannelNum()
    {
        return channelNum;
    }

    //所在乐器
    protected VirInstrument inst;
    public VirInstrument GetVirInstrument()
    {
        return inst;
    }
}
