package cymheart.tau.midi;

public class NoteOffEvent extends MidiEvent{
    // 音符
    public int note = 0;
    // 力度
    public int velocity = 0;

    NoteOffEvent()
    {
        type = MidiEvent.NoteOff;
    }
}
