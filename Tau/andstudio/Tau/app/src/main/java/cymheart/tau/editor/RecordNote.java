package cymheart.tau.editor;

/**录制音符*/
public class RecordNote {


    /**松开*/
    static public final int Off = 0;
    /**按下*/
    static public final int On = 1;
    /**等待*/
    static public final int Wait = 2;
    /**继续*/
    static public final int Continue = 3;

    //类型
    public int type = Off;
    // 音符
    public int note = 0;
    // 力度
    public int velocity = 0;
    // 轨道
    public int track = 0;

    /**时间点(单位:秒)*/
    public float sec = 0;
}
