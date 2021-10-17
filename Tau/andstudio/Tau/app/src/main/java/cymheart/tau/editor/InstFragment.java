package cymheart.tau.editor;

import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import cymheart.tau.midi.MidiEvent;

public class InstFragment {

    protected int startTick = 0;
    protected double startSec = 0;
    protected double endSec = 0;

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
        eventOffsetIter = midiEvents.listIterator();
        eventFirstIter = midiEvents.listIterator();
    }

}
