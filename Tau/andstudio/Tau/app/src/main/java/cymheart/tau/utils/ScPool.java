package cymheart.tau.utils;

import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class ScPool<T> {
    public interface AllocMethodCallBack<T> {
        T Execute(Object obj);
    }

    public interface DestroyMethodCallBack<T> {
        void Execute(List<T> objs);
    }

    public AllocMethodCallBack<T> NewMethod;
    public DestroyMethodCallBack<T> DestroyCB;
    public int createCount = 20;
    public Object newObjParam = null;

    String key;
    private List<T> objectList = new ArrayList<T>();
    private int pos = -1;
    private String name;

    /**
     * 当不在同一个线程进行pop,push操作时，会发生错误，需要加锁
     */
    private final ReentrantLock locker = new ReentrantLock();
    private boolean isUseOpLock = false;

    /**
     * 是否使用操作锁，以解决不在同一个线程进行pop,push操作时，会发生错误，需要加锁
     */
    public void SetUseOpLock(boolean isUse) {
        isUseOpLock = isUse;
    }

    public ScPool(String name) {
        this.name = name;
    }

    public void SetPerCreateCount(int count) {
        createCount = count;
    }

    public void CreatePool(int count) {
        T obj;
        for (int i = 0; i < count; i++) {
            obj = NewMethod.Execute(this.newObjParam);
            pos++;
            objectList.add(null);
            objectList.set(pos, obj);
        }
    }

    public T Pop() {
        if (isUseOpLock)
            locker.lock();

        if (pos < 0)
            CreatePool(createCount);

        T obj = objectList.get(pos);
        objectList.set(pos, null);
        pos--;

        if (isUseOpLock)
            locker.unlock();

        return obj;
    }


    public void Push(T obj) {
        if (isUseOpLock)
            locker.lock();

        pos++;
        if (pos >= objectList.size()) {
            Log.w("pool", "注意检查代码此处是否有obj在pool中重复被存取!");
            objectList.add(obj);
        } else {
            objectList.set(pos, obj);
        }

        if (isUseOpLock)
            locker.unlock();
    }

    public void Destory() {
        if (this.DestroyCB != null) {
            DestroyCB.Execute(objectList);
        }

        objectList.clear();
        this.pos = -1;
    }
}

