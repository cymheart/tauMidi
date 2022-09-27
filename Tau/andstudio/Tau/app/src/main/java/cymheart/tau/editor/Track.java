package cymheart.tau.editor;

import android.graphics.Color;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import cymheart.tau.Channel;
import cymheart.tau.midi.MidiEvent;
import cymheart.tau.utils.ScLinkedList;
import cymheart.tau.utils.ScLinkedListNode;

public class Track {


    //是否禁止播放
    protected boolean isDisablePlay = false;

    // 通道
    Channel channel;
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


    protected List<ScLinkedList<InstFragment>> instFragmentBranchs = new ArrayList<>();
    public List<ScLinkedList<InstFragment>> GetInstFragmentBranchs() {
        return instFragmentBranchs;
    }

    //结束时间点
    protected double endSec;
    protected int playType = MidiEvent.PlayType_Background;
    protected int noteColor = MidiEvent.NoteColor_None;

    public void Clear()
    {
        //
        InstFragment instFragment;
        for (int i = 0; i < instFragmentBranchs.size(); i++)
        {
            ScLinkedList<InstFragment> instFragList = instFragmentBranchs.get(i);
            ScLinkedListNode<InstFragment> node = instFragList.GetHeadNode();
            for(;node != null; node = node.next)
            {
                instFragment = node.elem;
                instFragment.Clear();
            }
        }
    }
}
