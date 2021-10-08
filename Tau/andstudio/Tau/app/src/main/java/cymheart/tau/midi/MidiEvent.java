package cymheart.tau.midi;

import cymheart.tau.editor.InstFragment;

public class MidiEvent {

    //未定义
    static public final int Unknown = -1;
    //按下音符
    static public final int NoteOn = 0;
    //松开音符
    static public final int NoteOff= 1;
    //速度设置
    static public final int Tempo= 2;
    //节拍设置
    static public final int TimeSignature= 3;
    //音调符号
    static public final int KeySignature = 4;
    //控制器
    static public final int Controller= 5;
    //乐器更换
    static public final int ProgramChange= 6;
    //音符触后力度
    static public final int KeyPressure= 7;
    //通道力度
    static public final int ChannelPressure= 8;
    //滑音
    static public final int PitchBend= 9;
    //文本
    static public final int Text= 10;
    //系统码
    static public final int Sysex= 11;
    //元事件
    static public final int Meta = 12;


    public int type = Unknown;

    // 起始tick
    public int startTick = 0;
    //起始时间点(单位:秒)
    public float startSec = 0;
    //结束时间点(单位:秒)
    public float endSec = 0;
    //事件相关通道
    public int channel = -1;

    InstFragment instFragment;
}
