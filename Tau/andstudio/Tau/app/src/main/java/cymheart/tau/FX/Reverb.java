package cymheart.tau.FX;

public class Reverb extends Effect {

    public Reverb()
    {
        ndkEffect = ndkCreateReverb();
    }

    /**Set the room size (comb filter feedback gain) parameter [0,1].*/
    public void SetRoomSize(float value)
    {
        ndkSetRoomSize(ndkEffect, value);
    }

    /**Set the width (left-right mixing) parameter [0,1].*/
    public void SetWidth(float value)
    {
        ndkSetWidth(ndkEffect, value);
    }

    /** Set the damping parameter [0=low damping, 1=higher damping].*/
    public void SetDamping(float value)
    {
        ndkSetDamping(ndkEffect, value);
    }

    /** Set the effect mix [0 = mostly dry, 1 = mostly wet].*/
    public void SetEffectMix(float value)
    {
        ndkSetEffectMix(ndkEffect, value);
    }

    private static native long ndkCreateReverb();
    private static native void ndkSetRoomSize(long ndkEffect, float value);
    private static native void ndkSetWidth(long ndkEffect, float value);
    private static native void ndkSetDamping(long ndkEffect, float value);
    private static native void ndkSetEffectMix(long ndkEffect, float value);

}
