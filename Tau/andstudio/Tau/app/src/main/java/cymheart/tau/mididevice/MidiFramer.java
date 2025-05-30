package cymheart.tau.mididevice;

import android.media.midi.MidiReceiver;

import java.io.IOException;

/**
 * Convert stream of arbitrary MIDI bytes into discrete messages.
 *
 * Parses the incoming bytes and then posts individual messages to the receiver
 * specified in the constructor. Short messages of 1-3 bytes will be complete.
 * System Exclusive messages may be posted in pieces.
 *
 * Resolves Running Status and interleaved System Real-Time messages.
 */
public class MidiFramer extends MidiReceiver {
    private MidiReceiver mReceiver;
    private byte[] mBuffer = new byte[3];
    private int mCount;
    private byte mRunningStatus;
    private int mNeeded;
    private boolean mInSysEx;

    public MidiFramer(MidiReceiver receiver) {
        mReceiver = receiver;
    }

    /*
     * @see android.midi.MidiReceiver#onSend(byte[], int, int, long)
     */
    @Override
    public void onSend(byte[] data, int offset, int count, long timestamp)
            throws IOException {
        int sysExStartOffset = (mInSysEx ? offset : -1);

        for (int i = 0; i < count; i++) {
            final byte currentByte = data[offset];
            final int currentInt = currentByte & 0xFF;
            if (currentInt >= 0x80) { // status byte?
                if (currentInt < 0xF0) { // channel message?
                    mRunningStatus = currentByte;
                    mCount = 1;
                    mNeeded = MidiConstants.getBytesPerMessage(currentByte) - 1;
                } else if (currentInt < 0xF8) { // system common?
                    if (currentInt == 0xF0 /* SysEx Start */) {
                        // Log.i(TAG, "SysEx Start");
                        mInSysEx = true;
                        sysExStartOffset = offset;
                    } else if (currentInt == 0xF7 /* SysEx End */) {
                        // Log.i(TAG, "SysEx End");
                        if (mInSysEx) {
                            mReceiver.send(data, sysExStartOffset,
                                    offset - sysExStartOffset + 1, timestamp);
                            mInSysEx = false;
                            sysExStartOffset = -1;
                        }
                    } else {
                        mBuffer[0] = currentByte;
                        mRunningStatus = 0;
                        mCount = 1;
                        mNeeded = MidiConstants.getBytesPerMessage(currentByte) - 1;
                    }
                } else { // real-time?
                    // Single byte message interleaved with other data.
                    if (mInSysEx) {
                        mReceiver.send(data, sysExStartOffset,
                                offset - sysExStartOffset, timestamp);
                        sysExStartOffset = offset + 1;
                    }
                    mReceiver.send(data, offset, 1, timestamp);
                }
            } else { // data byte
                if (!mInSysEx) {
                    mBuffer[mCount++] = currentByte;
                    if (--mNeeded == 0) {
                        if (mRunningStatus != 0) {
                            mBuffer[0] = mRunningStatus;
                        }
                        mReceiver.send(mBuffer, 0, mCount, timestamp);
                        mNeeded = MidiConstants.getBytesPerMessage(mBuffer[0]) - 1;
                        mCount = 1;
                    }
                }
            }
            ++offset;
        }

        // send any accumulatedSysEx data
        if (sysExStartOffset >= 0 && sysExStartOffset < offset) {
            mReceiver.send(data, sysExStartOffset,
                    offset - sysExStartOffset, timestamp);
        }
    }

}
