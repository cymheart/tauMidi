package cymheart.tau.midi;

import org.json.JSONException;
import org.json.JSONObject;

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

    //左手
    static public final int PlayType_LeftHand = 0;
    //右手
    static public final int PlayType_RightHand = 1;
    //双手
    static public final int PlayType_DoubleHand = 2;
    //背景
    static public final int PlayType_Background = 3;
    //自定
    static public final int PlayType_Custom = 4;

    //
    public final static int NoteColor_None = -1;
    public final static int NoteColor_Red = 2;
    public final static int NoteColor_Green = 3;
    public final static int NoteColor_Blue = 4;
    public final static int NoteColor_Yellow = 5;
    public final static int NoteColor_Orange = 6;
    public final static int NoteColor_Purple = 7;


    //
    public int type = Unknown;
    protected int playType = PlayType_Background;
    public void SetPlayType(int type)
    {
        playType = type;
        ndkSetPlayType(ndkMidiEvent, playType);
        SetPlayTypeToInnerJson(playType);
    }

    protected void SetPlayTypeToInnerJson(int type)
    {
        if(jsonMidiEvent == null)
            return;

        try {
            jsonMidiEvent.put("PlayType", type);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    // 起始tick
    public int startTick = 0;
    //起始时间点(单位:秒)
    public float startSec = 0;
    //结束时间点(单位:秒)
    public float endSec = 0;
    //事件相关通道
    public int channel = -1;
    //事件相关轨道
    public int track = -1;

    public JSONObject jsonMidiEvent;
    public void SetByInnerJson()
    {
        try {
            playType = jsonMidiEvent.getInt("PlayType");
            ndkSetPlayType(ndkMidiEvent, playType);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetInnerJson()
    {
        SetPlayTypeToInnerJson(playType);
    }

    InstFragment instFragment;
    private long ndkMidiEvent;


    protected static native void ndkSetPlayType(long ndkMidiEvent, int playType);
}
