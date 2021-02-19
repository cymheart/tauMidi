package cymheart.ventrue.dsignal;

public class Filter {
    public Filter(){ndkFilter = ndkCreateFilter();}

    protected void finalize() {
        // 对象的清理工作
       // ndkReleaseFilter(ndkFilter);
    }

    private long ndkFilter;
    public long GetNdkFilter()
    {
        return ndkFilter;
    }

    //
    private static native long ndkCreateFilter();
    private static native void ndkReleaseFilter(long ndkFilter);
}
