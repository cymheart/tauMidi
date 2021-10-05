package cymheart.tau;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import cymheart.tau.midi.NoteOnEvent;

public class VisualKeyEvents
{
    protected List<List<NoteOnEvent>> noteOnEvents = new ArrayList<>();
    public List<List<NoteOnEvent>> GetAllKeyEvents()
    {
        return noteOnEvents;
    }

    protected Set<Integer> noteSet = new HashSet<>();

    public Set<Integer> GetNoteSet()
    {
        return noteSet;
    }


    public VisualKeyEvents()
    {
        for(int i=0; i<128; i++)
            noteOnEvents.add(new ArrayList<NoteOnEvent>());
    }

}