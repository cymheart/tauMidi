package cymheart.tau.mididevice;

import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.media.midi.MidiOutputPort;
import android.media.midi.MidiSender;
import android.util.Log;

import java.io.IOException;

public class MidiOutputPortSelector extends MidiPortSelector
{
    private MidiOutputPort mOutputPort;
    private MidiDispatcher mDispatcher = new MidiDispatcher();
    private android.media.midi.MidiDevice mOpenDevice;

    public MidiOutputPortSelector(MidiManager midiManager) {
        super(midiManager, MidiDeviceInfo.PortInfo.TYPE_OUTPUT);
    }


    @Override
    public void onPortSelected(final MidiPortWrapper wrapper) {
        close();
        final MidiDeviceInfo info = wrapper.getDeviceInfo();
        if (info != null) {
            mMidiManager.openDevice(info, device -> {
                if (device == null) {
                    Log.e(MidiConstants.TAG, "could not open " + info);
                } else {
                    mOpenDevice = device;
                    mOutputPort = device.openOutputPort(wrapper.getPortIndex());
                    if (mOutputPort == null) {
                        Log.e(MidiConstants.TAG,
                                "could not open output port for " + info);
                        return;
                    }
                    mOutputPort.connect(mDispatcher);
                }
            }, null);
        }
    }

    @Override
    public void onClose() {
        try {
            if (mOutputPort != null) {
                mOutputPort.disconnect(mDispatcher);
            }
            mOutputPort = null;
            if (mOpenDevice != null) {
                mOpenDevice.close();
            }
            mOpenDevice = null;
        } catch (IOException e) {
            Log.e(MidiConstants.TAG, "cleanup failed", e);
        }
    }

    /**
     * You can connect your MidiReceivers to this sender. The user will then select which output
     * port will send messages through this MidiSender.
     * @return a MidiSender that will send the messages from the selected port.
     */
    public MidiSender getSender() {
        return mDispatcher.getSender();
    }

}
