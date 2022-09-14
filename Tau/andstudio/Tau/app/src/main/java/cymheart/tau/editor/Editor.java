package cymheart.tau.editor;

import android.content.Context;
import android.view.Display;
import android.view.WindowManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;

import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.ScLinkedList;
import cymheart.tau.utils.ScLinkedListNode;
import cymheart.tau.utils.Utils;

public class Editor {

    //停止
    static public final int STOP = 0;
    //播放
    static public final int PLAY = 1;
    //暂停
    static public final int PAUSE = 2;
    //结束暂停
    static public final int ENDPAUSE = 3;

    protected List<Track> tracks = new ArrayList<>();

    public List<Track> GetTracks() {
        return tracks;
    }

    protected boolean isLoadCompleted = false;

    protected int state = STOP;
    public int GetState(){return state;}

    protected float speed = 1;
    public float GetSpeed(){return speed;}
    public void SetSpeed(float speed)
    {
        this.speed = speed;
        ndkSetSpeed(ndkEditor, speed);
    }

    protected double curtPlaySec = 0;
    public double GetPlaySec()
    {
        return curtPlaySec;
    }

    //结束时间点
    protected double endSec = 0;
    public double GetEndSec()
    {
        return endSec;
    }

    //是否是步进播放模式
    protected  boolean isStepPlayMode = false;
    //是否为等待播放模式
    protected boolean isWaitPlayMode = false;

    protected List<MidiEvent> curtProcessMidiEvent = new ArrayList<>();
    protected VisualMidiEvents visualMidiEvents = new VisualMidiEvents();

    /**每帧花费秒数*/
    protected double perFrameCostSec = 1/60.0;
    /**设置每帧花费秒数*/
    public void SetPreFrameCostSec(double costSec)
    {
        perFrameCostSec = costSec;
    }


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

    private void  _JniLoadCompleted()
    {
        ndkCreateDatas(this, ndkEditor);
        _Load();
        isLoadCompleted = true;
    }

