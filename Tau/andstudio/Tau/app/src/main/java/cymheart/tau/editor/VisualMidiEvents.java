package cymheart.tau.editor;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import cymheart.tau.midi.NoteOnEvent;

public class VisualMidiEvents
{
    protected List<List<NoteOnEvent>> noteOnEvents = new ArrayList<>();
    public List<List<NoteOnEvent>> GetAllKeyEvents()
    {
        return noteOnEvents;
    }

    protected List<int[]> noteOnTrackGroups = new ArrayList<>();
    public  List<int[]> GetNoteOnTrackGroups()
    {
        return noteOnTrackGroups;
    }

    protected boolean[] noteUsedMark = new boolean[128];
    public boolean NoteUsedHav(int note)
    {
        if(usedNoteCount <= 0 || note > 127 || note < 0)
            return false;
        return noteUsedMark[note];
    }

    protected void ClearNoteUsedMark()
    {
        for(int i=0; i<usedNoteCount; i++) {
            noteOnEvents.get(usedNotes[i]).clear();
            noteUsedMark[usedNotes[i]] = false;
        }
    }

    /**按按键顺序排列的按键*/
    protected int[] usedNotes = new int[128];
    /**按按键顺序排列按键*/
    public int[] GetUsedNotes()
    {
        return usedNotes;
    }
    protected int usedNoteCount = 0;
    public int GetUsedNoteCount()
    {
        return usedNoteCount;
    }

    /**按按键顺序排列按键*/
    public void SortUsedNotes()
    {
        Arrays.sort(usedNotes, 0, usedNoteCount);
    }

    public VisualMidiEvents()
    {
        for(int i=0; i<128; i++) {
            noteUsedMark[i] = false;
            noteOnEvents.add(new ArrayList<>());
            noteOnTrackGroups.add(new int[200]);
        }

    }

}