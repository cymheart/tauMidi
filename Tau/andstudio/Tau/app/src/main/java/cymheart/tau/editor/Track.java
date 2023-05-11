package cymheart.tau.editor;




import cymheart.tau.Channel;
import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;


public class Track {

    public Editor editor;
    public int idx = 0;


    //是否禁止播放
    protected boolean isDisablePlay = false;

    // 通道
    protected Channel channel;
    public Channel GetChannel() {
        return channel;
    }

    public NoteOnEvent[] noteOnEvents;
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
        editor.SetTrackPlayType(idx, type);
    }

    protected int noteColor = MidiEvent.NoteColor_None;
    public int GetNoteColor() {
        return noteColor;
    }

    public void SetNoteColor(int color)
    {
        noteColor = color;
    }

    public void Clear()
    {
        noteOnEventsOffset = 0;
        noteOnEventsFirst = 0;
    }
}
