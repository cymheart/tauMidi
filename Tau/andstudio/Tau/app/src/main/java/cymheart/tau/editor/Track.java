package cymheart.tau.editor;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import cymheart.tau.Channel;
import cymheart.tau.midi.MidiEvent;

public class Track {


    //是否禁止播放
    protected boolean isDisablePlay = false;

    // 通道
    Channel channel;

    public Channel GetChannel() {
        return channel;
    }

    protected List<LinkedList<InstFragment>> instFragments = new ArrayList<>();

    public List<LinkedList<InstFragment>> GetInstFragments() {
        return instFragments;
    }

    //结束时间点
    protected double endSec;



    public void Clear()
    {
        //
        InstFragment instFragment;
        for (int i = 0; i < instFragments.size(); i++)
        {
            LinkedList<InstFragment> instFragList = instFragments.get(i);
            for(ListIterator<InstFragment> iter = instFragList.listIterator(); iter.hasNext();)
            {
                instFragment = iter.next();
                instFragment.Clear();
            }
        }
    }
}
