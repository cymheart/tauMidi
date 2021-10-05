package cymheart.tau;

import android.media.midi.MidiReceiver;

import java.util.Map;

import cymheart.tau.midi.MidiControllerType;

public class TauMidiReceiver extends MidiReceiver {

    enum MidiEventType
    {
        None,
        NoteOff,
        NoteOn,
        PolyTouch,
        Control,
        Program,
        Pressure,
        Bend,
        SysEx,       // F0
        TimeCode,    // F1
        SongPos,     // F2
        SongSel,     // F3
        F4,          // F4
        F5,          // F5
        TuneReq,     // F6
        EndSysex,    // F7
        TimingClock, // F8
        F9,          // F9
        Start,       // FA
        Continue,    // FB
        Stop,        // FC
        FD,          // FD
        ActiveSensing, // FE
        Reset        // FF
    }

    public static final MidiEventType[] CHANNEL_COMMAND_NAMES = {
            MidiEventType.NoteOff, MidiEventType.NoteOn,
            MidiEventType.PolyTouch, MidiEventType.Control,
            MidiEventType.Program, MidiEventType.Pressure, MidiEventType.Bend
    };

    public static final MidiEventType[] SYSTEM_COMMAND_NAMES = {
            MidiEventType.SysEx, // F0
            MidiEventType.TimeCode,    // F1
            MidiEventType.SongPos,     // F2
            MidiEventType.SongSel,     // F3
            MidiEventType.F4,          // F4
            MidiEventType.F5,          // F5
            MidiEventType.TuneReq,     // F6
            MidiEventType.EndSysex,    // F7
            MidiEventType.TimingClock, // F8
            MidiEventType.F9,          // F9
            MidiEventType.Start,       // FA
            MidiEventType.Continue,    // FB
            MidiEventType.Stop,        // FC
            MidiEventType.FD,          // FD
            MidiEventType.ActiveSensing, // FE
            MidiEventType.Reset        // FF
    };

    public Map<Integer, MidiControllerType> midiCotrollerTypeMap = MidiControllerType.buildMap();

    public MidiDeviceEventInterface eventProcesser;

    private Tau tau;

    public TauMidiReceiver(Tau tau) {
        this.tau = tau;
    }


    public MidiEventType getEventType(int status) {
        if (status >= 0xF0) {
            int index = status & 0x0F;
            return SYSTEM_COMMAND_NAMES[index];
        } else if (status >= 0x80) {
            int index = (status >> 4) & 0x07;
            return CHANNEL_COMMAND_NAMES[index];
        } else {
            return MidiEventType.None;
        }
    }

    @Override
    public void onSend(byte[] data, int offset, int count, long timestamp)
    {
        VirInstrument inst = tau.GetMidiDeviceConnectedInst();

        byte statusByte = data[offset++];
        int status = statusByte & 0xFF;

        MidiEventType eventType = getEventType(status);
      //  int numData = MidiConstants.getBytesPerMessage(statusByte) - 1;
        int key;
        int velocity;

        switch (eventType) {
            case NoteOn:
                key = data[offset++];
                velocity = data[offset];
                eventProcesser.OnKey(key, velocity, inst);
                break;

            case NoteOff:
                key = data[offset++];
                velocity = data[offset];
                eventProcesser.OffKey(key, velocity, inst);
                break;

            case Program:
                eventProcesser.ProgramChanges(inst, data[offset]);
                break;

            case Bend:
                eventProcesser.PitchBend(inst, data[2]<<7 | data[1]);
                break;

            case Control:
                MidiControllerType type = MidiControllerType.CC_None;
                key = data[1] & 0xff;
                if(midiCotrollerTypeMap.containsKey(key))
                    type = midiCotrollerTypeMap.get(key);
                eventProcesser.SetMidiController(inst, type, data[2]);
                break;

            case SysEx:
                int volume = GetMasterVolume(data, offset - 1, count);
                if(volume < 0)
                    break;
                eventProcesser.SetMasterVolume((float)volume/127.0f);
                break;

        }
    }

    //获取主音量
    protected  int GetMasterVolume(byte[] data, int offset, int count)
    {
        if(count == 8) {
            int[] masterVolumeCmd = {0xF0, 0x7F, 0x7F, 0x04, 0x01,0x00, 0x88, 0xF7};
            for (int i = 0; i < count; i++) {
                if(i == 6)
                    continue;
                if(masterVolumeCmd[i] != (data[offset + i] & 0xff))
                    return -1;
            }
        }

        return data[offset + 6];
    }

}