package cymheart.tau;

import android.content.Context;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;

import cymheart.tau.editor.Editor;
import cymheart.tau.effect.Effect;
import cymheart.tau.midi.MidiControllerType;
import cymheart.tau.mididevice.MidiFramer;
import cymheart.tau.mididevice.MidiOutputPortSelector;
import cymheart.tau.utils.Utils;


public class Tau
{
//    static {
//        System.loadLibrary("tau");
//    }

    protected Context context;
    protected MidiManager midiManager;
    protected TauMidiReceiver midiReceiver;
    protected MidiOutputPortSelector outputPortSelector;
    protected VirInstrument midiDeviceConnectedInst;
    protected BaseMidiDeviceEventProcesser midiDeviceEventProcesser = new BaseMidiDeviceEventProcesser();

    //
    protected Editor editor = new Editor();
    public Editor GetEditor() {
        return editor;
    }

    public Tau(Context context)
    {
        this.context = context;
        ndkTau = ndkCreateTau(this, editor);

        // Setup MIDI
        midiDeviceEventProcesser.SetTau(this);
        midiManager = (MidiManager)context.getSystemService(Context.MIDI_SERVICE);
        midiReceiver = new TauMidiReceiver(this);
        midiReceiver.eventProcesser = midiDeviceEventProcesser;
        MidiFramer connectFramer = new MidiFramer(midiReceiver);
        outputPortSelector = new MidiOutputPortSelector(midiManager);
        outputPortSelector.getSender().connect(connectFramer);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        Destory();
    }

    public void Destory()
    {
        outputPortSelector.onClose();
    }

    public void Open()
    {
        ndkOpen(ndkTau);
    }

    public void Close()
    {
        ndkClose(ndkTau);
    }

    public void SetSoundFont(SoundFont sf)
    {
        ndkSetSoundFont(ndkTau, sf.GetNdkSoundFont());
    }

    public void SetUseMulThread(boolean isUse)
    {
        ndkSetUseMulThread(ndkTau, isUse);
    }

    public void SetFrameSampleCount(int sampleCount)
    {
        ndkSetFrameSampleCount(ndkTau, sampleCount);
    }

    public void SetSampleProcessRate(int sampleRate)
    {
        ndkSetSampleProcessRate(ndkTau, sampleRate);
    }


    //设置声道模式(立体声，单声道设置)
    public void SetChannelCount(int channelCount)
    {
        ndkSetChannelCount(ndkTau, channelCount);
    }

    //设置单位处理midi轨道数量
    public void SetUnitProcessMidiTrackCount(int count)
    {
        ndkSetUnitProcessMidiTrackCount(ndkTau, count);
    }

    //获取所有发声区域的数量
    public int GetTotalRegionSounderCount()
    {
        return 0;
       // return ndkGetTotalRegionSounderCount(ndkTau);
    }

    public void SetMidiDeviceEventProcesser(MidiDeviceEventInterface eventProcesser)
    {
        midiReceiver.eventProcesser = eventProcesser;
    }

    public void ConntectMidiDevice(int idx)
    {
        outputPortSelector.PortSelect(idx);
    }

    public void SetMidiDeviceAddedCallBack(Utils.Action<MidiDeviceInfo> onDeviceAdded)
    {
        outputPortSelector.OnDeviceAdded = onDeviceAdded;
    }

    public void SetMidiDeviceRemovedCallBack(Utils.Action<MidiDeviceInfo> onDeviceRemoved)
    {
        outputPortSelector.OnDeviceRemoved = onDeviceRemoved;
    }

    //设置midi外设连接的发音乐器
    public void SetMidiDeviceConnectedInstrument(VirInstrument inst)
    {
        midiDeviceConnectedInst = inst;
    }

    public VirInstrument GetMidiDeviceConnectedInst()
    {
        return midiDeviceConnectedInst ;
    }

    //设置极限发声区域数量(默认值:600)
    //当播放有卡顿现象时，把这个值调小，会提高声音的流畅度
    public void SetLimitRegionSounderCount(int count)
    {
        ndkSetLimitRegionSounderCount(ndkTau, count);
    }

