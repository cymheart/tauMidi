package cymheart.tau.mididevice;

import android.media.midi.MidiDeviceInfo;

public class MidiPortWrapper
{
    MidiDeviceInfo deviceInfo;
    int portIndex;
    int portType = MidiDeviceInfo.PortInfo.TYPE_OUTPUT;

    public MidiPortWrapper(MidiDeviceInfo deviceInfo, int portIndex, int portType)
    {
        this.deviceInfo = deviceInfo;
        this.portIndex = portIndex;
        this.portType = portType;
    }

    public MidiDeviceInfo getDeviceInfo() {
        return deviceInfo;
    }
    public int getPortIndex() {
        return portIndex;
    }

    @Override
    public int hashCode() {
        int hashCode = 1;
        hashCode = 31 * hashCode + portIndex;
        hashCode = 31 * hashCode + portType;
        hashCode = 31 * hashCode + deviceInfo.hashCode();
        return hashCode;
    }
}
