package cymheart.ventrue;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import cymheart.ventrue.midi.MidiEvent;
import cymheart.ventrue.midi.MidiTrack;
import cymheart.ventrue.midi.NoteOnEvent;

public class MidiPlay {

    // MidiPlay状态
    enum MidiPlayState
    {
        //停止
        STOP,
        //播放
        PLAY,
        //暂停
        SUSPEND
    };

    protected MidiTrack[] midiTracks;
    boolean isOpen = false;
    boolean isGotoEnd = false;
    protected float gotoSec = 0;
    protected float endSec = 0;
    MidiPlayState state = MidiPlayState.STOP;


    public float GetEndSec()
    {
        return endSec;
    }

    public void GetKeyEventToMap(float secWidth, List<List<NoteOnEvent>> noteOnEvents, Set<Integer> noteSet)
    {
        for (int i = 0; i < midiTracks.length; i++)
        {
            float endSec = midiTracks[i].curtSec + secWidth;
            MidiEvent[] events = midiTracks[i].GetMidiEvents();
            int j = midiTracks[i].eventOffsetIdx - 1;
            int fristIdx = midiTracks[i].eventFristIdx;

            for(; j >= fristIdx; j--)
            {
                if(events[j] == null)
                    continue;

                if (events[j].endSec < midiTracks[i].curtSec)
                    continue;

                midiTracks[i].eventFristIdx = j;
                NoteOnEvent noteOnEvent = (NoteOnEvent)events[j];
                List<NoteOnEvent> list = noteOnEvents.get(noteOnEvent.note);
                list.add(noteOnEvent);
                noteSet.add(noteOnEvent.note);
            }

            j = midiTracks[i].eventOffsetIdx;
            for(; j<events.length; j++)
            {
                if(events[j] == null)
                    continue;

                if (events[j].startSec > endSec)
                    break;

                NoteOnEvent noteOnEvent = (NoteOnEvent)events[j];
                List<NoteOnEvent> list = noteOnEvents.get(noteOnEvent.note);
                list.add(noteOnEvent);
                noteSet.add(noteOnEvent.note);

            }
        }
    }

    //设置播放的起始时间点
    public void Goto(float gotoSec)
    {
        isOpen = false;
        isGotoEnd = false;
        this.gotoSec = gotoSec;
    }

    //轨道运行
    public void TrackRun(float sec)
    {
        //if (state != MidiPlayState.PLAY)
          //  return;

        if (isOpen == false)
        {
            isOpen = true;
            for (int i = 0; i < midiTracks.length; i++) {
                midiTracks[i].eventOffsetIdx = 0;
                midiTracks[i].eventFristIdx = 0;
            }

            if (gotoSec > 0)
            {
                TrackPlayCore(gotoSec + sec);
            }
        }

        TrackPlayCore(gotoSec + sec);
    }

    protected void TrackPlayCore(float sec)
    {
        for (int i = 0; i < midiTracks.length; i++)
        {
            if (midiTracks[i].isEnded)
                continue;

            midiTracks[i].curtSec = sec;
            MidiEvent[] events = midiTracks[i].GetMidiEvents();
            if(events == null)
                continue;

            int j = midiTracks[i].eventOffsetIdx;
            for(; j<events.length; j++)
            {
                if(events[j] == null)
                    continue;

                if (events[j].startSec > sec)
                {
                    midiTracks[i].eventOffsetIdx = j;
                    break;
                }
            }

            if(j == events.length)
                midiTracks[i].isEnded = true;
        }
    }

}
