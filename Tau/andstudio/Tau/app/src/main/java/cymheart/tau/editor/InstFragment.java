package cymheart.tau.editor;

import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import cymheart.tau.midi.MidiEvent;

public class InstFragment {

    //是否播放结束
    protected boolean isEnded = false;

    protected int startTick = 0;
    protected float startSec = 0;

    protected Track track = null;
    protected int branchIdx = 0;

    //
    protected MidiEvent[] _ndkMidiEvent;
    protected List<MidiEvent> midiEvents = new LinkedList<>();
    protected ListIterator<MidiEvent> eventOffsetIter;
    protected ListIterator<MidiEvent> eventFirstIter;

    public List<MidiEvent> GetMidiEvents() {
        return midiEvents;
    }

    void Clear()
    {
        isEnded = false;
        eventOffsetIter = midiEvents.listIterator();
        eventFirstIter = midiEvents.listIterator();
    }

}
