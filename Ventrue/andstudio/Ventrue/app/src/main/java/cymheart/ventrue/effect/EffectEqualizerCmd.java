package cymheart.ventrue.effect;

import cymheart.ventrue.Ventrue;

public class EffectEqualizerCmd {

    private Ventrue ventrue;
    private Equalizer eq;

    public EffectEqualizerCmd(Ventrue ventrue, Equalizer eq)
    {
        this.ventrue = ventrue;
        this.eq = eq;
        ndkEffectEqCmd = ndkCreateEffectEqualizerCmd(ventrue.GetNdkVentrue(), eq.GetNdkEffect());
    }


    public void SetFreqBandGain(int bandIdx, float gainDB)
    {
        ndkSetFreqBandGain(ndkEffectEqCmd, bandIdx, gainDB);
    }


    private long ndkEffectEqCmd;
    private static native long ndkCreateEffectEqualizerCmd(long ndkVentrue, long ndkEqualizer);
    private static native void ndkSetFreqBandGain(long ndkEffectEqCmd, int bandIdx, float gainDB);
}
