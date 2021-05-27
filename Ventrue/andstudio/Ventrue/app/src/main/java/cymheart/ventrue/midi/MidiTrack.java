package cymheart.ventrue.midi;


public class MidiTrack {
   protected  MidiEvent[] midiEvents;
   public int eventOffsetIdx = 0;
   public int eventFristIdx = 0;
   public float curtSec = 0;
   public boolean isEnded = false;

   public MidiEvent[] GetMidiEvents() {
      return midiEvents;
   }
}
