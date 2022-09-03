package cymheart.tau.mididevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.util.Log;

import java.io.IOException;



/**
 * Select an output port and connect it to a destination input port.
 */
public class MidiOutputPortConnectionSelector extends MidiPortSelector {

    private MidiPortConnector mSynthConnector;
    private MidiDeviceInfo mDestinationDeviceInfo;
    private int mDestinationPortIndex;
    private MidiPortConnector.OnPortsConnectedListener mConnectedListener;

    /**
     * @param midiManager
     */
    public MidiOutputPortConnectionSelector(MidiManager midiManager,
                                            MidiDeviceInfo destinationDeviceInfo, int destinationPortIndex) {
        super(midiManager, MidiDeviceInfo.PortInfo.TYPE_OUTPUT);
        mDestinationDeviceInfo = destinationDeviceInfo;
        mDestinationPortIndex = destinationPortIndex;
    }

    @Override
    public void onPortSelected(final MidiPortWrapper wrapper) {
        onClose();
        if (wrapper.getDeviceInfo() != null) {
            mSynthConnector = new MidiPortConnector(mMidiManager);
            mSynthConnector.connectToDevicePort(wrapper.getDeviceInfo(),
                    wrapper.getPortIndex(), mDestinationDeviceInfo,
                    mDestinationPortIndex,
                    // not safe on UI thread
                    mConnectedListener, null);
        }
    }

    @Override
    public void onClose() {
        try {
            if (mSynthConnector != null) {
                mSynthConnector.close();
                mSynthConnector = null;
            }
        } catch (IOException e) {
            Log.e(MidiConstants.TAG, "Exception in closeSynthResources()", e);
        }
    }


    public void setConnectedListener(
            MidiPortConnector.OnPortsConnectedListener connectedListener) {
        mConnectedListener = connectedListener;
    }
}
