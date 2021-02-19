package cymheart.ventrue.effect;

import cymheart.ventrue.dsignal.Filter;

public class Equalizer extends Effect {
    public Equalizer()
    {
        ndkEffect = ndkCreateEqualizer();
    }
    public Filter[] GetFilter()
    {
        return ndkGetFilter(ndkEffect);
    }

    private static native long ndkCreateEqualizer();
    private static native Filter[] ndkGetFilter(long ndkEffect);
}
