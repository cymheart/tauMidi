package cymheart.tau.editor;

import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import cymheart.tau.midi.MidiEvent;
import cymheart.tau.utils.ScLinkedList;
import cymheart.tau.utils.ScLinkedListNode;

public class InstFragment {

    protected int startTick = 0;
    protected double startSec = 0;
    protected double endSec = 0;

    protected Track track = null;
    protected int branchIdx = 0;

    //
    protected MidiEvent[] _ndkMidiEvent;
    protected ScLinkedList<MidiEvent> midiEvents = new ScLinkedList<>();
    protected ScLinkedListNode<MidiEvent> eventOffsetNode;
    protected ScLinkedListNode<MidiEvent> eventFirstNode;

    public ScLinkedList<MidiEvent> GetMidiEvents() {
        return midiEvents;
    }

    void Clear()
    {
        eventOffsetNode = eventFirstNode = midiEvents.GetHeadNode();
    }

}
