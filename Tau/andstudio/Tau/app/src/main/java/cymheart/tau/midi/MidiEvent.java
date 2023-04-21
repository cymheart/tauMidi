package cymheart.tau.midi;

import org.json.JSONException;
import org.json.JSONObject;

import cymheart.tau.editor.Track;

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
    public final static int NoteColor_DownBlue = 1;
    public final static int NoteColor_DownGreen = 2;
    public final static int NoteColor_DownOrange = 3;
    public final static int NoteColor_DownYellow = 4;
    public final static int NoteColor_DownPurple = 5;
    public final static int NoteColor_DownRed = 6;


    //
    public int type = Unknown;
    protected int playType = PlayType_Background;
    public int GetPlayType(){return playType;}
    public void SetPlayType(int type)
    {
        playType = type;

        if(ndkMidiEvent != 0)
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

    public int index = 0;

    // 起始tick
    public int startTick = 0;
    //起始时间点(单位:秒)
    public float startSec = 0;
    //结束时间点(单位:秒)
    public float endSec = 0;
    //事件相关通道
    public int channel = -1;
    //事件相关轨道编号
    public int trackIdx = -1;
    //事件相关轨道
    public Track track = null;

    public JSONObject jsonMidiEvent = null;
    public void SetByInnerJson()
    {
        if(jsonMidiEvent == null)
            return;

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

    private long ndkMidiEvent = 0;

    protected static native void ndkSetPlayType(long ndkMidiEvent, int playType);
}
