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

    protected List<Track> tracks = new ArrayList<>();

    public List<Track> GetTracks() {
        return tracks;
    }

    protected boolean isLoadCompleted = false;

    protected int state = STOP;

    protected float speed = 1;
    protected double curtPlaySec = 0;
    public double GetPlaySec()
    {
        return curtPlaySec;
    }

    //是否是步进播放模式
    protected  boolean isStepPlayMode = false;
    //是否为等待播放模式
    protected boolean isWaitPlayMode = false;

    //结束时间点
    protected double endSec = 0;
    public double GetEndSec()
    {
        return endSec;
    }

    protected List<MidiEvent> curtProcessMidiEvent = new ArrayList<>();
    protected VisualMidiEvents visualMidiEvents = new VisualMidiEvents();

    private void Init()
    {
        ndkInit(this, ndkEditor);
    }

    private void Release()
    {
        ndkInit(this, ndkEditor);
    }

    //判断是否载入完成
    public boolean IsLoadCompleted()
    {
        return isLoadCompleted;
    }

    //载入
    public void Load(String midifile, boolean isWaitLoadCompleted)
    {
        ndkLoad(this, ndkEditor, midifile, isWaitLoadCompleted);
    }

    //载入
    public void Load(String midifile)
    {
        Load(midifile, true);
    }

    private void _JniLoadStart()
    {
        isLoadCompleted = false;
        _Remove();
    }

    private void _JniLoadCompleted()
    {
        ndkCreateDatas(this, ndkEditor);
        _Load();
        isLoadCompleted = true;
    }


    private void _Load()
    {
        Track track;
        for(int i=0; i<_ndkTracks.length; i++)
        {
            track = _ndkTracks[i];
            tracks.add(track);

            Object[] a = (Object[])(_ndkInstFragmentArray[i]);

            for (Object o : a) {
                InstFragment[] instFrags = (InstFragment[]) o;
                LinkedList<InstFragment> instFragList = new LinkedList<>();
                for (InstFragment instFrag : instFrags) {
                    instFrag.midiEvents.addAll(Arrays.asList(instFrag._ndkMidiEvent));
                    instFrag._ndkMidiEvent = null;
                    instFrag.track = track;
                    instFragList.add(instFrag);
                }

                track.instFragments.add(instFragList);
            }
        }

        _ndkTracks = null;
        _ndkInstFragmentArray = null;
    }


    //开始播放
    public void Play()
    {
        ndkPlay(ndkEditor);

        if (state == PLAY)
            return;

        if (state == STOP)
        {
            curtPlaySec = 0;
            for (int i = 0; i < tracks.size(); i++)
                tracks.get(i).Clear();
        }

        state = PLAY;
    }

    //暂停播放
    public void Pause()
    {
        ndkPause(ndkEditor);
        _Pause();
    }

    //暂停播放
    public void _Pause()
    {
        if (state != PLAY)
            return;

        Goto(curtPlaySec);
        state = PAUSE;
    }

    //停止播放
    public void Stop()
    {
        ndkStop(ndkEditor);

        if (state == STOP)
            return;

        for (int i = 0; i < tracks.size(); i++)
            tracks.get(i).Clear();

        curtPlaySec = 0;
        state = STOP;
    }


    //移除
    public void Remove()
    {
        ndkRemove(ndkEditor);
        _Remove();
    }

    //移除
    protected void _Remove()
    {

        for (int i = 0; i < tracks.size(); i++)
            tracks.get(i).Clear();
        tracks.clear();
        curtPlaySec = 0;
        state = STOP;
    }

    //设置播放的起始时间点
    public void Goto(double sec)
    {
        ndkGoto(ndkEditor, sec);

        _Goto(sec);
    }

    //设置播放的起始时间点
    protected void _Goto(double sec)
    {
        for (int i = 0; i < tracks.size(); i++)
            tracks.get(i).Clear();

        curtPlaySec = 0;
        ProcessCore(sec / speed, true);
    }

    //设置快进到开头
    public void GotoStart()
    {
        Goto(0);
    }

    //设置快进到结尾
    public void GotoEnd()
    {
        Goto(endSec + 1);
    }

    //移动到指定时间点
    public void Runto(double sec)
    {
        if (state != PLAY || !isStepPlayMode) {
            Goto(sec);
            return;
        }

        if (sec >= curtPlaySec) {
            Process((sec - curtPlaySec) / speed, true);
        }
        else {
            Goto(sec);
        }
    }

    //移动到指定时间点
    public void GraphRunto(double sec)
    {
        if (state != PLAY || !isStepPlayMode) {
            _Goto(sec);
            return;
        }

        if (sec >= curtPlaySec) {
            Process((sec - curtPlaySec) / speed, true);
        }
        else {
            _Goto(sec);
        }
    }

    //处理
    public void Process()
    {
        Process(false);
    }

    //处理
    public void Process(boolean isStepOp)
    {
        double ndkPlaySec = ndkGetPlaySec(ndkEditor);
        Process(ndkPlaySec - curtPlaySec, isStepOp);
    }

    //处理
    protected void Process(double sec, boolean isStepOp)
    {
        if (isStepPlayMode && !isStepOp)
            return;

        if (state != PLAY)
            return;

        ProcessCore(sec, false);

        if(curtPlaySec >= endSec)
            _Pause();
    }


    protected void ProcessCore(double sec, boolean isDirectGoto)
    {
        curtProcessMidiEvent.clear();
        curtPlaySec += sec;

        Track track;
        for (int i = 0; i < tracks.size(); i++)
        {
            track = tracks.get(i);

            //重新处理当前时间点在事件处理时间中间时，可以重新启用此时间
            List<MidiEvent> evs = track.reProcessMidiEvents;
            if (!evs.isEmpty()) {
                for (int j = 0; j < evs.size(); j++)
                    ProcessEvent(evs.get(j), track, isDirectGoto);
                evs.clear();
            }

            ProcessTrack(track, isDirectGoto);
        }
    }

    void ProcessTrack(Track track, boolean isDirectGoto)
    {
        MidiEvent ev;
        List<LinkedList<InstFragment>> instFragments = track.instFragments;
        LinkedList<InstFragment> instFragmentList;
        for (int j = 0; j < instFragments.size(); j++)
        {
            instFragmentList = instFragments.get(j);
            for (InstFragment instFrag : instFragmentList) {

                ListIterator<MidiEvent> it = instFrag.eventOffsetIter;
                while (it.hasNext())
                {
                    ev = it.next();

                    if(ev == null)
                        continue;

                    //重新处理当前时间点在事件处理时间中间时，可以重新启用此事件
                    if (isDirectGoto &&
                            ev.startSec < curtPlaySec &&
                            ev.endSec > curtPlaySec)
                    {
                        track.reProcessMidiEvents.add(ev);
                    }

                    //
                    if (ev.startSec > curtPlaySec)
                        break;

                    ProcessEvent(ev, track, isDirectGoto);
                }

                instFrag.eventOffsetIter = it;
            }
        }
    }



    //处理轨道事件
    void ProcessEvent(MidiEvent midEv, Track track, boolean isDirectGoto)
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
        double endSec = curtPlaySec + secWidth;

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
                for (InstFragment instFragment : instFragmentList) {
                    instFrag = instFragment;
                    ListIterator<MidiEvent> it = instFrag.eventOffsetIter;
                    while (it.hasPrevious()) {
                        ev = it.previous();

                        if (ev == null || ev.type != MidiEvent.NoteOn || ev.endSec < curtPlaySec)
                            continue;

                        instFrag.eventFirstIter = it;
                        NoteOnEvent noteOnEvent = (NoteOnEvent) ev;
                        List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                        noteEvlist.add(noteOnEvent);
                        noteSet.add(noteOnEvent.note);
                    }

                    it = instFrag.eventOffsetIter;
                    while (it.hasNext()) {
                        ev = it.next();

                        if (ev == null || ev.type != MidiEvent.NoteOn)
                            continue;

                        if (ev.startSec > endSec)
                            break;

                        NoteOnEvent noteOnEvent = (NoteOnEvent) ev;
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
    private static native void ndkInit(Editor editor, long ndkEditor);
    private static native void ndkLoad(Editor editor, long ndkEditor, String midifile, boolean isWaitLoadCompleted);
    private static native void ndkCreateDatas(Editor editor, long ndkEditor);
    private static native void ndkPlay(long ndkEditor);
    private static native void ndkPause(long ndkEditor);
    private static native void ndkStop(long ndkEditor);
    private static native void ndkRemove(long ndkEditor);
    private static native void ndkGoto(long ndkEditor, double sec);
    private static native double ndkGetPlaySec(long ndkEditor);
}
