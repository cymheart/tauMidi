package cymheart.tau.midi;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
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
        return startTick<<7 | note;
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

    /**记录推迟多长时间被按下*/
    public float lateDownSec = 0;

    /**是否弹奏*/
    public boolean isPlay = false;
    /**是否丢失*/
    public boolean isMiss = false;
    /**游戏点数*/
    public float gamePoint = 0;
    /**游戏点数倍率*/
    public float gamePointMul = 1;
    /**推迟多长时间被按下*/
    public float lateSec = 0;


    /**显示区域*/
    public float left, top, right, bottom;

    // 结束tick
    public int endTick = 0;
    // 音符
    public int note = 0;
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


    @Override
    public void SetByInnerJson()
    {
        if(jsonMidiEvent == null)
            return;

        super.SetByInnerJson();

        try {
            fingerIdxCount = 0;
            JSONArray jsonFingerIdxs = jsonMidiEvent.getJSONArray("fingerIdxs");
            if(jsonFingerIdxs != null){
                for (int j = 0; j < jsonFingerIdxs.length(); j++) {
                    JSONObject jsonFingerIdx = jsonFingerIdxs.getJSONObject(j);
                    fingerAtHand[fingerIdxCount++] = jsonFingerIdx.getInt("hand");
                    fingerIdxs[fingerIdxCount++] = jsonFingerIdx.getInt("idx");
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void SetInnerJson()
    {
        if(jsonMidiEvent == null)
            return;

        super.SetInnerJson();

        try {
            JSONArray jsonFingerIdxs = jsonMidiEvent.getJSONArray("fingerIdxs");
            if(jsonFingerIdxs == null)
            {
                jsonFingerIdxs = new JSONArray();
                jsonMidiEvent.put("fingerIdxs", jsonFingerIdxs);
            }else{
                for (int j = jsonFingerIdxs.length(); j >= 0; j--)
                    jsonFingerIdxs.remove(j);
            }

            for(int i=0; i<fingerIdxCount; i++)
            {
                JSONObject jsonFingerIdx = new JSONObject();
                jsonFingerIdx.put("hand", fingerAtHand[i]);
                jsonFingerIdx.put("idx", fingerIdxs[i]);
                jsonFingerIdxs.put(jsonFingerIdx);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private long ndkNoteOffEvent = 0;
}
