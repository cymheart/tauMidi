package cymheart.tau;

import cymheart.tau.midi.MidiControllerType;

public interface MidiDeviceEventInterface {
    void OnKey(int key, float velocity, VirInstrument inst);
    void OffKey(int key, float velocity, VirInstrument inst);
    void ProgramChanges( VirInstrument inst, int num);
    void PitchBend(VirInstrument inst, int value);
    void SetMidiController(VirInstrument inst, MidiControllerType type, int value);
    void SetMasterVolume(float volume);
}
