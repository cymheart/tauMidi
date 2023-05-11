package cymheart.tau.editor;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import cymheart.tau.midi.NoteOnEvent;

/**可视音符*/
public class VisualNotes
{

    protected List<List<NoteOnEvent>> noteGroups = new ArrayList<>();
    public List<List<NoteOnEvent>> GetNoteGroups()
    {
        return noteGroups;
    }

    public List<NoteOnEvent> GetNoteList(int noteNum){
        return noteGroups.get(noteNum);
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
            noteGroups.get(usedNotes[i]).clear();
            noteUsedMark[usedNotes[i]] = false;
        }
        usedNoteCount = 0;
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

    public VisualNotes()
    {
        for(int i=0; i<128; i++) {
            noteUsedMark[i] = false;
            noteGroups.add(new ArrayList<>());
            noteOnTrackGroups.add(new int[200]);
        }

    }

}