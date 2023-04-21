package cymheart.tau.editor;


import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import cymheart.tau.Channel;
import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.ScLinkedList;
import cymheart.tau.utils.ScLinkedListNode;

public class Track {

    public int idx = 0;



    //是否禁止播放
    protected boolean isDisablePlay = false;

    // 通道
    protected Channel channel;
    public Channel GetChannel() {
        return channel;
    }

    protected JSONObject jsonTrack;
    public void SetByInnerJson()
    {
        try {
            playType = jsonTrack.getInt("PlayType");
            noteColor = jsonTrack.getInt("NoteColor");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetInnerJson()
    {
        if(jsonTrack == null)
            return;

        try {
            jsonTrack.put("PlayType", playType);
            jsonTrack.put("NoteColor", noteColor);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    protected NoteOnEvent[] noteOnEvents;
    protected int noteOnEventsOffset = 0;
    protected int noteOnEventsFirst = 0;

    //结束时间点
    protected double endSec;
    protected int playType = MidiEvent.PlayType_Background;
    public int GetPlayType() {
        return playType;
    }

    public void SetPlayType(int type)
    {
        playType = type;

        if(jsonTrack == null)
            return;

        try {
            jsonTrack.put("PlayType", type);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }


    protected int noteColor = MidiEvent.NoteColor_None;
    public int GetNoteColor() {
        return noteColor;
    }

    public void SetNoteColor(int color)
    {
        noteColor = color;

        if(jsonTrack == null)
            return;

        try {
            jsonTrack.put("NoteColor", color);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void Clear()
    {
        noteOnEventsOffset = 0;
        noteOnEventsFirst = 0;
    }
}
