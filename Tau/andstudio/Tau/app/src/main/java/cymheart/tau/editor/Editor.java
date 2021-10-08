package cymheart.tau.editor;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;

import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.Utils;

public class Editor {

    //停止
    static public final int STOP = 0;
    //播放
    static public final int PLAY = 1;
    //暂停
    static public final int PAUSE = 2;


    Utils.Action0RetF GetCurtSec = null;

    protected List<Track> tracks = new ArrayList<>();

    public List<Track> GetTracks() {
        return tracks;
    }

    protected boolean isOpen = false;
    protected boolean isDirectGoto = false;
    protected boolean isGotoEnd = false;
    protected int state = STOP;
    protected float gotoSec = 0;

    protected float speed = 1;
    protected float playStartSec = 0;
    protected float baseSpeedSec = 0;
    protected float curtPlaySec = 0;

    //结束时间点
    protected float endSec = 0;

    protected List<MidiEvent> curtProcessMidiEvent = new ArrayList<>();
    protected VisualMidiEvents visualMidiEvents = new VisualMidiEvents();

    public void Load(String midifile)
    {
        ndkLoad(this, ndkEditor, midifile);
        _Load();
    }

    private void _Load()
    {
        Track track;
        for(int i=0; i<_ndkTracks.length; i++)
        {
            track = _ndkTracks[i];
            tracks.add(track);

            Object[] a = (Object[])(_ndkInstFragmentArray[i]);

            for(int n = 0; n<a.length; n++)
            {
                InstFragment[] instFrags = (InstFragment[]) a[n];
                LinkedList<InstFragment> instFragList = new LinkedList<>();
                for (int j = 0; j < instFrags.length; j++) {
                    instFrags[j].midiEvents.addAll(Arrays.asList(instFrags[j]._ndkMidiEvent));
                    instFrags[j]._ndkMidiEvent = null;
                    instFrags[j].track = track;
                    instFragList.add(instFrags[j]);
                }

                track.instFragments.add(instFragList);
            }
        }

        _ndkTracks = null;
        _ndkInstFragmentArray = null;
    }


    //开始播放
    void Play()
    {
        if (state == PLAY)
            return;

        state = PLAY;

        playStartSec = 0;
        if(GetCurtSec != null)
            playStartSec = GetCurtSec.Execute();

    }

    //暂停播放
    void Pause()
    {
        if (state != PLAY)
            return;

        Goto(curtPlaySec);
        state = PAUSE;
    }

    //停止播放
    void Stop()
    {
        if (state == STOP)
            return;

        isGotoEnd = false;
        isDirectGoto = false;
        isOpen = false;
        gotoSec = 0;
        state = STOP;
    }


    //移除
    void Remove()
    {
        isGotoEnd = false;
        isDirectGoto = false;
        isOpen = false;
        gotoSec = 0;
        state = STOP;
    }


    //设置播放的起始时间点
    void Goto(float gotoSec_)
    {
       int oldState = state;

        isDirectGoto = false;
        isOpen = false;
        isGotoEnd = false;
        gotoSec = gotoSec_;

        playStartSec = 0;
        if(GetCurtSec != null)
            playStartSec = GetCurtSec.Execute();

        curtPlaySec = 0;
        baseSpeedSec = 0;
        state = oldState;
    }

    //设置快进到开头
    void GotoStart()
    {
        Goto(0);
    }

    //设置快进到结尾
    void GotoEnd()
    {
        Goto(9999999);
        isGotoEnd = true;
    }


    //运行
    public void Run(float sec)
    {
        if (state != PLAY)
            return;

        if (isOpen == false)
        {
            isOpen = true;

            Track track;
            for (int i = 0; i < tracks.size(); i++)
            {
                track = tracks.get(i);
                track.Clear();
            }

            if (gotoSec > 0)
            {
                isDirectGoto = true;
                RunCore(gotoSec / speed);
                isDirectGoto = false;
                isGotoEnd = false;
            }
        }

        //
        RunCore(gotoSec / speed + sec - playStartSec);
    }


