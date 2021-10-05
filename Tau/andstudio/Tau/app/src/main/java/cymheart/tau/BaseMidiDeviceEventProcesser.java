package cymheart.tau;

import cymheart.tau.midi.MidiControllerType;

public class BaseMidiDeviceEventProcesser implements MidiDeviceEventInterface {

    protected Tau tau;
    public void SetTau(Tau tau){this.tau = tau;}

    public void OnKey(int key, float velocity, VirInstrument inst)
    {
        if(tau == null)
            return;

        tau.OnKey(key, velocity, inst);
    }

    public void OffKey(int key, float velocity, VirInstrument inst)
    {
        if(tau == null)
            return;

        tau.OffKey(key, velocity, inst);
    }

    @Override
    public void ProgramChanges(VirInstrument inst, int num)
    {
        if(tau == null)
            return;

        tau.SetVirInstrumentProgram(inst,0,0, num);
    }

    @Override
    public void PitchBend(VirInstrument inst, int value) {
        tau.SetVirInstrumentPitchBend(inst, value);
    }

    @Override
    public void SetMidiController(VirInstrument inst, MidiControllerType type, int value) {
        tau.SetVirInstrumentMidiControllerValue(inst, type, value);
    }


    @Override
    public void SetMasterVolume(float volume) {

    }
}