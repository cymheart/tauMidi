package cymheart.tau.FX;

import cymheart.tau.utils.dsignal.Filter;

public class Equalizer extends Effect {
    public Equalizer()
    {
        ndkEffect = ndkCreateEqualizer();
    }
    public Filter[] GetFilter()
    {
        return ndkGetFilter(ndkEffect);
    }

    public void SetFreqBandGain(int bandIdx, float gainDB)
    {
        ndkSetFreqBandGain(ndkEffect, bandIdx, gainDB);
    }

    private static native long ndkCreateEqualizer();
    private static native Filter[] ndkGetFilter(long ndkEffect);
    private static native void ndkSetFreqBandGain(long ndkEffect, int bandIdx, float gainDB);
}
