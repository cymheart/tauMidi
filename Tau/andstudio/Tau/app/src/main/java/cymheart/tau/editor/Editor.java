package cymheart.tau.editor;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.FileUtils;
import cymheart.tau.utils.ScLinkedList;
import cymheart.tau.utils.ScLinkedListNode;
import cymheart.tau.utils.Utils;

/**
 * ----JsonMidiFile结构----
 *
 * [
 *   track0:
 *   {
 *      PlayType: value,
 *      NoteColor:value,
 *      instFragmentBranchs:
 *     [
 *       instFragmentBranch0:
 *       [
 *         instFragmentList0:
 *         [
 *           instFragment0:
 *           [
 *                midiEvent0:
 *                {
 *                 name0:value0,
 *                 name1:value1,
 *                    ...
 *                 namen:valuen
 *                }
 *                    ...
 *                midiEventn:
 *                {
 *                    ...
 *                }
 *          ],
 *
 *               ...,
 *
 *          instFragmentn:
 *          [
 *               ...
 *          ]
 *      ],
 *           ...
 *      instFragmentListn:
 *      [
 *           ...
 *      ]
 *    ],
 *         ...
 *    instFragmentBranchn:
 *    [
 *        ...
 *    ]
 *  ]
 *  },
 *    ...
 *  trackn:
 *  {
 *    ...
 *  }
 * ]
 *
 * */
public class Editor {

    /**无标签*/
    static public final int TagType_Empty = 0;
    /**八度*/
    static public final int TagType_Octave = 1;
    /**手指提示*/
    static public final int TagType_Finger = 2;
    /**字母音符名称*/
    static public final int TagType_NoteName = 3;
    /**固定唱名音符*/
    static public final int TagType_FixedDoNoteName = 4;

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

    /**tracks的json*/
    protected JSONArray jsonTracks = null;

    protected boolean isLoadCompleted = false;

    /**是否载入midi扩展信息*/
    protected boolean isLoadMidiExInfo = false;
    protected String midiFilePath;
    protected String midiExFilePath;
    protected String user;

    /**音符标签类型*/
    public int noteTagType = TagType_Empty;

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


    //设置演奏类型
    public void SetPlayType(int playType)
    {
        ndkSetPlayType(ndkEditor, playType);
    }

    //进入到等待播放模式
    public void EnterWaitPlayMode()
    {
        ndkEnterWaitPlayMode(ndkEditor);
    }

    //离开播放模式
    public void LeavePlayMode()
    {
        ndkLeavePlayMode(ndkEditor);
    }

    //设置轨道事件演奏方式
    public void SetTrackPlayType(int trackIdx, int playType)
    {
        ndkSetTrackPlayType(ndkEditor, trackIdx, playType);
    }

    //按键信号
    public void OnKeySignal(int key)
    {
        ndkOnKeySignal(ndkEditor, key);
    }

    //按键信号
    public void OffKeySignal(int key)
    {
        ndkOffKeySignal(ndkEditor, key);
    }


    //载入
    public void Load(String midifile, boolean isWaitLoadCompleted,  boolean isLoadMidiExInfo, String user)
    {
        jsonTracks = null;
        midiExFilePath = null;
        midiFilePath = midifile;
        this.isLoadMidiExInfo = isLoadMidiExInfo;
        this.user = (user == null ? "" : user);

        //
        ndkLoad(this, ndkEditor, midifile, isWaitLoadCompleted);
    }

    //载入
    public void Load(String midifile, boolean isWaitLoadCompleted,  boolean isLoadMidiExInfo)
    {
        Load(midifile, isWaitLoadCompleted, isLoadMidiExInfo, null);
    }

    //载入
    public void Load(String midifile,  boolean isWaitLoadCompleted)
    {
        Load(midifile, isWaitLoadCompleted,  false, null);
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

        try {
            _Load();
        } catch (JSONException e) {
            e.printStackTrace();
        }

        isLoadCompleted = true;
    }