    //设置极限按键速率(默认值:800)
    //当播放有卡顿现象时，把这个值调小，会提高声音的流畅度
    public void SetLimitOnKeySpeed(float speed)
    {
        ndkSetSetLimitOnKeySpeed(ndkTau, speed);
    }


    public void Load(String midifile)
    {
        editor.Load(midifile);
    }

    public void Play()
    {
        editor.Play();

//        //创建对象，及属性赋值
      //  LoginInfo.Login.Builder builder = LoginInfo.Login.newBuilder();
//
       // builder.setAccount("Mrzhang")
        //        .setPassword("18");
//
      //  LoginInfo.Login login = builder.build();
//
////序列化(通过protobuf生成的java类的内部方法进行序列化)
//        byte[] bytes = login.toByteArray();
//
////反序列化(通过protobuf生成的java类的内部方法进行反序列化)
//        try {
//            LoginInfo.Login parseFrom = LoginInfo.Login.parseFrom(bytes);
//        } catch (InvalidProtocolBufferException e) {
//            e.printStackTrace();
//        }

    }

    public void Pause()
    {
        editor.Pause();
    }

    public void Stop()
    {
        editor.Stop();
    }

    public void Remove()
    {
        editor.Remove();
    }

    public void Goto(double sec)
    {
        editor.Goto(sec);
    }

    public int GetState()
    {
        return 0;
    }

    // 禁止播放指定编号Midi文件的轨道
    public void DisableTrack(int trackIdx)
    {

    }

    // 禁止播放Midi的所有轨道
    public void DisableAllTrack() {

    }

    // 启用播放指定编号Midi文件的轨道
    public void EnableTrack(int trackIdx) {

    }

    // 启用播放Midi的所有轨道
    public void EnableAllTrack()
    {

    }


    public void AddEffect(Effect effect)
    {
        ndkAddEffect(ndkTau, effect.GetNdkEffect());
    }

    //设置是否开启所有乐器效果器
    public void SetEnableAllVirInstEffects(boolean isEnable)
    {
        ndkSetEnableAllVirInstEffects(ndkTau, isEnable);
    }

    //添加替换乐器
    public void AppendReplaceInstrument(
            int orgBankMSB, int orgBankLSB, int orgInstNum,
            int repBankMSB, int repBankLSB, int repInstNum)
    {
        ndkAppendReplaceInstrument(ndkTau,
                orgBankMSB, orgBankLSB, orgInstNum,
                repBankMSB, repBankLSB, repInstNum);
    }