    protected void RunCore(float sec)
    {
        Track track;
        InstFragment instFrag;
        MidiEvent ev;
        int orgInstFragCount = 0;
        int trackEndCount = 0;
        int instFragCount;

        curtProcessMidiEvent.clear();
        curtPlaySec = baseSpeedSec + (sec - baseSpeedSec) * speed;

        for (int i = 0; i < tracks.size(); i++)
        {
            track = tracks.get(i);

            if (track.isEnded) {
                trackEndCount++;
                continue;
            }

            instFragCount = 0;
            List<LinkedList<InstFragment>> instFragments = track.instFragments;
            LinkedList<InstFragment> instFragmentList;
            for (int j = 0; j < instFragments.size(); j++)
            {
                instFragmentList = instFragments.get(j);
                orgInstFragCount += instFragmentList.size();

                ListIterator<InstFragment> frag_it = instFragmentList.listIterator();
                for (; frag_it.hasNext(); )
                {
                    instFrag = frag_it.next();
                    if (instFrag.isEnded) {
                        instFragCount++;
                        continue;
                    }

                    ListIterator<MidiEvent> it = instFrag.eventOffsetIter;
                    for (; it.hasNext(); )
                    {
                        ev = it.next();

                        //
                        if (!isGotoEnd && ev.startSec > curtPlaySec)
                        {
                            instFrag.eventOffsetIter = it;
                            break;
                        }

                        ProcessEvent(ev, i);
                    }

                    if (!it.hasNext())
                    {
                        instFrag.isEnded = true;
                    }
                }
            }

            if (instFragCount == orgInstFragCount)
            {
                track.isEnded = true;
            }
        }

        //检测播放是否结束
        if (trackEndCount == tracks.size())
        {
            Pause();
        }
    }

    //处理轨道事件
    void ProcessEvent(MidiEvent midEv, int trackIdx)
    {
        switch (midEv.type) {
            case MidiEvent.NoteOn:
                case MidiEvent.NoteOff: {
                if (isDirectGoto)
                    break;
                curtProcessMidiEvent.add(midEv);
            }
            break;
        }
    }

    /**
     * 根据当前时间点，获取当前处理中的midi事件
     * */
    public List<MidiEvent> GetCurtProcessMidiEvents()
    {
        return curtProcessMidiEvent;
    }

    /**获取当前可视midi事件*/
    public VisualMidiEvents GetCurtVisualMidiEvents()
    {
        return visualMidiEvents;
    }

    /**
     * 根据当前时间点，生成指定时长范围的可视midi事件
     * @param secWidth 时长
     * @param trackFilter 需要获取的轨道标号列表
     * */
    public void CreateCurtVisualMidiEvents(float secWidth, int[] trackFilter)
    {
        List<List<NoteOnEvent>> noteOnEvents = visualMidiEvents.GetAllKeyEvents();
        Set<Integer> noteSet = visualMidiEvents.GetNoteSet();
        for (int note : noteSet)
            noteOnEvents.get(note).clear();
        noteSet.clear();

        //
        Track track;
        InstFragment instFrag;
        MidiEvent ev;
        float endSec = curtPlaySec + secWidth;

        for (int i = 0; i < tracks.size(); i++)
        {
            if(trackFilter!=null && !Utils.IsContainIntValue(trackFilter,i))
                continue;

            track = tracks.get(i);

            List<LinkedList<InstFragment>> instFragments = track.instFragments;
            LinkedList<InstFragment> instFragmentList;
            for (int j = 0; j < instFragments.size(); j++)
            {
                instFragmentList = instFragments.get(j);
                ListIterator<InstFragment> frag_it = instFragmentList.listIterator();
                for (; frag_it.hasNext(); )
                {
                    instFrag = frag_it.next();
                    ListIterator<MidiEvent> it = instFrag.eventOffsetIter;
                    for (; it.hasPrevious(); )
                    {
                        ev = it.previous();

                        if(ev == null || ev.endSec < curtPlaySec)
                            continue;

                        instFrag.eventFirstIter = it;
                        NoteOnEvent noteOnEvent = (NoteOnEvent)ev;
                        List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                        noteEvlist.add(noteOnEvent);
                        noteSet.add(noteOnEvent.note);
                    }

                    it = instFrag.eventOffsetIter;
                    for (; it.hasNext(); )
                    {
                        ev = it.next();

                        if(ev == null)
                            continue;

                        if (ev.startSec > endSec)
                            break;

                        NoteOnEvent noteOnEvent = (NoteOnEvent)ev;
                        List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                        noteEvlist.add(noteOnEvent);
                        noteSet.add(noteOnEvent.note);

                    }
                }
            }
        }
    }


    //
    private Track[] _ndkTracks;
    private Object[] _ndkInstFragmentArray;
    private long ndkEditor;
    public long GetNdkEditor() {
        return ndkEditor;
    }

    //
    private static native void ndkLoad(Editor editor, long ndkEditor, String midifile);
}