    private void _Load()
    {
        if(_ndkTracks == null)
            return;

        Track track;
        for(int i=0; i<_ndkTracks.length; i++)
        {
            track = _ndkTracks[i];
            tracks.add(track);

            Object[] a = (Object[])(_ndkInstFragmentArray[i]);

            for (Object o : a) {
                InstFragment[] instFrags = (InstFragment[]) o;
                ScLinkedList<InstFragment> instFragList = new ScLinkedList<>();
                for (InstFragment instFrag : instFrags) {
                    instFrag.midiEvents.AddLast(instFrag._ndkMidiEvent);
                    instFrag._ndkMidiEvent = null;
                    instFrag.track = track;
                    instFrag.Clear();
                    instFragList.AddLast(instFrag);

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

        System.gc();
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

    public double GetBackgroundPlaySec()
    {
        return ndkGetPlaySec(ndkEditor);
    }

    public double GetBackgroundEndSec()
    {
        return ndkGetEndSec(ndkEditor);
    }

    public int GetBackgroundPlayState()
    {
        return ndkGetPlayState(ndkEditor);
    }


    //获取采样流的频谱
    public int GetSampleStreamFreqSpectrums(int channel, double[] outLeft, double[] outRight)
    {
        return ndkGetSampleStreamFreqSpectrums(ndkEditor, channel, outLeft, outRight);
    }

    //每帧处理
    public void ProcessForPerFrame()
    {
        ProcessForPerFrame(false);
    }

    //每帧处理
    public void ProcessForPerFrame(boolean isStepOp)
    {
        Process(perFrameCostSec, isStepOp);
    }

    //处理
    protected void Process(double sec, boolean isStepOp)
    {
        if (isStepPlayMode && !isStepOp)
            return;

        if (state != PLAY)
            return;

        ProcessCore(sec, false);

        if(curtPlaySec >= endSec) {
            _Pause();
            state = ENDPAUSE;
        }
    }

    protected void ProcessCore(double sec, boolean isDirectGoto)
    {
        curtProcessMidiEvent.clear();
        curtPlaySec += sec * speed;

        //
        double ndkPlaySec = ndkGetPlaySec(ndkEditor);
        if(ndkPlaySec - curtPlaySec > 0.002f)
           curtPlaySec = ndkPlaySec;

        //
        Track track;
        for (int i = 0; i < tracks.size(); i++)
        {
            track = tracks.get(i);
            ProcessTrack(track, isDirectGoto);
        }
    }

    protected void ProcessTrack(Track track, boolean isDirectGoto)
    {
        MidiEvent ev;
        List<ScLinkedList<InstFragment>> instFragments = track.instFragments;
        ScLinkedList<InstFragment> instFragmentList;
        ScLinkedListNode<MidiEvent> evNode;
        for (int j = 0; j < instFragments.size(); j++)
        {
            instFragmentList = instFragments.get(j);

            ScLinkedListNode<InstFragment> node = instFragmentList.GetHeadNode();
            for(;node != null; node = node.next)
            {
                InstFragment instFrag = node.elem;
                evNode = instFrag.eventOffsetNode;
                for(;evNode != null; evNode = evNode.next)
                {
                    ev = evNode.elem;
                    if(ev == null)
                        continue;

                    if (ev.startSec > curtPlaySec)
                        break;
                    ProcessEvent(ev, track, isDirectGoto);
                }

                instFrag.eventOffsetNode = evNode;
            }
        }
    }



    //处理轨道事件
    protected void ProcessEvent(MidiEvent midEv, Track track, boolean isDirectGoto)
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

        //
        visualMidiEvents.ClearNoteUsedMark();
        visualMidiEvents.usedNoteCount = 0;
        int[] usedNotes = visualMidiEvents.GetUsedNotes();

        //
        Track track;
        InstFragment instFrag;
        MidiEvent ev;
        double endSec = curtPlaySec + secWidth;

        for (int i = 0; i < tracks.size(); i++)
        {
            if(trackFilter != null && !Utils.IsContainIntValue(trackFilter,i))
                continue;

            track = tracks.get(i);
            List<ScLinkedList<InstFragment>> instFragments = track.instFragments;
            ScLinkedList<InstFragment> instFragmentList;
            for (int j = 0; j < instFragments.size(); j++)
            {
                instFragmentList = instFragments.get(j);

                ScLinkedListNode<InstFragment> node = instFragmentList.GetHeadNode();
                for(;node != null; node = node.next)
                {
                    instFrag = node.elem;
                    ScLinkedListNode<MidiEvent> evNode = instFrag.eventOffsetNode;
                    if(evNode != null)
                        evNode = evNode.prev;

                    for(;evNode!=null; evNode = evNode.prev)
                    {
                        ev = evNode.elem;
                        if (ev == null || ev.type != MidiEvent.NoteOn || ev.endSec < curtPlaySec)
                            continue;


                        NoteOnEvent noteOnEvent = (NoteOnEvent) ev;
                        if(noteOnEvent.note > 127 || noteOnEvent.note < 0)
                            continue;

                        ev.track = i;
                        List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                        noteEvlist.add(noteOnEvent);

                        if(visualMidiEvents.noteUsedMark[noteOnEvent.note] != true) {
                            usedNotes[visualMidiEvents.usedNoteCount++] = noteOnEvent.note;
                            visualMidiEvents.noteUsedMark[noteOnEvent.note] = true;
                        }

                        if(evNode == instFrag.eventFirstNode)
                            break;

                        instFrag.eventFirstNode = evNode;
                    }

                    evNode = instFrag.eventOffsetNode;
                    for(;evNode != null; evNode = evNode.next)
                    {
                        ev = evNode.elem;
                        if (ev == null || ev.type != MidiEvent.NoteOn)
                            continue;

                        if (ev.startSec > endSec)
                            break;

                        NoteOnEvent noteOnEvent = (NoteOnEvent) ev;
                        if(noteOnEvent.note > 127 || noteOnEvent.note < 0)
                            continue;

                        ev.track = i;
                        List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                        noteEvlist.add(noteOnEvent);

                        if(visualMidiEvents.noteUsedMark[noteOnEvent.note] != true) {
                            usedNotes[visualMidiEvents.usedNoteCount++] = noteOnEvent.note;
                            visualMidiEvents.noteUsedMark[noteOnEvent.note] = true;
                        }

                    }
                }
            }
        }


        //生成noteOn轨道分组
        List<int[]> noteOnTrackGroups = visualMidiEvents.noteOnTrackGroups;
        int m;
        List<NoteOnEvent> visualKey;
        int[] noteOnTrackGroup;
        int trackidx;
        int visualKeyCount;

        for(int j=0; j<visualMidiEvents.usedNoteCount; j++){
            m = 0;
            trackidx = -1;
            visualKey = noteOnEvents.get(usedNotes[j]);
            visualKeyCount = visualKey.size();
            noteOnTrackGroup = noteOnTrackGroups.get(usedNotes[j]);
            noteOnTrackGroup[0] = 1;  //保存最后位置到数组0位
            noteOnTrackGroup[1] = visualKeyCount;

            for (int i = 0; i < visualKeyCount; i++) {
                NoteOnEvent noteOnEV = visualKey.get(i);
                if(noteOnEV.track != trackidx) {
                    noteOnTrackGroup[++m] = i;
                    noteOnTrackGroup[m + 1] = visualKeyCount;
                    noteOnTrackGroup[0] = m + 1; //保存最后位置到数组0位
                    trackidx = noteOnEV.track;
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

    private static native boolean ndkIsLoadCompleted(long ndkEditor);
    private static native void ndkLoad(Editor editor, long ndkEditor, String midifile, boolean isWaitLoadCompleted);
    private static native void ndkCreateDatas(Editor editor, long ndkEditor);
    private static native void ndkPlay(long ndkEditor);
    private static native void ndkPause(long ndkEditor);
    private static native void ndkStop(long ndkEditor);
    private static native void ndkRemove(long ndkEditor);
    private static native void ndkGoto(long ndkEditor, double sec);
    private static native double ndkGetPlaySec(long ndkEditor);
    private static native double ndkGetEndSec(long ndkEditor);
    private static native void ndkSetSpeed(long ndkEditor, float speed);
    private static native int ndkGetPlayState(long ndkEditor);
    private static native int ndkGetSampleStreamFreqSpectrums(long ndkEditor, int channel, double[] outLeft, double[] outRight);
}
