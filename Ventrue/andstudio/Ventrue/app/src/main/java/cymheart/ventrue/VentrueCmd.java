package cymheart.ventrue;

import cymheart.ventrue.effect.Effect;

public class VentrueCmd {

    private Ventrue ventrue;

    public VentrueCmd(Ventrue _ventrue)
    {
        ventrue = _ventrue;
        ndkVentrueCmd = ndkCreateVentrueCmd(ventrue.GetNdkVentrue());
    }

    public void AppendMidiFile(String midifile)
    {
        ndkAppendMidiFile(ndkVentrueCmd, midifile);
    }

    public void PlayMidi(int midiFileIdx)
    {
        ndkPlayMidi(ndkVentrueCmd, midiFileIdx);
    }


    public void AddEffect(Effect effect)
    {
        ndkAddEffect(ndkVentrueCmd, effect.GetNdkEffect());
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
        VirInstrument virInst = (VirInstrument)ndkEnableVirInstrument(
                ndkVentrueCmd, deviceChannelNum, bankSelectMSB, bankSelectLSB, instrumentNum);
        return virInst;
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
    private static native void ndkPlayMidi(long ndkVentrueCmd, int midiFileIdx);

    private static native void ndkAddEffect(long ndkVentrueCmd, long ndkEffect);

    private static native void OnKey(long ndkVentrueCmd,
                                     int key, float velocity, long ndkVirInst);

    private static native void OffKey(long ndkVentrueCmd,
                                     int key, float velocity, long ndkVirInst);

    private static native Object ndkEnableVirInstrument(
            long ndkVentrueCmd,
            int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);
}