    //移除替换乐器
    public void RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum)
    {
        ndkRemoveReplaceInstrument(ndkTau,
                orgBankMSB, orgBankLSB, orgInstNum);
    }

    public void OnKey(int key, float velocity, VirInstrument virInst)
    {
        ndkOnKey(ndkTau, key, velocity, virInst.GetNdkVirInstrument(), 0);
    }

    public void OnKey(int key, float velocity, VirInstrument virInst, int delayMS)
    {
        ndkOnKey(ndkTau, key, velocity, virInst.GetNdkVirInstrument(), delayMS);
    }

    // 释放按键
    public void OffKey(int key, float velocity, VirInstrument virInst)
    {
        ndkOffKey(ndkTau, key, velocity, virInst.GetNdkVirInstrument(), 0);
    }

    // 释放按键
    public void OffKey(int key, float velocity, VirInstrument virInst, int delayMS)
    {
        ndkOffKey(ndkTau, key, velocity, virInst.GetNdkVirInstrument(), delayMS);
    }


    // 取消按键
    public void CancelDownKey(int key, float velocity, VirInstrument virInst, int delayMS)
    {
        ndkCancelOnKey(ndkTau, key, velocity, virInst.GetNdkVirInstrument(), delayMS);
    }

    // 取消释放按键
    public void CancelOffKey(int key, float velocity, VirInstrument virInst, int delayMS)
    {
        ndkCancelOffKey(ndkTau, key, velocity, virInst.GetNdkVirInstrument(), delayMS);
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
                ndkTau, deviceChannelNum, bankSelectMSB, bankSelectLSB, instrumentNum);
        return virInst;
    }

    public void SetVirInstrumentMidiControllerValue(VirInstrument virInst, MidiControllerType type, int value)
    {
        ndkSetVirInstrumentMidiControllerValue(ndkTau, virInst.GetNdkVirInstrument(), type.getId(), value);
    }

    public void SetVirInstrumentPitchBend(VirInstrument virInst, int value)
    {
        ndkSetVirInstrumentPitchBend(ndkTau, virInst.GetNdkVirInstrument(), value);
    }

    public void SetVirInstrumentProgram(VirInstrument virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
    {
        ndkSetVirInstrumentProgram(ndkTau, virInst.GetNdkVirInstrument(), bankSelectMSB, bankSelectLSB, instrumentNum);
    }

    /// <summary>
    /// 移除乐器
    /// </summary>
    public void RemoveVirInstrument(VirInstrument virInst, boolean isFade)
    {
        ndkRemoveVirInstrument(ndkTau, virInst.GetNdkVirInstrument(), isFade);
    }

    /// <summary>
    /// 打开乐器
    /// </summary>
    public void OnVirInstrument(VirInstrument virInst, boolean isFade)
    {
        ndkOnVirInstrument(ndkTau, virInst.GetNdkVirInstrument(), isFade);
    }

    /// <summary>
    /// 关闭虚拟乐器
    /// </summary>
    public void OffVirInstrument(VirInstrument virInst, boolean isFade)
    {
        ndkOffVirInstrument(ndkTau, virInst.GetNdkVirInstrument(), isFade);
    }



    private long ndkTau;
    public long GetNdkTau()
    {
        return ndkTau;
    }

    //
    private static native long ndkCreateTau(Tau tau, Editor editor);
    private static native void ndkOpen(long ndkTau);
    private static native void ndkClose(long ndkTau);
    private static native void ndkSetSoundFont(long ndkTau, long ndkSoundFont);
    private static native void ndkSetUseMulThread(long ndkTau, boolean isUse);
    private static native void ndkSetFrameSampleCount(long ndkTau, int sampleCount);
    private static native void ndkSetSampleProcessRate(long ndkTau, int sampleRate);
    private static native void ndkSetChannelCount(long ndkTau, int channelCount);
    private static native void ndkSetUnitProcessMidiTrackCount(long ndkTau, int count);
    private static native void ndkSetLimitRegionSounderCount(long ndkTau, int count);
    private static native void ndkSetSetLimitOnKeySpeed(long ndkTau, float speed);

    private static native void ndkAddEffect(long ndkTau, long ndkEffect);
    private static native void ndkSetEnableAllVirInstEffects(long ndkTau, boolean isEnable);


    private static native void ndkAppendReplaceInstrument(long ndkTau,
                                                          int orgBankMSB, int orgBankLSB, int orgInstNum,
                                                          int repBankMSB, int repBankLSB, int repInstNum);

    private static native void ndkRemoveReplaceInstrument(long ndkTau,
                                                          int orgBankMSB, int orgBankLSB, int orgInstNum);

    private static native void ndkOnKey(long ndkTau,
                                     int key, float velocity, long ndkVirInst, int delayMS);

    private static native void ndkOffKey(long ndkTau,
                                      int key, float velocity, long ndkVirInst, int delayMS);

    private static native void ndkCancelOnKey(long ndkTau,
                                        int key, float velocity, long ndkVirInst, int delayMS);

    private static native void ndkCancelOffKey(long ndkTau,
                                         int key, float velocity, long ndkVirInst, int delayMS);

    private static native VirInstrument ndkEnableVirInstrument(
            long ndkTau,
            int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

    private static native void ndkSetVirInstrumentProgram(
            long ndkTau,
            long ndkVirInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

    private static native void ndkSetVirInstrumentPitchBend(long ndkTau, long ndkVirInst, int value);

    private static native void ndkSetVirInstrumentMidiControllerValue(long ndkTau, long ndkVirInst, int type, int value);

    private static native void ndkRemoveVirInstrument(long ndkTau, long ndkVirInstrument, boolean isFade);
    private static native void ndkOnVirInstrument(long ndkTau, long ndkVirInstrument, boolean isFade);
    private static native void ndkOffVirInstrument(long ndkTau, long ndkVirInstrument, boolean isFade);
}
