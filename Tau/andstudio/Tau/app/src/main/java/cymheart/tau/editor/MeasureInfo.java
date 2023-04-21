package cymheart.tau.editor;

/**小节信息*/
public class MeasureInfo
{
    protected float[] measure;
    protected float[] beat;

    //小节结束位置
    protected int mIdx = 0;
    //拍子结束位置
    protected int bIdx = 0;
    //小节数量
    protected int measureNum = 0;

    //所在小节
    protected int atMeasure = 1;

    //获取当前时间点所在小节
    public int GetAtMeasure()
    {
        return atMeasure;
    }

    public void Clear()
    {
        mIdx = 0;
        bIdx = 0;
        measureNum = 0;
        atMeasure = 1;
    }


    //获取小节数量
    public int GetMeasureCount()
    {
        return measureNum;
    }

    //获取指定小节的开始时间点
    public float GetMeasureStartSec(int i)
    {
        i = Math.min(measureNum, i) - 1;
        return measure[i * 2];
    }

    //获取指定小节的结束时间点
    public float GetMeasureEndSec(int i)
    {
        i = Math.min(measureNum, i);
        if (i == measureNum)
            return beat[bIdx];

        return measure[i * 2];
    }

    //获取小节拍子数量
    public int GetMeasureBeatCount(int i)
    {
        i = Math.min(measureNum, i);
        if (i == measureNum) {
            int a = (int)measure[(i - 1) * 2 + 1];
            return bIdx - a + 1;
        }

        //
        int a = (int)measure[(i - 1) * 2 + 1];
        int b = (int)measure[i * 2 + 1];
        return b - a + 1;

    }

    //获取小节指定拍子的结束时间点
    public float GetMeasureBeatEndSec(int measureIdx, int beatIdx)
    {
        int a = (int)measure[(measureIdx - 1) * 2 + 1];
        return beat[a + (beatIdx - 1)];
    }

    //获取指定时间所在的小节
    public int GetSecAtMeasure(float sec)
    {
        int left = 0, right = measureNum - 1;
        int curt = 0;

        while (true)
        {
            curt = (right + left) / 2;
            float s = measure[curt * 2];
            if (s > sec) right = curt;
            else if (s < sec) left = curt;
            else return curt + 1;

            if (left + 1 == right || left == right)
                return left + 1;
        }
    }


}
