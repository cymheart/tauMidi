package cymheart.tau.FX;

import cymheart.tau.utils.dsignal.Filter;

public class Effect {

    protected long ndkEffect;
    public long GetNdkEffect()
    {
        return ndkEffect;
    }

    public void Relese(){
        ndkRelease(this);
    }

    public void LockData()
    {
        ndkLockData(ndkEffect);

    }

    public void UnLockData()
    {
        ndkUnLockData(ndkEffect);
    }


    private static native long ndkRelease(Effect effect);


    private static native void ndkLockData(long ndkEffect);
    private static native void ndkUnLockData(long ndkEffect);
}
