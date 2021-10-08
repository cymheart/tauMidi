package cymheart.tau.utils;

public final class Utils
{
    public interface Action0{ public abstract void Execute();}
    public interface Action<T>{ public abstract void Execute(T data);}
    public interface Action2<T1, T2>{ public abstract void Execute(T1 data1, T2 data2);}
    public interface Action2RetBool<T1, T2>{ public abstract boolean Execute(T1 data1, T2 data2);}
    public interface Action0Ret{ public abstract Object Execute();}
    public interface Action0RetF{ public abstract float Execute();}

    static public boolean IsContainIntValue(int[] values, int value)
    {
        for(int i=0; i<values.length; i++)
        {
            if(values[i] == value)
                return true;
        }
        return  false;
    }
}