    private void _Load() throws JSONException {
        if(_ndkTracks == null)
            return;

        //是否存在midi扩展信息
        boolean isExistMidiExInfo = false;

        //
        if(isLoadMidiExInfo)
        {
            int end = midiFilePath.lastIndexOf('.');
            midiExFilePath = midiFilePath.substring(0, end) + "." + user + ".midexinfo";
            if(FileUtils.getInstance().IsFileExist(midiExFilePath)) {
                jsonTracks = new JSONArray(midiExFilePath);
                isExistMidiExInfo = true;
            } else {
                jsonTracks = new JSONArray();
                isExistMidiExInfo = false;
            }
        }

        //
        Track track;
        for(int i=0; i<_ndkTracks.length; i++) {
            track = _ndkTracks[i];

            if (isLoadMidiExInfo){
                if (isExistMidiExInfo) {
                    track.jsonTrack = jsonTracks.getJSONObject(i);
                    track.SetByInnerJson();
                    ndkSetTrackPlayType(ndkEditor, i, track.playType);
                } else {
                    JSONObject jsonTrack = new JSONObject();
                    JSONArray jsonInstFragmentBranchs = new JSONArray();
                    jsonTrack.put("InstFragmentBranchs", jsonInstFragmentBranchs);
                    jsonTracks.put(jsonTrack);
                    track.jsonTrack = jsonTrack;
                    track.SetInnerJson();
                }
            }

            tracks.add(track);
            JSONArray jsonInstFragmentBranchs = null;
            if(track.jsonTrack != null)
                jsonInstFragmentBranchs = track.jsonTrack.getJSONArray("InstFragmentBranchs");

            Object[] a = (Object[])(_ndkInstFragmentArray[i]);

            int j = 0;
            JSONArray jsonInstFragList = null;
            for (Object o : a) {

                if(isLoadMidiExInfo) {
                    if (isExistMidiExInfo) {
                        jsonInstFragList = jsonInstFragmentBranchs.getJSONArray(j++);
                    } else {
                        jsonInstFragList = new JSONArray();
                        jsonInstFragmentBranchs.put(jsonInstFragList);
                    }
                }

                InstFragment[] instFrags = (InstFragment[]) o;
                ScLinkedList<InstFragment> instFragList = new ScLinkedList<>();
                track.instFragmentBranchs.add(instFragList);

                int k = 0;
                JSONArray jsonInstFrag;
                for (InstFragment instFrag : instFrags) {

                    if(isLoadMidiExInfo) {
                        if (isExistMidiExInfo) {
                            jsonInstFrag = jsonInstFragList.getJSONArray(k++);
                            for (int m = 0; m < instFrag._ndkMidiEvent.length; m++) {
                                instFrag._ndkMidiEvent[m].jsonMidiEvent = jsonInstFrag.getJSONObject(m);
                                instFrag.midiEvents.AddLast(instFrag._ndkMidiEvent[m]);
                                instFrag._ndkMidiEvent[m].SetByInnerJson();
                            }

                        } else {
                            jsonInstFrag = new JSONArray();
                            jsonInstFragList.put(jsonInstFrag);

                            JSONObject jsonMidiEvent;
                            for (int m = 0; m < instFrag._ndkMidiEvent.length; m++) {
                                jsonMidiEvent = new JSONObject();
                                jsonInstFrag.put(jsonMidiEvent);
                                instFrag._ndkMidiEvent[m].jsonMidiEvent = jsonMidiEvent;
                                instFrag.midiEvents.AddLast(instFrag._ndkMidiEvent[m]);
                                instFrag._ndkMidiEvent[m].SetInnerJson();
                            }
                        }
                    }
                    else
                    {
                        instFrag.midiEvents.AddLast(instFrag._ndkMidiEvent);
                    }

                    instFrag._ndkMidiEvent = null;
                    instFrag.track = track;
                    instFrag.Clear();
                    instFragList.AddLast(instFrag);
                }
            }
        }

        _ndkTracks = null;
        _ndkInstFragmentArray = null;
    }

    public void SaveMidiExInfo()
    {
        if(jsonTracks == null || midiExFilePath == null || midiExFilePath.isEmpty())
            return;

        String jsonStr = jsonTracks.toString();
        InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
        FileUtils.getInstance().WriteToFile(midiExFilePath, jsonStream);
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
        jsonTracks = null;
        midiExFilePath = null;
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
        if(curtPlaySec == sec)
            return;

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
        if(!isDirectGoto) {
            double ndkPlaySec = ndkGetPlaySec(ndkEditor);
            if (ndkPlaySec - curtPlaySec > 0.002f)
                curtPlaySec = ndkPlaySec;
            else if(ndkPlaySec - curtPlaySec < -0.03f)
                curtPlaySec = ndkPlaySec;
        }

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
        List<ScLinkedList<InstFragment>> instFragmentBranchs = track.instFragmentBranchs;
        ScLinkedList<InstFragment> instFragmentList;
        ScLinkedListNode<MidiEvent> evNode;
        for (int j = 0; j < instFragmentBranchs.size(); j++)
        {
            instFragmentList = instFragmentBranchs.get(j);

            ScLinkedListNode<InstFragment> node = instFragmentList.GetHeadNode();
            for(;node != null; node = node.next)
            {
                InstFragment instFrag = node.elem;
                evNode = instFrag.eventOffsetNode;
                for(; evNode != null; evNode = evNode.next)
                {
                    ev = evNode.elem;
                    if(ev == null)
                        continue;

                    if (ev.startSec > curtPlaySec)
                        break;

                   // ProcessEvent(ev, track, isDirectGoto);
                }

                if(evNode != null)
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
            List<ScLinkedList<InstFragment>> instFragmentBranchs = track.instFragmentBranchs;
            ScLinkedList<InstFragment> instFragmentList;
            for (int j = 0; j < instFragmentBranchs.size(); j++)
            {
                instFragmentList = instFragmentBranchs.get(j);

                ScLinkedListNode<InstFragment> node = instFragmentList.GetHeadNode();
                for(;node != null; node = node.next)
                {
                    instFrag = node.elem;
                    ScLinkedListNode<MidiEvent> evNode = instFrag.eventOffsetNode;
                    if(evNode != null)
                        evNode = evNode.prev;

                    for(; evNode!=null; evNode = evNode.prev)
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

    private static native void ndkSetPlayType(long ndkEditor, int playType);
    private static native void ndkEnterWaitPlayMode(long ndkEditor);
    private static native void ndkLeavePlayMode(long ndkEditor);
    private static native void ndkSetTrackPlayType(long ndkEditor, int trackIdx, int playType);
    private static native void ndkOnKeySignal(long ndkEditor, int key);
    private static native void ndkOffKeySignal(long ndkEditor, int key);

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
