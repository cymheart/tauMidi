package cymheart.tau.midi;


import android.graphics.RectF;

import java.util.List;

public class NoteOnEvent extends MidiEvent {

    /**子事件*/
    public NoteOnEvent[] childNoteOnEvents = null;
    public int[] childNoteState = null;
    public void CreateChildNoteOnEvents(List<NoteOnEvent> notes)
    {
        childNoteOnEvents = new NoteOnEvent[notes.size()];
        childNoteState = new int[notes.size()];

        for(int i=0; i<notes.size(); i++) {
            childNoteOnEvents[i] = notes.get(i);
            childNoteState[i] = 0;
        }
    }

    public int GetID()
    {
        return startTick<<7 | num;
    }


    /**手指所在手<p>
     * 0：左手<p>
     * 1: 右手<p>
     * */
    public int[] fingerAtHand = new int[20];
    /**手指编号提示*/
    public int[] fingerIdxs = new int[20];
    /**手指编号提示数量*/
    public int fingerIdxCount = 0;

    public void ClearHandFingers(){
        fingerIdxCount = 0;
    }

    /**是否已具有手指标签*/
    public boolean HavHandFingerLabel(int hand, int finger){
        for(int i=0; i<fingerIdxCount; i++) {
            if(fingerAtHand[i] == hand && fingerIdxs[i] == finger)
                return true;
        }
        return false;
    }


    /**添加手指标签*/
    public void AddHandFingerLabel(int hand, int finger){
        fingerAtHand[fingerIdxCount++] = hand;
        fingerIdxs[fingerIdxCount++] = finger;
    }

    /**移除手指标签*/
    public void RemoveHandFingerLabel(int hand, int finger){
        for(int i=0; i<fingerIdxCount; i++)
        {
            if(fingerAtHand[i] == hand && fingerIdxs[i] == finger) {
                fingerAtHand[i] = -1;
                fingerIdxs[i] = -1;
                for(int j=i+1; j<fingerIdxCount; j++) {
                    fingerAtHand[j - 1] = fingerAtHand[j];
                    fingerIdxs[j - 1] = fingerIdxs[j];
                }
                return;
            }
        }
    }



    /**记录的推迟多长时间被按下*/
    public float recordLateSec = 0;

    /**记录的是否丢失*/
    public boolean recordIsMiss = false;

    /**是否弹奏*/
    public boolean isPlay = false;
    /**是否丢失*/
    public boolean isMiss = false;

    /**是否已发送按键信号*/
    public boolean isOnKeySignal = false;

    /**游戏点数*/
    public float gamePoint = 0;
    /**游戏点数倍率*/
    public float gamePointMul = 1;
    /**推迟多长时间被按下*/
    public float lateSec = 0;


    /**显示区域*/
    public float left, top, right, bottom;


    /**弹奏的开始时间*/
    public float playedStartSec;
    /**弹奏的结束时间*/
    public float playedEndSec;
    /**弹奏的显示区域*/
    public RectF playedArea = new RectF();



    // 结束tick
    public int endTick = 0;
    // 音符号
    public int num = 0;
    // 力度
    public int velocity = 0;

    public NoteOnEvent()
    {
        type = MidiEvent.NoteOn;
    }

    @Override
    public void SetPlayType(int type)
    {
        super.SetPlayType(type);
        if(ndkNoteOffEvent != 0)
            ndkSetPlayType(ndkNoteOffEvent, playType);
    }


    private long ndkNoteOffEvent = 0;
}
