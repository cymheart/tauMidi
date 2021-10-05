package cymheart.tau.midi;


public class MidiTrack {
   protected  MidiEvent[] midiEvents;
   protected  MidiEvent[][] midiEventsAtChannel =new MidiEvent[16][];

   public int eventOffsetIdx = 0;
   public int eventFristIdx = 0;
   public float curtSec = 0;
   public boolean isEnded = false;

   public MidiEvent[] GetMidiEvents() {
      return midiEvents;
   }
}
