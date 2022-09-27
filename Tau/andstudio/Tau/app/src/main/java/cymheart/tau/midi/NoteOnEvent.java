package cymheart.tau.midi;

import org.json.JSONException;

public class NoteOnEvent extends MidiEvent {

    /**手指编号提示*/
    public String fingerNumHits;

    // 结束tick
    public int endTick = 0;
    // 音符
    public int note = 0;
    // 力度
    public int velocity = 0;


    private NoteOnEvent()
    {
        type = MidiEvent.NoteOn;
    }

    @Override
    public void SetPlayType(int type)
    {
        super.SetPlayType(type);
        ndkSetPlayType(ndkNoteOffEvent, playType);
    }


    @Override
    public void SetByInnerJson()
    {
        super.SetByInnerJson();

        try {
            fingerNumHits = jsonMidiEvent.getString("FingerNumHits");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void SetInnerJson()
    {
        super.SetInnerJson();

        try {
            jsonMidiEvent.put("FingerNumHits", fingerNumHits);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private long ndkNoteOffEvent;
}
