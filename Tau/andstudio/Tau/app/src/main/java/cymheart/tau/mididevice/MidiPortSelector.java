package cymheart.tau.mididevice;

import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiDeviceStatus;
import android.media.midi.MidiManager;
import android.media.midi.MidiManager.DeviceCallback;
import android.os.Handler;
import android.os.Looper;

import java.util.ArrayList;
import java.util.List;

import cymheart.tau.utils.Utils;

public abstract class MidiPortSelector extends DeviceCallback
{
    protected MidiManager mMidiManager;
    private int mType = MidiDeviceInfo.PortInfo.TYPE_OUTPUT;
    protected List<MidiPortWrapper> devicePorts = new ArrayList<>();

    public Utils.Action<MidiDeviceInfo> OnDeviceAdded;
    public Utils.Action<MidiDeviceInfo> OnDeviceRemoved;

    public MidiPortSelector(MidiManager midiManager, int portType)
    {
        mMidiManager = midiManager;
        mType = portType;

        mMidiManager.registerDeviceCallback(this,
                new Handler(Looper.getMainLooper()));

        MidiDeviceInfo[] infos = mMidiManager.getDevices();
        for (MidiDeviceInfo info : infos) {
            onDeviceAdded(info);
        }
    }

    private int GetInfoPortCount(final MidiDeviceInfo info) {
        int portCount = (mType == MidiDeviceInfo.PortInfo.TYPE_INPUT)
                ? info.getInputPortCount() : info.getOutputPortCount();
        return portCount;
    }

    public void onDeviceAdded( MidiDeviceInfo info ) {
        int portCount = GetInfoPortCount(info);
        for (int i = 0; i < portCount; ++i) {
            MidiPortWrapper devicePort = new MidiPortWrapper(info, i, mType);
            devicePorts.add(devicePort);

            if(OnDeviceAdded != null)
                OnDeviceAdded.Execute(info);
        }
    }
    public void onDeviceRemoved( MidiDeviceInfo info ) {
        int portCount = GetInfoPortCount(info);
        for (int i = 0; i < portCount; ++i) {
            MidiPortWrapper devicePort = new MidiPortWrapper(info, i, mType);
            devicePorts.remove(devicePort);

            if(OnDeviceRemoved != null)
                OnDeviceRemoved.Execute(info);
        }
    }

    public void onDeviceStatusChanged(MidiDeviceStatus status)
    {
    }

    public void PortSelect(int idx)
    {
        if(idx >= devicePorts.size())
            return;
        onPortSelected(devicePorts.get(idx));
    }

    public abstract void onPortSelected(MidiPortWrapper wrapper);

    public abstract void onClose();

    public void close() {
        onClose();
    }
}
