package cymheart.tau;

import java.util.List;
import java.util.Set;

import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.MidiTrack;
import cymheart.tau.midi.NoteOnEvent;

public class MidiEditor {

    //停止
    static public final int STOP = 0;
    //播放
    static public final int PLAY = 1;
    //暂停
    static public final int PAUSE = 2;

    //
    protected MidiTrack[] midiTracks;
    boolean isOpen = false;
    protected float endSec = 0;
    public String filePath;

    public float GetEndSec()
    {
        return endSec;
    }

    public void Reset()
    {
        for (int i = 0; i < midiTracks.length; i++) {
            midiTracks[i].eventOffsetIdx = 0;
            midiTracks[i].eventFristIdx = 0;
        }
    }

    //轨道运行
    public void TrackRun(float sec)
    {
        if (isOpen == false)
        {
            isOpen = true;
            Reset();
        }

        TrackPlayCore(sec);
    }

    protected void TrackPlayCore(float sec)
    {
        for (int i = 0; i < midiTracks.length; i++)
        {
            midiTracks[i].curtSec = sec;
            MidiEvent[] events = midiTracks[i].GetMidiEvents();
            if(events == null)
                continue;

            if(events[events.length - 1]!=null &&
                    events[events.length - 1].endSec <= sec) {
                midiTracks[i].eventOffsetIdx = events.length - 1;
            }
            else {
                int j = midiTracks[i].eventOffsetIdx;
                for (; j < events.length; j++) {
                    if (events[j] == null)
                        continue;

                    if (events[j].startSec > sec) {
                        midiTracks[i].eventOffsetIdx = j;
                        break;
                    }
                }
            }
        }
    }


    /**
     * 根据当前时间点，往后获取指定时长的按键事件
     * @param secWidth 时长
     * @param trackFilter 需要获取的轨道标号列表
     * @param outVisualKeyEvents 输出
     * */
    public void TakeKeyEvents(float secWidth, int[] trackFilter, VisualKeyEvents outVisualKeyEvents)
    {
        List<List<NoteOnEvent>> noteOnEvents = outVisualKeyEvents.GetAllKeyEvents();
        Set<Integer> noteSet = outVisualKeyEvents.GetNoteSet();

        for (int i = 0; i < midiTracks.length; i++)
        {
            if(trackFilter!=null && !IsContainTrack(trackFilter,i))
                continue;

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
                List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                noteEvlist.add(noteOnEvent);
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
                List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                noteEvlist.add(noteOnEvent);
                noteSet.add(noteOnEvent.note);

            }
        }
    }

    protected boolean IsContainTrack(int[] tracks, int track)
    {
        for(int i=0; i<tracks.length; i++)
        {
            if(tracks[i] == track)
                return true;
        }
        return  false;
    }

}
