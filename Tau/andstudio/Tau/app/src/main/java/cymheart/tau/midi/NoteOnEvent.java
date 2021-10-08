package cymheart.tau.midi;

public class NoteOnEvent extends MidiEvent {

    // 结束tick
    int endTick = 0;
    // 音符
    public int note = 0;
    // 力度
    public int velocity = 0;

    NoteOnEvent()
    {
        type = MidiEvent.NoteOn;
    }
}
