package cymheart.ventrue;

import cymheart.ventrue.effect.Effect;

public class VentrueCmd {

    private Ventrue ventrue;

    private VentrueCmd(Ventrue _ventrue)
    {
        ventrue = _ventrue;
        ndkVentrueCmd = ndkCreateVentrueCmd(ventrue.GetNdkVentrue());
    }

    public void AppendMidiFile(String midifile)
    {
        ndkAppendMidiFile(ndkVentrueCmd, midifile);
    }

    public MidiPlay LoadMidi(int idx, boolean isShowTips)
    {
        MidiPlay midiPlay = ndkLoadMidi(ndkVentrueCmd, idx, isShowTips);
        if(isShowTips)
            ventrue.AddMidiPlay(idx, midiPlay);

        return midiPlay;
    }

    public void PlayMidi(int idx)
    {
        ndkPlayMidi(ndkVentrueCmd, idx);
    }

    public void StopMidi(int idx)
    {
        ndkStopMidi(ndkVentrueCmd, idx);
    }

    public void RemoveMidi(int idx)
    {
        ndkRemoveMidi(ndkVentrueCmd, idx);
    }

    public void MidiGoto(int idx, float sec)
    {
        ndkMidiGoto(ndkVentrueCmd, idx, sec);
    }


    public void AddEffect(Effect effect)
    {
        ndkAddEffect(ndkVentrueCmd, effect.GetNdkEffect());
    }


    //添加替换乐器
    public void AppendReplaceInstrument(
            int orgBankMSB, int orgBankLSB, int orgInstNum,
            int repBankMSB, int repBankLSB, int repInstNum)
    {
        ndkAppendReplaceInstrument(ndkVentrueCmd,
                orgBankMSB, orgBankLSB, orgInstNum,
                repBankMSB, repBankLSB, repInstNum);
    }

    //移除替换乐器
    public void RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum)
    {
        ndkRemoveReplaceInstrument(ndkVentrueCmd,
                orgBankMSB, orgBankLSB, orgInstNum);
    }


    public void OnKey(int key, float velocity, VirInstrument virInst)
    {
        OnKey(ndkVentrueCmd, key, velocity, virInst.GetNdkVirInstrument());
    }

    // 释放按键
    public void OffKey(int key, float velocity, VirInstrument virInst)
    {
        OffKey(ndkVentrueCmd, key, velocity, virInst.GetNdkVirInstrument());
    }

    public VirInstrument NewVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
    {
        VirInstrument virInst = ndkNewVirInstrument(
                ndkVentrueCmd, bankSelectMSB, bankSelectLSB, instrumentNum);
        return virInst;
    }

    /**
     *     在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在，将在虚拟乐器列表中自动创建它
     *     注意如果deviceChannelNum已经被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
     *     而不会同时在一个通道上创建超过1个的虚拟乐器
     *     <param name="deviceChannel">乐器所在的设备通道</param>
     *     <param name="bankSelectMSB">声音库选择0</param>
     *     <param name="bankSelectLSB">声音库选择1</param>
     *    <param name="instrumentNum">乐器编号</param>
     * */
    public VirInstrument EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
    {
        VirInstrument virInst = ndkEnableVirInstrument(
                ndkVentrueCmd, deviceChannelNum, bankSelectMSB, bankSelectLSB, instrumentNum);
        return virInst;
    }

    /// <summary>
    /// 移除乐器
    /// </summary>
    public void RemoveVirInstrument(VirInstrument virInst, boolean isFade)
    {
        ndkRemoveVirInstrument(ndkVentrueCmd, virInst.GetNdkVirInstrument(), isFade);
    }

    /// <summary>
    /// 打开乐器
    /// </summary>
    public void OnVirInstrument(VirInstrument virInst, boolean isFade)
    {
        ndkOnVirInstrument(ndkVentrueCmd, virInst.GetNdkVirInstrument(), isFade);
    }

    /// <summary>
    /// 关闭虚拟乐器
    /// </summary>
    public void OffVirInstrument(VirInstrument virInst, boolean isFade)
    {
        ndkOffVirInstrument(ndkVentrueCmd, virInst.GetNdkVirInstrument(), isFade);
    }

    //
    private long ndkVentrueCmd;
    public long GetNdkVentrueCmd()
    {
        return ndkVentrueCmd;
    }
    //
    private static native long ndkCreateVentrueCmd(long ndkVentrue);
    private static native void ndkAppendMidiFile(long ndkVentrueCmd, String midifile);
    private static native MidiPlay ndkLoadMidi(long ndkVentrueCmd, int idx, boolean isShowTips);
    private static native void ndkPlayMidi(long ndkVentrueCmd, int idx);
    private static native void ndkStopMidi(long ndkVentrueCmd, int idx);
    private static native void ndkRemoveMidi(long ndkVentrueCmd, int idx);
    private static native void ndkMidiGoto(long ndkVentrueCmd, int idx, float sec);

    private static native void ndkAddEffect(long ndkVentrueCmd, long ndkEffect);

    private static native void ndkAppendReplaceInstrument(long ndkVentrueCmd,
            int orgBankMSB, int orgBankLSB, int orgInstNum,
            int repBankMSB, int repBankLSB, int repInstNum);

    private static native void ndkRemoveReplaceInstrument(long ndkVentrueCmd,
                                                          int orgBankMSB, int orgBankLSB, int orgInstNum);

    private static native void OnKey(long ndkVentrueCmd,
                                     int key, float velocity, long ndkVirInst);

    private static native void OffKey(long ndkVentrueCmd,
                                     int key, float velocity, long ndkVirInst);

    private static native VirInstrument ndkNewVirInstrument(
            long ndkVentrueCmd, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

    private static native VirInstrument ndkEnableVirInstrument(
            long ndkVentrueCmd,
            int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

    private static native void ndkRemoveVirInstrument(long ndkVentrueCmd, long ndkVirInstrument, boolean isFade);
    private static native void ndkOnVirInstrument(long ndkVentrueCmd, long ndkVirInstrument, boolean isFade);
    private static native void ndkOffVirInstrument(long ndkVentrueCmd, long ndkVirInstrument, boolean isFade);
}
