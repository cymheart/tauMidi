package cymheart.tau.editor;


import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;

import cymheart.tau.Tau;
import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.FileUtils;
import cymheart.tau.utils.MD5Utils;
import cymheart.tau.utils.Utils;

/**
 * ----JsonMidiExFile结构----
 *{
 *  loop:
 *  {
 *    startSec: value0
 *    endSec: value1
 *    delaySec: value2
 *    errorCount: value3
 *  },
 *
 *   MarkerSetting:
 *   {
 *     myMarker: bool
 *     keySign: bool
 *     midiMarker: bool
 *    },
 *
 *  myMarkers:
 *  [
 *    myMarker0:
 *    {
 *      sec:value0
 *    }
 *  ],
 *
 *  tracks:
 * [
 *   track0:
 *   {
 *      PlayType: value,
 *      NoteColor:value,
 *      noteOnEvents:
 *      [
 *          noteOnEvent0:
 *          {
 *             index:int
 *             name0:value0,
 *             name1:value1,
 *                 ...
 *             namen:valuen
 *           },
 *                 ...,
 *          noteOnEventn:
 *          {
 *             ...
 *          }
 *       ]
 *  },
 *    ...
 *  trackn:
 *  {
 *    ...
 *  }
 * ]
 * }
 *
 * */
public class Editor {

    /**按键类型:未知*/
    static public final int KeyUnKnown = 0;
    /**按键类型:白色*/
    static public final int KeyWhite = 1;
    /**按键类型:黑色*/
    static public final int KeyBlack = 2;

    //
    final static int A0 = 21;
    final static int C1 = 24;
    final static int C2 = 36;
    final static int C3 = 48;
    final static int C4 = 60;
    final static int C5 = 72;
    final static int C6 = 84;
    final static int C7 = 96;
    final static int C8 = 108;

    /**88键钢琴键盘定义*/
    final static public int[] keyTypes88Std = new int[]
            {
                    KeyWhite, KeyBlack, KeyWhite, //A0, bB0, B0
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C1 - B1
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C2 - B2
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C3 - B3
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C4 - B4
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C5 - B5
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C6 - B6
                    KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C7 - B7
                    KeyWhite, //C8
            };

    /**获取note类型*/
    static public int GetNoteType(int note)
    {
        return keyTypes88Std[note - A0];
    }
    /**获取下一个黑色类型note*/
    static public int GetNextBlackNote(int note)
    {
        for(int i = note - A0 + 1; i < keyTypes88Std.length; i++)
            if(keyTypes88Std[i] == KeyBlack)
                return i + A0;
        return -1;
    }

    /**获取前一个白色类型note*/
    static public int GetPrevWhiteNote(int note)
    {
        for(int i = note - A0; i >=0; i--)
            if(keyTypes88Std[i] == KeyWhite)
                return i + A0;
        return -1;
    }

    /**获取下一个白色类型note*/
    static public int GetNextWhiteNote(int note)
    {
        for(int i = note - A0; i < keyTypes88Std.length; i++)
            if(keyTypes88Std[i] == KeyWhite)
                return i + A0;
        return -1;
    }

    /**获取白色类型note数量*/
    static public int GetWhiteNoteCount(int noteStart, int noteEnd)
    {
        int count = 0;
        for(int i = noteStart - A0; i <= noteEnd - A0; i++) {
            if (keyTypes88Std[i] == KeyWhite)
                count++;
        }
        return count;
    }

    private class MarkerComparator implements Comparator<MidiMarker> {
        @Override
        public int compare(MidiMarker left, MidiMarker right) {
            return left.startSec < right.startSec ? -1: 1;
        }
    }

    private class NoteComparator implements Comparator<NoteOnEvent> {

        /**由小到大排列*/
        @Override
        public int compare(NoteOnEvent left, NoteOnEvent right) {
            if(left == null && null == right) return 0;
            else if(left == null) return 1;
            else if(right == null) return -1;
            else if(left.startSec == right.startSec) return 0;
            return left.startSec < right.startSec ? -1: 1;
        }
    }

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

    //通常播放模式
    static public final int PlayMode_Common = 0;
    //等待播放模式
    static public final int PlayMode_Wait = 1;
    //步进播放模式
    //步进播放模式会启用Runto()函数控制播放时间，非步进模式是由midi固定时间内部控制
    static public final int PlayMode_Step = 2;
    //静音模式
    static public final int PlayMode_Mute = 3;


    protected Tau tau;

    protected MarkerComparator markerComparator = new MarkerComparator();
    protected NoteComparator noteCmp = new NoteComparator();

    //初始化开始播放时间点
    protected double initStartPlaySec = 0;

    //获取初始化开始播放时间点
    public double GetInitStartPlaySec()
    {
        return initStartPlaySec;
    }

    //设置初始化开始播放时间点
    public void SetInitStartPlaySec(double sec)
    {
        initStartPlaySec = sec;
        ndkSetInitStartPlaySec(ndkEditor, sec);
    }

    //设置音符发音开始时间点
    public void SetNoteSoundStartSec(double sec)
    {
        ndkSetNoteSoundStartSec(ndkEditor, sec);
    }

    //设置音符发音结束时间点
    public void SetNoteSoundEndSec(double sec)
    {
        ndkSetNoteSoundEndSec(ndkEditor, sec);
    }

    protected Track[] tracks;
    public Track[] GetTracks() {
        return tracks;
    }
    public Track GetTrack(int i){return tracks[i];}

    /**需要弹奏的音符，按时间顺序存放*/
    protected List<NoteOnEvent> needPlayNoteEvs = new ArrayList<>();
    /**乐谱所对应的键盘按键数量*/
    protected int keybaordKeyCountForMusicScore = 15;
    /**获取乐谱所对应的键盘按键数量*/
    public int GetKeybaordKeyCountForMusicScore()
    {
        return keybaordKeyCountForMusicScore;
    }

    /**乐谱所对应的键盘中心按键号*/
    protected int keyboarCenterNoteForMusicScore = 0;
    /**获取乐谱所对应的键盘中心按键号*/
    public int GetKeyboarCenterNoteForMusicScore()
    {
        return keyboarCenterNoteForMusicScore;
    }


    private int minNote, maxNote;
    /**计算乐谱所对应的键盘的中心
     * @param musicScoreAreaWidth 乐谱视区的宽度
     * @param minKeyWidth 最小的按键宽度
     * */
    public void CalKeyboardCenterForMusicScore(float musicScoreAreaWidth, float minKeyWidth)
    {
        float visualWidth = musicScoreAreaWidth - minKeyWidth*2;
        minNote = -1;
        maxNote = -1;
        if(!isSimpleMode) {
            for(int i=0; i<needPlayNoteEvs.size(); i++) {
                if(!GetKeyboardCenter(needPlayNoteEvs.get(i), visualWidth, minKeyWidth))
                    break;
            }
            return;
        }

        //SimpleMode
        for (int i = 0; i < simpleModeTrackNotes.length; i++) {
            if(!GetKeyboardCenter(simpleModeTrackNotes[i], visualWidth, minKeyWidth))
                break;
        }

    }

    protected boolean GetKeyboardCenter(NoteOnEvent ev, float visualWidth, float minKeyWidth)
    {
        if(minNote == -1 || ev.note <minNote)
            minNote = ev.note;
        if(maxNote == -1 || ev.note > maxNote)
            maxNote = ev.note;

        int noteStart = GetPrevWhiteNote(minNote);
        int noteEnd = GetNextWhiteNote(maxNote);
        int count = GetWhiteNoteCount(noteStart, noteEnd);

        float keyWidth = visualWidth/count;
        if(keyWidth < minKeyWidth)
            return false;

        keyboarCenterNoteForMusicScore = GetNextWhiteNote((noteStart + noteEnd)/2);
        keybaordKeyCountForMusicScore = count + 2;
        return true;
    }


    /**是否为简单模式
     * 简单模式下，音符会自动对应到仅几个按键上
     * */
    protected boolean isSimpleMode = false;
    /**设置是否为简单模式
     * 简单模式下，音符会自动对应到仅几个按键上
     * */
    public void SetSimpleMode(boolean is)
    {
        isSimpleMode = is;
    }

    /**简单模式下, 白色按键的数量*/
    protected int simpleModePlayWhiteKeyCount = 7;
    /**设置简单模式下, 白色按键的数量*/
    public void SetSimpleModePlayWhiteKeyCount(int count)
    {
        simpleModePlayWhiteKeyCount = count;
        ndkSetSimpleModePlayWhiteKeyCount(ndkEditor, count);
    }

    /**获取简单模式下, 白色按键的数量*/
    public int GetSimpleModePlayWhiteKeyCount()
    {
        return simpleModePlayWhiteKeyCount;
    }

    /**简单模式轨道音符*/
    protected NoteOnEvent[] simpleModeTrackNotes = null;
    /**获取简单模式音符轨道*/
    public NoteOnEvent[]  GetSimpleModeNoteTrack()
    {
        return simpleModeTrackNotes;
    }

    /**简单模式音符轨道偏移*/
    protected int simpleModeNoteTrackOffset = 0;
    protected int simpleModeNoteTrackFirst = 0;

    /**被合并音符的最大时长*/
    protected float mergeSimpleSrcNoteLimitSec = 0.25f;
    /**设置被合并音符的最大时长*/
    public void SetMergeSimpleSrcNoteLimitSec(float sec)
    {
        mergeSimpleSrcNoteLimitSec = sec;
        ndkSetMergeSimpleSrcNoteLimitSec(ndkEditor, sec);
    }

    /**获取被合并音符的最大时长*/
    public float GetMergeSimpleSrcNoteLimitSec()
    {
        return mergeSimpleSrcNoteLimitSec;
    }

    /**合并到目标音符的最大时长*/
    protected float mergeSimpleDestNoteLimitSec = 0.4f;
    /**设置合并到目标音符的最大时长*/
    public void SetMergeSimpleDestNoteLimitSec(float sec)
    {
        mergeSimpleDestNoteLimitSec = sec;
        ndkSetMergeSimpleDestNoteLimitSec(ndkEditor, sec);
    }

    /**获取合并到目标音符的最大时长*/
    public float GetMergeSimpleDestNoteLimitSec()
    {
        return mergeSimpleDestNoteLimitSec;
    }

    //
    protected List<MidiMarker> myMarkers = new ArrayList<>();
    public List<MidiMarker> GetMyMarkers(){return myMarkers;}

    protected List<MidiMarker> midiMarkers = new ArrayList<>();
    public List<MidiMarker> GetMidiMarkers() {
        return midiMarkers;
    }

    protected MeasureInfo measureInfo = null;
    public MeasureInfo GetMeasureInfo() {
        return measureInfo;
    }

    /**获取最后一小节开头时间点*/
    public double GetLastMeasureStartSec()
    {
        if(measureInfo == null)
            return 0;

        return measureInfo.GetMeasureEndSec(1);
    }

    /**获取最后一小节结尾时间点*/
    public double GetLastMeasureEndSec()
    {
        if(measureInfo == null)
            return 0;

        return measureInfo.GetMeasureEndSec(measureInfo.GetMeasureCount());
    }

    //
    protected double loopStartSec = -1;
    public double GetLoopStartSec(){return loopStartSec;}

    protected double loopEndSec = -1;
    public double GetLoopEndSec(){return loopEndSec;}

    protected double loopDelaySec = 0;
    public double GetLoopDelaySec(){return loopDelaySec;}

    protected int loopErrorCount  = 0;
    public int GetLoopErrorCount(){return loopErrorCount;}

    //
    protected boolean isEnableMyMarker = true;
    public boolean IsEnableMyMarker(){return isEnableMyMarker;}

    protected boolean isEnableKeySign = true;
    public boolean IsEnableKeySign(){return isEnableKeySign;}

    protected boolean isEnableMidiMarker = true;
    public boolean IsEnableMidiMarker(){return isEnableMidiMarker;}


    //
    protected JSONObject jsonMidiFileEx = null;
    protected JSONObject jsonLoop = null;
    protected JSONObject jsonMarkerSetting = null;
    protected JSONArray jsonMyMarkers = null;

    /**tracks的json*/
    protected JSONArray jsonTracks = null;

    protected boolean isLoadCompleted = false;

    /**是否载入弹奏历史记录*/
    protected boolean isLoadPlayHistory = false;
    /**弹奏历史记录文件名称*/
    protected String playHistoryFileName = null;

    /**原始midi文件路径*/
    protected String midiFilePath;
    /**用户数据根路径*/
    protected String userDatasRootPath;
    /**是否载入midi扩展信息*/
    protected boolean isLoadMidiExInfo = false;
    /**midi扩展文件路径*/
    protected String midiExFilePath;
    /**midi扩展文件MD5*/
    protected String midiExFileMD5;
    /**用户名称*/
    protected String user;
    /**用户数据midi文件目录路径*/
    protected String userDataMidiDirPath;
    /**midi名称*/
    protected String midiName;

    /**是否载入弹奏录制*/
    protected boolean isLoadPlayRecord = false;
    /**判断是否载入了弹奏录制*/
    public boolean IsLoadPlayRecord()
    {
        return isLoadPlayRecord;
    }


    /**是否弹奏录制*/
    protected boolean isPlayRecord = false;
    /**设置是否弹奏录制*/
    public void SetPlayRecord(boolean isPlay)
    {
        isPlayRecord = isPlay;
    }

    /**判断是否弹奏录制*/
    public boolean IsPlayRecord()
    {
        return isPlayRecord;
    }

    /**弹奏历史*/
    protected PlayHistory playHistory = new PlayHistory();


    /**弹奏录制器*/
    protected PlayRecorder playRecorder = new PlayRecorder();
    /**获取弹奏录制器*/
    public PlayRecorder GetPlayRecorder()
    {
        return playRecorder;
    }

    /**录制继续事件*/
    public void RecordContinue(float sec)
    {
        playRecorder.Continue(sec);
    }

    /**录制等待事件*/
    public void RecordWait(float sec)
    {
        playRecorder.Wait(sec);
    }

    /**录制按压按键动作*/
    public void RecordOnKey(int note, float sec)
    {
        playRecorder.OnKey(note, 127, 0, sec);
    }

    /**录制松开按键动作*/
    public void RecordOffKey(int note, float sec)
    {
        playRecorder.OffKey(note,0, sec);
    }


    /**音符标签类型*/
    public int noteTagType = TagType_Empty;

    protected int state = STOP;
    public int GetState(){return state;}

    /**是否需要同步时间点*/
    protected boolean isNeedSyncSec = false;

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

    //根据指定秒数获取tick数
    public int GetSecTickCount(double sec)
    {
       return ndkGetSecTickCount(ndkEditor, sec);
    }

    //根据指定tick数秒数获取时间点
    public double GetTickSec(int tick)
    {
        return ndkGetTickSec(ndkEditor, tick);
    }

    //播放模式
    protected int playMode = PlayMode_Common;
    //获取播放模式
    public int GetPlayMode()
    {
        return playMode;
    }

    /**视图是否等待中*/
    protected boolean isWaitForGraph = false;
    /**视图是否等待中*/
    public boolean IsWaitForGraph()
    {
        return isWaitForGraph;
    }

    /**视图等待*/
    public void WaitForGraph()
    {
        isWaitForGraph = true;
    }

    /**视图继续*/
    public void ContinueForGraph()
    {
        isWaitForGraph = false;
    }


    //等待(区别于暂停，等待相当于在原始位置播放)
    public void Wait(){
        ndkWait(ndkEditor);
    }
    //继续，相对于等待命令
    public void Continue()
    {
        ndkContinue(ndkEditor);
    }

    //是否等待中
    public boolean IsWait()
    {
        return ndkIsWait(ndkEditor);
    }

    //是否是等待中的按键
    public boolean IsWaitKey(int key)
    {
        return ndkIsWaitKey(ndkEditor, key);
    }

    //获取等待中按键的数量
    public int GetWaitKeyCount(int key)
    {
        return ndkGetWaitKeyCount(ndkEditor, key);
    }

    //是否是等待中的按键事件
    public boolean IsWaitNoteOnEvent(NoteOnEvent noteOnEv)
    {
        return IsWaitNoteOnEvent(ndkEditor, noteOnEv.getNdkMidiEvent());
    }

    //是否有等待中的按键
    public boolean HavWaitKey()
    {
        return ndkHavWaitKey(ndkEditor);
    }

    //所有等待按键信号
    public void OnWaitKeysSignal()
    {
        ndkOnWaitKeysSignal(ndkEditor);
    }

    //
    protected VisualMidiEvents visualMidiEvents = new VisualMidiEvents();

    /**需要演奏的音符数量*/
    protected int needPlayNoteCount = 0;
    /**获取需要演奏的音符数量*/
    public int GetNeedPlayNoteCount()
    {
        return needPlayNoteCount;
    }

    /**总游戏点数*/
    protected float totalGamePoint = 0;
    /**获取总游戏点数*/
    public float GetTotalGamePoint()
    {
        return totalGamePoint;
    }

    /**弹奏错误音符个数*/
    protected int playErrorNoteCount = 0;
    public void SetPlayErrorNoteCount(int count)
    {
        playErrorNoteCount = count;
    }
    /**获取弹奏错误音符个数*/
    public int GetPlayErrorNoteCount()
    {
       return playErrorNoteCount;
    }

    /**回拉时间次数*/
    protected int pullTimeCount = 0;
    /**设置回拉时间次数*/
    public void SetPullTimeCount(int count)
    {
        pullTimeCount = count;
    }
    /**获取回拉时间次数*/
    public int GetPullTimeCount()
    {
        return pullTimeCount;
    }

    /**循环播放次数*/
    protected int loopCount = 0;
    /**设置循环播放次数*/
    public void SetLoopCount(int count)
    {
        loopCount = count;
    }
    /**获取循环播放次数*/
    public int GetLoopCount()
    {
        return loopCount;
    }

    /**分数*/
    protected int score = 0;
    /**设置分数*/
    public void SetScore(int score)
    {
        this.score = score;
    }
    /**获取分数*/
    public int GetScore()
    {
        return score;
    }

    /**丢失音符事件*/
    protected NoteOnEvent[] missNoteEv = new NoteOnEvent[10000];
    /**添加已丢失的音符事件*/
    public void AddMissNoteEv(NoteOnEvent ev)
    {
        ev.isMiss = true;
        missNoteEv[missNoteCount++] = ev;
    }

    /**丢失音符数量*/
    protected int missNoteCount = 0;
    /**获取丢失音符数量*/
    public int GetMissNoteCount() {
        return missNoteCount;
    }

    /**已演奏的音符事件*/
    protected NoteOnEvent[] playedNoteEv = new NoteOnEvent[10000];
    /**添加已演奏的音符事件*/
    public void AddPlayedNoteEv(NoteOnEvent ev)
    {
        ev.isPlay = true;
        playedNoteEv[playedNoteCount++] = ev;
    }

    /**已演奏的音符事件数量*/
    protected int playedNoteCount = 0;
    /**获取已演奏的音符事件数量*/
    public int GetPlayedNoteCount()
    {
        return playedNoteCount;
    }

    /**弹奏点数*/
    public float playGamePoint = 0;
    /**获取弹奏点数*/
    public float GetPlayGamePoint()
    {
        return playGamePoint;
    }

    /**等待结束的MidiEvs*/
    protected NoteOnEvent[] waitEndEvs = new NoteOnEvent[10000];
    /**等待结束的MidiEvs数量*/
    protected int waitEndEvCount = 0;
    /**获取等待结束的MidiEvs数量*/
    public int GetWaitEndEvCount()
    {
        return waitEndEvCount;
    }

    /**当前时间需要弹奏的noteEvs*/
    protected NoteOnEvent[] curtNeedPlayNoteEvs = new NoteOnEvent[1000];
    /**当前时间需要弹奏的noteEv*/
    public boolean IsCurtNeedPlayNoteEv(NoteOnEvent noteEv)
    {
        for(int i=0; i<curtNeedPlayNoteCount; i++) {
            if(curtNeedPlayNoteEvs[i] == noteEv)
                return true;
        }
        return false;
    }

    /**当前时间需要弹奏的noteEv数量*/
    protected int curtNeedPlayNoteCount = 0;
    /**当前时间需要弹奏的时间点*/
    protected float curtNeedPlayStartSec = 0;

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

    //演奏类型
    protected int playType = MidiEvent.PlayType_DoubleHand;
    //设置演奏类型
    public void SetPlayType(int playType)
    {
        this.playType = playType;
        ndkSetPlayType(ndkEditor, playType);
    }

    /**进入到等待播放模式*/
    public void EnterWaitPlayMode()
    {
        if (playMode == PlayMode_Wait)
            return;

        playMode = PlayMode_Wait;
        ndkEnterWaitPlayMode(ndkEditor);
    }

    /**进入到静音模式*/
    public void EnterMuteMode() {

        if (playMode == PlayMode_Mute)
            return;

        playMode = PlayMode_Mute;
        ndkEnterMuteMode(ndkEditor);
    }


    /**离开播放模式*/
    public void LeavePlayMode()
    {
        playMode = PlayMode_Common;
        ndkLeavePlayMode(ndkEditor);
    }

    //设置轨道事件演奏方式
    public void SetTrackPlayType(int trackIdx, int playType)
    {
        if(tracks.length <= trackIdx)
            return;
        tracks[trackIdx].SetPlayType(playType);
        ndkSetTrackPlayType(ndkEditor, trackIdx, playType);
    }

    //设置排除需要等待的按键
    public void SetExcludeNeedWaitKey(int key)
    {
        ndkSetExcludeNeedWaitKey(ndkEditor, key);
    }

    //设置包含需要等待的按键
    public void SetIncludeNeedWaitKey(int key)
    {
        ndkSetIncludeNeedWaitKey(ndkEditor, key);
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


    // 禁止播放指定编号Midi文件的轨道
    public void DisableTrack(int trackIdx)
    {
        ndkDisableTrack(ndkEditor, trackIdx);
    }

    // 禁止播放Midi的所有轨道
    public void DisableAllTrack() {
        ndkDisableAllTrack(ndkEditor);
    }

    // 启用播放指定编号Midi文件的轨道
    public void EnableTrack(int trackIdx) {
        ndkEnableTrack(ndkEditor, trackIdx);
    }

    // 启用播放Midi的所有轨道
    public void EnableAllTrack()
    {
        ndkEnableAllTrack(ndkEditor);
    }

    public void OnKey(int key, float velocity, int trackIdx)
    {
        ndkOnKeyAtTrack(ndkEditor, key, velocity, trackIdx, 0);
    }

    // 释放按键
    public void OffKey(int key, float velocity, int trackIdx)
    {
        ndkOffKeyAtTrack(ndkEditor, key, velocity, trackIdx, 0);
    }

    public void OnKey(int key, float velocity, int trackIdx, int id)
    {
        ndkOnKeyAtTrack(ndkEditor, key, velocity, trackIdx, id);
    }

    // 释放按键
    public void OffKey(int key, float velocity, int trackIdx, int id)
    {
        ndkOffKeyAtTrack(ndkEditor, key, velocity, trackIdx, id);
    }

    // 释放所有按键
    public void OffAllKeys()
    {
        ndkOffAllKeys(ndkEditor);
    }


    public Editor(Tau tau)
    {
        this.tau = tau;
        playHistory.editor = this;
    }

    //载入历史记录
    public void LoadByHistory(String midifile, boolean isWaitLoadCompleted,  String playHistoryFileName, String user)
    {
        isLoadCompleted = false;
        jsonTracks = null;
        midiExFilePath = null;
        midiFilePath = midifile;
        isLoadMidiExInfo = true;
        isLoadPlayHistory = true;
        this.playHistoryFileName = playHistoryFileName;
        this.user = user;

        //
        ndkLoad(this, ndkEditor, midifile, isWaitLoadCompleted);
    }



    //载入
    public void Load(String midifile, boolean isWaitLoadCompleted,  boolean isLoadMidiExInfo, String user)
    {
        isLoadCompleted = false;
        jsonTracks = null;
        midiExFilePath = null;
        midiFilePath = midifile;
        this.isLoadMidiExInfo = isLoadMidiExInfo;
        this.user = user;

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
        _Remove();
    }

    private void  _JniLoadCompleted()
    {
        ndkCreateDatas(this, ndkEditor);

        try {
            CreateMidiMarkers();
            ParseMidi();
        } catch (JSONException e) {
            e.printStackTrace();
        }

        isLoadCompleted = true;
    }


    private void CreateMidiMarkers()
    {
        midiMarkers.clear();
        if(_ndkMidiMarkers == null)
            return;

        MidiMarker midiMarker;
        for(int i=0; i<_ndkMidiMarkers.length; i++) {
            midiMarker = _ndkMidiMarkers[i];
            midiMarkers.add(midiMarker);
        }

        //
        _ndkMidiMarkers = null;
    }


    private void ParseMidi() throws JSONException
    {
        if(tracks == null)
            return;

        //是否存在midi扩展信息
        boolean isExistMidiExInfo = false;
        needPlayNoteCount = 0;
        missNoteCount = 0;
        playedNoteCount = 0;

        //
        userDatasRootPath = FileUtils.getInstance().GetExtralFilePath(tau.GetContext()) + "/UserDatas/";
        int end = midiFilePath.lastIndexOf('.');
        int start = midiFilePath.lastIndexOf('/', end);
        userDataMidiDirPath = midiFilePath.substring(0, end) + "/";
        midiName = midiFilePath.substring(start, end);

        String userName;
        if(user == null || user.isEmpty()) userName = "visitor";
        else userName = user;


        //载入弹奏记录
        if(isLoadPlayHistory)
        {
            String historyPath = userDatasRootPath + userName +
                    userDataMidiDirPath + "history/" + playHistoryFileName + ".History";

            playHistory.LoadFromFile(historyPath);
            playRecorder.LoadFromFile(playHistory.playRecordFilePath);

            SetPlayRecord(true);
        }

        //
        if(isLoadMidiExInfo)
        {
            if(!isLoadPlayHistory) {
                midiExFilePath = userDatasRootPath + userName + "/" +
                        userDataMidiDirPath + midiName + ".midexinfo";
            }else{
                midiExFilePath = playHistory.midiExFilePath;
            }

            if(FileUtils.getInstance().IsFileExist(midiExFilePath)) {
                String jsonStr = FileUtils.getInstance().ReadFileToString(midiExFilePath);
                jsonMidiFileEx = new JSONObject(jsonStr);
                jsonTracks = jsonMidiFileEx.getJSONArray("tracks");
                jsonLoop = jsonMidiFileEx.getJSONObject("loop");
                jsonMarkerSetting = jsonMidiFileEx.getJSONObject("markerSetting");
                jsonMyMarkers = jsonMidiFileEx.getJSONArray("myMarkers");

                isExistMidiExInfo = true;
            } else {
                jsonMidiFileEx = new JSONObject();

                //
                jsonLoop = new JSONObject();
                jsonLoop.put("startSec", loopStartSec);
                jsonLoop.put("endSec", loopEndSec);
                jsonLoop.put("delaySec", loopDelaySec);
                jsonLoop.put("errorCount", loopErrorCount);
                jsonMidiFileEx.put("loop", jsonLoop);

                //
                jsonMarkerSetting = new JSONObject();
                jsonMarkerSetting.put("myMarker", isEnableMyMarker);
                jsonMarkerSetting.put("keySign", isEnableKeySign);
                jsonMarkerSetting.put("midiMarker", isEnableMidiMarker);
                jsonMidiFileEx.put("markerSetting", jsonLoop);

                //
                jsonMyMarkers = new JSONArray();
                jsonMidiFileEx.put("myMarkers", jsonMyMarkers);


                jsonTracks = new JSONArray();
                jsonMidiFileEx.put("tracks", jsonTracks);

                isExistMidiExInfo = false;
            }
        }


        //
        myMarkers.clear();
        if(isLoadMidiExInfo)
        {
            //loop
            if(jsonLoop != null && jsonLoop.length() != 0) {

                if(jsonLoop.has("startSec"))
                    loopStartSec = jsonLoop.getDouble("startSec");
                if(jsonLoop.has("endSec"))
                    loopEndSec = jsonLoop.getDouble("endSec");
                if(jsonLoop.has("delaySec"))
                    loopDelaySec = jsonLoop.getDouble("delaySec");
                if(jsonLoop.has("errorCount"))
                    loopErrorCount = jsonLoop.getInt("errorCount");
            }

            //markerSetting
            if(jsonMarkerSetting != null && jsonMarkerSetting.length() != 0) {
                if(jsonMarkerSetting.has("myMarker"))
                    isEnableMyMarker = jsonMarkerSetting.getBoolean("myMarker");
                if(jsonMarkerSetting.has("keySign"))
                    isEnableKeySign = jsonMarkerSetting.getBoolean("keySign");
                if(jsonMarkerSetting.has("midiMarker"))
                    isEnableMidiMarker = jsonMarkerSetting.getBoolean("midiMarker");
            }

            //mymarkers
            MidiMarker myMarker;
            double sec = 0;
            for(int i=0;i<jsonMyMarkers.length();i++)
            {
                JSONObject jsonMyMarker = jsonMyMarkers.getJSONObject(i);
                sec = jsonMyMarker.getDouble("sec");

                myMarker = new MidiMarker();
                myMarker.isEnableMarkerText = true;
                myMarker.startSec = sec;
                myMarkers.add(myMarker);
            }

            Collections.sort(myMarkers, markerComparator);
        }

        //
        Track track;
        JSONObject jsonMidiEvent;
        JSONArray jsonNoteOnEvents;

        if (isLoadMidiExInfo)
        {
            for (int i = 0; i < tracks.length; i++) {
                track = tracks[i];

                if (isExistMidiExInfo) {
                    track.jsonTrack = jsonTracks.getJSONObject(i);
                    track.SetByInnerJson();
                    ndkSetTrackPlayType(ndkEditor, i, track.playType);
                    jsonNoteOnEvents = track.jsonTrack.getJSONArray("noteOnEvents");

                    int midiIndex;
                    for (int m = 0; m < jsonNoteOnEvents.length(); m++) {
                        jsonMidiEvent = jsonNoteOnEvents.getJSONObject(m);
                        midiIndex = jsonMidiEvent.getInt("index");
                        track.noteOnEvents[midiIndex].jsonMidiEvent = jsonMidiEvent;
                        track.noteOnEvents[midiIndex].SetByInnerJson();
                    }

                } else {
                    JSONObject jsonTrack = new JSONObject();
                    jsonTracks.put(jsonTrack);
                    track.jsonTrack = jsonTrack;
                    track.SetInnerJson();

                    jsonNoteOnEvents = new JSONArray();
                    track.jsonTrack.put("noteOnEvents", jsonNoteOnEvents);
                }
            }
        }

        //分配轨道颜色
        AssignTrackColors();

        //生成简单模式下的音符组，并移除被合并的轨道音符
        if(isSimpleMode) {
            simpleModeTrackNotes = null;
            simpleModeNoteTrackOffset = 0;
            simpleModeNoteTrackFirst = 0;
            RemoveSimpleModeNotesFromTracks();
            //此调用将会填充simpleModeTrackNotes数据
            ndkCreateSimpleModeTrack(this, ndkEditor);

            //
            if(simpleModeTrackNotes != null && simpleModeTrackNotes.length > 0) {
                track = tracks[simpleModeTrackNotes[0].trackIdx];
                for (int i = 0; i < simpleModeTrackNotes.length; i++)
                    simpleModeTrackNotes[i].track = track;
            }
        }

        //计算总得分点数，和需要弹奏的音符数量
        if(isLoadMidiExInfo)
        {
            totalGamePoint = 0;
            float mul = 1;
            NoteOnEvent ev;

            if(!isSimpleMode)
            {
                needPlayNoteEvs.clear();
                for (int i = 0; i < tracks.length; i++)
                {
                    track = tracks[i];
                    for (int j = 0; j < track.noteOnEvents.length; j++) {
                        ev = track.noteOnEvents[j];
                        if (!IsPointerPlayNote(ev))
                            continue;
                        needPlayNoteEvs.add(ev);
                    }
                }

                Collections.sort(needPlayNoteEvs, noteCmp);
                for(int i=0; i<needPlayNoteEvs.size(); i++)
                {
                    ev = needPlayNoteEvs.get(i);
                    //计算总得分点数，和需要弹奏的音符数量
                    needPlayNoteCount++;
                    totalGamePoint += 30 * mul + (ev.endSec - ev.startSec) * mul * 10;
                    mul += 0.1;
                }

            }else {
                for (int i = 0; i < simpleModeTrackNotes.length; i++) {
                    ev = simpleModeTrackNotes[i];
                    //计算总得分点数，和需要弹奏的音符数量
                    needPlayNoteCount++;
                    totalGamePoint += 30 * mul + (ev.endSec - ev.startSec) * mul * 10;
                    mul += 0.1;
                }
            }
        }


        //
        if(isLoadMidiExInfo && needPlayNoteCount > 10000) {
            playedNoteEv = new NoteOnEvent[needPlayNoteCount];
            missNoteEv = new NoteOnEvent[needPlayNoteCount];
            waitEndEvs = new NoteOnEvent[needPlayNoteCount];
        }
    }


    /**自动分配轨道颜色*/
    protected void AssignTrackColors()
    {
        int minUseCount;
        int[] colorUseCount = new int[]{0,0,0,0,0,0,0,0};  //每个被轨道使用的颜色的次数

        //
        int canUseColorCount = 0; //可以被轨道使用的颜色总数
        int[] canUseColor = new int[8];   //可以被轨道使用的颜色

        //
        Track track;
        for(int i = 0; i<tracks.length; i++)
        {
            track = tracks[i];
            int noteColor = track.GetNoteColor();
            if(noteColor != MidiEvent.NoteColor_None)
                colorUseCount[noteColor]++;
        }

        //
        boolean isContinue = true;
        while(isContinue)
        {
            minUseCount = 9999;
            for (int i = 1; i <= 6; i++) {
                if (colorUseCount[i] < minUseCount) {
                    canUseColorCount = 0;
                    minUseCount = colorUseCount[i];
                }

                if (minUseCount == colorUseCount[i])
                    canUseColor[canUseColorCount++] = i;
            }


            isContinue = false;
            int j = 0;
            for (int i = 0; i < tracks.length; i++)
            {
                track = tracks[i];
                int noteColor = track.GetNoteColor();
                if (noteColor != MidiEvent.NoteColor_None ||
                        track.GetChannel().GetChannelNum() == 9)
                    continue;

                track.SetNoteColor(canUseColor[j]);
                colorUseCount[canUseColor[j]]++;

                if (++j >= canUseColorCount) {
                    if(i+1 < tracks.length)
                        isContinue = true;
                    break;
                }
            }
        }
    }

    /**移除简单模式下相关的轨道事件*/
    protected void RemoveSimpleModeNotesFromTracks()
    {
        int removeCount;
        Track track;
        for (int i = 0; i < tracks.length; i++)
        {
            removeCount = 0;
            track = tracks[i];
            NoteOnEvent ev;
            NoteOnEvent[] noteOnEvents = track.noteOnEvents;
            for (int j = 0; j < noteOnEvents.length; j++) {
                ev = noteOnEvents[j];
                if (IsPointerPlayNote(ev)) {
                    noteOnEvents[j] = null;
                    removeCount++;
                }
            }

            int idx = 0;
            NoteOnEvent[] newNoteOnEvents = new NoteOnEvent[noteOnEvents.length - removeCount];
            for (int j = 0; j < noteOnEvents.length; j++) {
                if (noteOnEvents[j] != null)
                    newNoteOnEvents[idx++] = noteOnEvents[j];
            }

            track.noteOnEvents = newNoteOnEvents;
        }
    }

    /**在指定时间点，添加我的书签*/
    public void AddMyMarker(double sec)
    {
        MidiMarker myMarker;
        for(int i=0; i< myMarkers.size(); i++) {
            myMarker = myMarkers.get(i);
            if(Math.abs(myMarker.startSec - sec) < 0.0001f)
            {
                myMarkers.remove(i);

                try {
                    for (int j = 0; j < jsonMyMarkers.length(); j++) {
                        JSONObject jsonMyMarker = jsonMyMarkers.getJSONObject(j);
                        sec = jsonMyMarker.getDouble("sec");
                        if (Math.abs(myMarker.startSec - sec) < 0.0001f) {
                            jsonMyMarkers.remove(j);
                            break;
                        }
                    }
                }catch (JSONException e) {
                    e.printStackTrace();
                }
                return;
            }
        }

        JSONObject jsonMyMarker = new JSONObject();

        try {
            jsonMyMarker.put("sec", sec);
        }catch (JSONException e) {
            e.printStackTrace();
        }

        jsonMyMarkers.put(jsonMyMarker);

        myMarker = new MidiMarker();
        myMarker.isEnableMarkerText = true;
        myMarker.startSec = sec;

        myMarkers.add(myMarker);
        Collections.sort(myMarkers, markerComparator);
    }

    public void RemoveAllMyMarker()
    {
        myMarkers.clear();
        for (int j = jsonMyMarkers.length(); j >= 0; j--)
            jsonMyMarkers.remove(j);
    }

    public void SetLoopRange(double startSec, double endSec)
    {
        loopStartSec = startSec;
        loopEndSec = endSec;

        try {
            jsonLoop.put("startSec", startSec);
            jsonLoop.put("endSec", endSec);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetLoopStartSec(double sec)
    {
        loopStartSec = sec;
        try {
            jsonLoop.put("startSec", sec);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetLoopEndSec(double sec)
    {
        loopEndSec = sec;

        try {
            jsonLoop.put("endSec", sec);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetLoopDelaySec(double sec)
    {
        loopDelaySec = sec;

        try {
            jsonLoop.put("delaySec", sec);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetLoopErrorCount(int count)
    {
        loopErrorCount = count;

        try {
            jsonLoop.put("errorCount", count);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetEnableMyMarker(boolean isEnable){
        isEnableMyMarker = isEnable;
        try {
            jsonMarkerSetting.put("myMarker", isEnableMyMarker);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetEnableKeySign(boolean isEnable){
        isEnableKeySign = isEnable;
        try {
            jsonMarkerSetting.put("keySign", isEnableKeySign);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void SetEnableMidiMarker(boolean isEnable){
        isEnableMidiMarker = isEnable;
        try {
            jsonMarkerSetting.put("midiMarker", isEnableMidiMarker);
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }


    /**保存midi扩展信息*/
    public void SaveMidiExInfo()
    {
        if(isPlayRecord || jsonTracks == null ||
                midiExFilePath == null || midiExFilePath.isEmpty())
            return;

        String jsonStr = jsonMidiFileEx.toString();
        InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
        FileUtils.getInstance().WriteToFile(midiExFilePath, jsonStream);
    }

    /**保存弹奏历史记录*/
    public void SavePlayHistory()
    {
        if(isPlayRecord)
            return;

        String jsonMidiFileExStr = jsonMidiFileEx.toString();
        midiExFileMD5 = MD5Utils.getMD5String(jsonMidiFileExStr);

        Date date = new Date();
        SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        String dateStr = formatter.format(date);
        //
        String userName;
        if(user == null || user.isEmpty()) userName = "visitor";
        else userName = user;

        String midiExFilePath = userDatasRootPath + userName + "/" +
                userDataMidiDirPath + "history/" + midiName + "." + midiExFileMD5 + ".midexinfo";

        InputStream jsonStream = new ByteArrayInputStream(jsonMidiFileExStr.getBytes(StandardCharsets.UTF_8));
        FileUtils.getInstance().WriteToFile(midiExFilePath, jsonStream);


        String recordFilePath = userDatasRootPath + userName + "/" +
                userDataMidiDirPath + "history/" + dateStr + ".NoteRecord";

        String historyFilePath = userDatasRootPath + userName + "/" +
                userDataMidiDirPath + "history/" + dateStr + ".History";

        //保存弹奏录制
        if(!playRecorder.IsEmpty())
            playRecorder.SaveToFile(recordFilePath);

        playHistory.SaveToFile(historyFilePath, recordFilePath, midiExFilePath);
    }


    /**载入弹奏录制*/
    public void LoadPlayRecord()
    {
        //playRecorder.LoadFromFile(recordFilePath);
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

    //获取当前bpm
    public float GetCurtBPM()
    {
        return ndkGetCurtBPM(ndkEditor);
    }

    //开始播放
    public void Play()
    {
        ndkPlay(ndkEditor);

        if (state == PLAY)
            return;

        if(tracks == null)
            return;

        if (state == STOP)
        {
            for (int i = 0; i < tracks.length; i++)
                tracks[i].Clear();
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

        for (int i = 0; i < tracks.length; i++)
            tracks[i].Clear();

        curtPlaySec = initStartPlaySec;
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
        if(tracks != null) {
            for (int i = 0; i < tracks.length; i++)
                tracks[i].Clear();
            tracks = null;
        }

        needPlayNoteEvs.clear();

        simpleModeNoteTrackOffset = 0;
        simpleModeNoteTrackFirst = 0;
        simpleModeTrackNotes = null;

        if(measureInfo != null)
            measureInfo.Clear();
        measureInfo = null;

        isLoadCompleted = false;
        isLoadPlayHistory = false;
        isPlayRecord = false;
        curtPlaySec = initStartPlaySec;
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

        if(measureInfo != null)
            measureInfo.atMeasure = measureInfo.GetSecAtMeasure((float)sec);

        if(sec < curtPlaySec) {
            for (int i = 0; i < tracks.length; i++)
                tracks[i].Clear();

            simpleModeNoteTrackOffset = 0;
            simpleModeNoteTrackFirst = 0;

            playRecorder.ResetOffset();

            if(!isPlayRecord)
                playRecorder.RemoveAfterSec((float)sec);

            curtPlaySec = 0;
            waitEndEvCount = 0;
            waitEndEvs[0] = null;
        }

        ProcessCore(sec - curtPlaySec, true);
    }

    //设置快进到开头
    public void GotoStart()
    {
        Goto(initStartPlaySec);
    }

    //设置快进到结尾
    public void GotoEnd()
    {
        Goto(endSec + 1);
    }

    //移动到指定时间点
    public void Runto(double sec)
    {
        if (playMode != PlayMode_Step) {
            Goto(sec);
            return;
        }

        if (sec >= curtPlaySec) {
            Process(sec - curtPlaySec, true);
        }
        else {
            Goto(sec);
        }
    }

    //移动到指定时间点
    public void GraphRunto(double sec)
    {
        if (state != PLAY || playMode != PlayMode_Step) {
            _Goto(sec);
            return;
        }

        if (sec >= curtPlaySec) {
            Process(sec - curtPlaySec, true);
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

        Process(perFrameCostSec * speed, isStepOp);
    }

    //处理
    protected void Process(double sec, boolean isStepOp)
    {
        if (playMode == PlayMode_Step && !isStepOp)
            return;

        if (state != PLAY)
            return;

        if(!IsWait())
            isWaitForGraph = false;

        if(isWaitForGraph)
            sec = 0;

        ProcessCore(sec, false);

        if(curtPlaySec > endSec && curtPlaySec > GetLastMeasureEndSec()) {
            _Pause();
            state = ENDPAUSE;
        }
    }

    protected void ProcessCore(double sec, boolean isDirectGoto)
    {
        curtPlaySec += sec;
        if(isDirectGoto)
        {
            UpdateNotePlayState();
            UpdateNoteMissState();
        }else{
            //同步播放时间
            double ndkPlaySec = ndkGetPlaySec(ndkEditor);
            if (ndkPlaySec - curtPlaySec > 0.1)
                curtPlaySec = ndkPlaySec;
        }


        int n = 2;
        isNeedSyncSec = true;
        if(playMode != PlayMode_Wait || isWaitForGraph)
        {
            n = 1;
            isNeedSyncSec = false;
        }

        while (n > 0) {
            n--;
            for (int i = 0; i < tracks.length; i++)
                ProcessTrack(tracks[i]);

            if (isSimpleMode)
                ProcessSimpleModeNoteTrack();

            if(!isWaitForGraph)
                break;
        }

        UpdateWaitEndEvs();

        //
        StepMeasure();
    }



    protected void ProcessTrack(Track track)
    {
        NoteOnEvent ev;
        NoteOnEvent[] noteOnEvents = track.noteOnEvents;
        if(noteOnEvents == null || noteOnEvents.length == 0)
            return;

        int i = track.noteOnEventsOffset;
        for(; i < noteOnEvents.length; i++)
        {
            ev = noteOnEvents[i];
            if (ev.startSec > curtPlaySec)
                break;

            if(IsPointerPlayNote(ev)) {

                if(playMode == PlayMode_Wait && isNeedSyncSec &&
                        !ev.isPlay && !ev.isMiss)
                {
                    if(ev.startSec < curtPlaySec)
                        curtPlaySec = ev.startSec;
                    isWaitForGraph = true;
                    return;
                }

                waitEndEvs[waitEndEvCount++] = ev;
            }
        }

        track.noteOnEventsOffset = i;
    }

    /**处理简单模式下的音符轨道*/
    protected void ProcessSimpleModeNoteTrack()
    {
        if(simpleModeTrackNotes == null || simpleModeTrackNotes.length <= 0)
            return;

        NoteOnEvent ev;
        int i = simpleModeNoteTrackOffset;
        for(;  i<simpleModeTrackNotes.length; i++)
        {
            ev = simpleModeTrackNotes[i];
            if (ev.startSec > curtPlaySec)
                break;

            if(playMode == PlayMode_Wait && isNeedSyncSec &&
                    !ev.isPlay && !ev.isMiss)
            {
                if(ev.startSec < curtPlaySec)
                    curtPlaySec = ev.startSec;
                isWaitForGraph = true;
                return;
            }

            waitEndEvs[waitEndEvCount++] = ev;
        }

        simpleModeNoteTrackOffset = i;
    }


    /**更新等待结束的MidiEvs*/
    protected void UpdateWaitEndEvs()
    {
        //按时间排序需要等待弹奏的Notes
        Arrays.sort(waitEndEvs, 0, waitEndEvCount, noteCmp);
        waitEndEvCount = 0;

        NoteOnEvent ev;
        for (int i = 0; waitEndEvs[i] != null; i++)
        {
            waitEndEvCount++;
            ev = (NoteOnEvent) waitEndEvs[i];

            if(!ev.isMiss && !ev.isPlay &&
                    curtPlaySec - ev.startSec > 0.2f)
            {
                ev.isMiss = true;
                if (missNoteCount > 0 && ev.startSec < missNoteEv[missNoteCount - 1].startSec)
                {
                    for (int j = missNoteCount - 1; j >= 0; j--) {
                        if(missNoteEv[j].startSec <= ev.startSec) {
                            missNoteEv[j + 1] = ev;
                            break;
                        }
                        else {
                            missNoteEv[j + 1] = missNoteEv[j];
                            if(j == 0) {missNoteEv[0] = ev; break;}
                        }
                    }
                    missNoteCount++;
                } else {
                    missNoteEv[missNoteCount++] = ev;
                }
            }

            if (ev.endSec <= curtPlaySec)
                waitEndEvs[i] = null;
        }
    }




    /**步进小节到当前时间点*/
    protected void StepMeasure()
    {
        if(measureInfo == null)
            return;

        float measureSec;
        for(int i = measureInfo.atMeasure; i <= measureInfo.GetMeasureCount(); i++)
        {
            measureSec = measureInfo.GetMeasureStartSec(i);
            if(measureSec >= curtPlaySec)
                return;
            measureInfo.atMeasure = i;
        }

        measureInfo.atMeasure = measureInfo.GetMeasureCount();
    }

    protected void UpdateNotePlayState()
    {
        UpdateNotePlayStateToSec(curtPlaySec);
    }

    /**更新音符弹奏状态到指定时间点*/
    public void UpdateNotePlayStateToSec(double sec)
    {
        for(int i = playedNoteCount - 1; i >= 0; i--)
        {
            if(playedNoteEv[i].startSec > sec) {
                playGamePoint -= playedNoteEv[i].gamePoint;
                playedNoteEv[i].gamePoint = 0;
                playedNoteEv[i].lateSec = 0;

                if(!isPlayRecord)
                    playedNoteEv[i].lateDownSec = 0;

                playedNoteEv[i].isPlay = false;
                playedNoteCount--;
            }
            else{
                break;
            }
        }

        if(playGamePoint < 0)
            playGamePoint = 0;
    }


    protected void UpdateNoteMissState()
    {
        for(int i = missNoteCount - 1; i >= 0; i--)
        {
            if(missNoteEv[i].startSec > curtPlaySec) {
                missNoteEv[i].isMiss = false;
                missNoteEv[i].lateSec = 0;

                if(!isPlayRecord)
                    missNoteEv[i].lateDownSec = 0;

                missNoteCount--;
            }
            else{
                break;
            }
        }
    }




    /**获取当前可视midi事件*/
    public VisualMidiEvents GetCurtVisualMidiEvents()
    {
        return visualMidiEvents;
    }


    protected int[] noteEvCount = new int[128];

    /**
     * 根据当前时间点，生成指定时长范围的可视midi事件
     * @param secWidth 时长
     * @param trackFilter 需要获取的轨道标号列表
     * */
    public VisualMidiEvents CreateCurtVisualMidiEvents(float secWidth, int[] trackFilter) {
        List<List<NoteOnEvent>> noteOnEvents = visualMidiEvents.GetAllKeyEvents();

        //
        visualMidiEvents.ClearNoteUsedMark();
        visualMidiEvents.usedNoteCount = 0;
        int[] usedNotes = visualMidiEvents.GetUsedNotes();

        //
        Track track;
        MidiEvent ev;
        double endSec = curtPlaySec + secWidth + 0.1f;
        curtNeedPlayNoteCount = 0;
        curtNeedPlayStartSec = (float) endSec;

        for (int i = 0; i < tracks.length; i++)
        {
            if (trackFilter != null && !Utils.IsContainIntValue(trackFilter, i))
                continue;

            track = tracks[i];
            if (track.noteOnEvents.length == 0 ||
                    track.GetNoteColor() == MidiEvent.NoteColor_None)
                continue;

            //往前寻找一个endSec大于curtPlaySec的事件
            if(track.noteOnEventsOffset >= track.noteOnEvents.length)
                track.noteOnEventsOffset--;

            for(int j = track.noteOnEventsOffset; j >= 0; j--)
            {
                ev = track.noteOnEvents[j];
                if (ev.type != MidiEvent.NoteOn || ev.endSec < curtPlaySec)
                    continue;

                if(isSimpleMode && IsPointerPlayNote((NoteOnEvent)ev))
                    continue;

                if (j == track.noteOnEventsFirst)
                    break;

                //找一个endSec大于curtPlaySec的事件
                if (ev.endSec >= curtPlaySec)
                    track.noteOnEventsFirst = j;
            }

            for(int j= track.noteOnEventsFirst; j<track.noteOnEvents.length;j++)
            {
                ev = track.noteOnEvents[j];
                if (ev == null || ev.type != MidiEvent.NoteOn || ev.endSec < curtPlaySec)
                    continue;

                NoteOnEvent noteOnEvent = (NoteOnEvent) ev;
                if(isSimpleMode && IsPointerPlayNote(noteOnEvent))
                    continue;

                if (noteOnEvent.startSec > endSec)
                    break;

                //保存当前时间点需要弹奏的noteEvs
                SaveCurtNeedPlayNoteEvent(noteOnEvent);

                //
                List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                noteEvlist.add(noteOnEvent);

                if (visualMidiEvents.noteUsedMark[noteOnEvent.note] != true) {
                    usedNotes[visualMidiEvents.usedNoteCount++] = noteOnEvent.note;
                    visualMidiEvents.noteUsedMark[noteOnEvent.note] = true;
                }

                if (noteEvlist.size() - noteEvCount[noteOnEvent.note] > 100)
                    break;
            }


            //
            for (int j = 0; j < visualMidiEvents.usedNoteCount; j++) {
                List<NoteOnEvent> noteEvlist = noteOnEvents.get(usedNotes[j]);
                noteEvCount[usedNotes[j]] = noteEvlist.size();
            }
        }


        //对简单模式的弹奏轨道处理
        if(isSimpleMode && simpleModeTrackNotes != null && simpleModeTrackNotes.length > 0)
        {
            if(simpleModeNoteTrackOffset >= simpleModeTrackNotes.length)
                simpleModeNoteTrackOffset--;
            for (int i = simpleModeNoteTrackOffset; i >=0; i--) {
                ev = simpleModeTrackNotes[i];
                if (ev.endSec < curtPlaySec)
                    continue;
                if (i == simpleModeNoteTrackFirst)
                    break;
                //找一个endSec大于curtPlaySec的事件
                if (ev.endSec >= curtPlaySec)
                    simpleModeNoteTrackFirst = i;
            }

            //
            for(int i=simpleModeNoteTrackFirst; i<simpleModeTrackNotes.length; i++)
            {
                NoteOnEvent noteOnEvent = simpleModeTrackNotes[i];
                if (noteOnEvent.endSec < curtPlaySec)
                    continue;
                if (noteOnEvent.startSec > endSec)
                    break;

                //保存当前时间点需要弹奏的noteEvs
                SaveCurtNeedPlayNoteEvent(noteOnEvent);

                //
                List<NoteOnEvent> noteEvlist = noteOnEvents.get(noteOnEvent.note);
                noteEvlist.add(noteOnEvent);

                if (visualMidiEvents.noteUsedMark[noteOnEvent.note] != true) {
                    usedNotes[visualMidiEvents.usedNoteCount++] = noteOnEvent.note;
                    visualMidiEvents.noteUsedMark[noteOnEvent.note] = true;
                }

                if (noteEvlist.size() - noteEvCount[noteOnEvent.note] > 100)
                    break;
            }
        }


        //
        visualMidiEvents.SortUsedNotes();


        //生成noteOn轨道分组
        List<int[]> noteOnTrackGroups = visualMidiEvents.noteOnTrackGroups;
        int m;
        List<NoteOnEvent> notes;
        int[] noteOnTrackGroup;
        int trackidx;
        int noteCount;

        for(int j=0; j<visualMidiEvents.usedNoteCount; j++)
        {
            m = -1;
            trackidx = -1;
            notes = noteOnEvents.get(usedNotes[j]);
            noteCount = notes.size();
            noteOnTrackGroup = noteOnTrackGroups.get(usedNotes[j]);
            noteOnTrackGroup[0] = 1;  //保存最后位置到数组0位
            noteOnTrackGroup[1] = noteCount;
          //  noteOnTrackGroup[2] = noteCount;


            //由于在notes中，所有的NoteOnEvent是按照trackidx增序排列的,所以对NoteOnEvent分组，就是遍历所有item，
            //一但发现noteOnEvent的trackidx发生变动，即说明进入到下一组trackidx
            for (int i = 0; i < noteCount; i++) {
                NoteOnEvent noteOnEV = notes.get(i);
                if(noteOnEV.trackIdx != trackidx) {
                    m+=2;
                    noteOnTrackGroup[m] = i;
                    noteOnTrackGroup[m + 1] = i;
                    noteOnTrackGroup[m + 2] = noteCount;
                    noteOnTrackGroup[0] = m + 2; //保存最后位置到数组0位
                    trackidx = noteOnEV.trackIdx;
                }
            }

            //
            int end = noteOnTrackGroup[0];
            int count = 0, missCount, b;
            boolean flag = false;
            boolean isLoop = true;

            while(isLoop)
            {
                missCount = 0;

                for (int i = 1; i < end; i+=2)
                {
                    b = noteOnTrackGroup[i + 2] - 1;
                    if (noteOnTrackGroup[i + 1] == b) {
                        missCount+=2;
                        continue;
                    }

                    noteOnTrackGroup[i + 1]++;
                    count++;
                    if(count >= 40 && flag) {
                        isLoop = false;
                        break;
                    }
                }

                if(missCount >= end - 1)
                    isLoop = false;
                flag = true;
            }
        }

        return visualMidiEvents;
    }


    /**保存当前时间点需要弹奏的noteEv*/
    protected void SaveCurtNeedPlayNoteEvent(NoteOnEvent noteOnEvent)
    {
        if (!noteOnEvent.isMiss && !noteOnEvent.isPlay && IsPointerPlayNote(noteOnEvent))
        {
            //当前note开始时间点大于已知的最低时间点0.1sec
            if(noteOnEvent.startSec - curtNeedPlayStartSec >= 0.1)
                return;

            //已知的最低时间点高于当前note开始时间点，说明所有已知的需要弹奏note比当前note高出0.1sec
            //将只使用当前note
            if (curtNeedPlayStartSec - noteOnEvent.startSec >= 0.1)
            {
                curtNeedPlayNoteCount = 0;
                curtNeedPlayStartSec = noteOnEvent.startSec;
                curtNeedPlayNoteEvs[curtNeedPlayNoteCount++] = noteOnEvent;
            }
            else
            {
                //当前note开始时间点依然是最低点，此时需要排除已存在的note和当前note大于0.1sec的
                if(curtNeedPlayStartSec - noteOnEvent.startSec >= 0) {
                    int idx = 0;
                    for (int i = 0; i < curtNeedPlayNoteCount; i++) {
                        if (curtNeedPlayNoteEvs[i].startSec - noteOnEvent.startSec < 0.1)
                            curtNeedPlayNoteEvs[idx++] = curtNeedPlayNoteEvs[i];
                    }
                    curtNeedPlayNoteEvs[idx++] = noteOnEvent;
                    curtNeedPlayStartSec = noteOnEvent.startSec;
                    curtNeedPlayNoteCount = idx;
                }else{//当前note被最低点包含,直接添加
                    curtNeedPlayNoteEvs[curtNeedPlayNoteCount++] = noteOnEvent;
                }
            }
        }
    }


    /**是否为手指弹奏的音符*/
    public boolean IsPointerPlayNote(NoteOnEvent noteOnEv)
    {
        Track track = noteOnEv.track;
        //
        if(noteOnEv.childNoteOnEvents != null)
            return true;

        if (track.GetPlayType() == MidiEvent.PlayType_Custom)
        {
            if ((playType == MidiEvent.PlayType_DoubleHand &&
                    noteOnEv.GetPlayType() != MidiEvent.PlayType_Background) ||

                    (playType == MidiEvent.PlayType_LeftHand &&
                            noteOnEv.GetPlayType() ==  MidiEvent.PlayType_LeftHand) ||

                    (playType == MidiEvent.PlayType_RightHand &&
                            noteOnEv.GetPlayType() ==  MidiEvent.PlayType_RightHand))
            {
                return true;
            }

            return false;
        }

        if (playType == MidiEvent.PlayType_DoubleHand &&
                track.GetPlayType() !=  MidiEvent.PlayType_Background)
            return true;

        if (playType == track.GetPlayType())
            return true;

        return false;
    }


    /**是否为手指弹奏的轨道*/
    public boolean IsPointerPlayTrack(Track track)
    {
        //
        if (track.GetPlayType() == MidiEvent.PlayType_Custom)
            return true;

        if (playType == MidiEvent.PlayType_DoubleHand &&
                track.GetPlayType() !=  MidiEvent.PlayType_Background)
            return true;

        if (playType == track.GetPlayType())
            return true;

        return false;
    }


    //
    private MidiMarker[] _ndkMidiMarkers;

    private long ndkEditor;
    public long GetNdkEditor() {
        return ndkEditor;
    }

    //
    private static native void ndkInit(Editor editor, long ndkEditor);

    private static native void ndkSetPlayType(long ndkEditor, int playType);
    private static native void ndkEnterWaitPlayMode(long ndkEditor);
    private static native void ndkEnterMuteMode(long ndkEditor);
    private static native void ndkLeavePlayMode(long ndkEditor);
    private static native void ndkSetTrackPlayType(long ndkEditor, int trackIdx, int playType);

    private static native void ndkSetExcludeNeedWaitKey(long ndkEditor, int key);
    private static native void ndkSetIncludeNeedWaitKey(long ndkEditor, int key);
    private static native void ndkOnKeySignal(long ndkEditor, int key);
    private static native void ndkOffKeySignal(long ndkEditor, int key);

    private static native void ndkDisableTrack(long ndkEditor, int trackIdx);
    private static native void ndkDisableAllTrack(long ndkEditor);
    private static native void ndkEnableTrack(long ndkEditor, int trackIdx);
    private static native void ndkEnableAllTrack(long ndkEditor);


    private static native void ndkOnKeyAtTrack(long ndkEditor,
                                               int key, float velocity, int trackIdx, int id);

    private static native void ndkOffKeyAtTrack(long ndkEditor,
                                                int key, float velocity, int trackIdx, int id);

    private static native void ndkOffAllKeys(long ndkEditor);


    private static native boolean ndkIsLoadCompleted(long ndkEditor);
    private static native void ndkLoad(Editor editor, long ndkEditor, String midifile, boolean isWaitLoadCompleted);
    private static native void ndkSetSimpleModePlayWhiteKeyCount(long ndkEditor, int count);
    private static native void ndkSetMergeSimpleSrcNoteLimitSec(long ndkEditor, float sec);
    private static native void ndkSetMergeSimpleDestNoteLimitSec(long ndkEditor, float sec);

    private static native void ndkCreateSimpleModeTrack(Editor editor, long ndkEditor);
    private static native void ndkCreateDatas(Editor editor, long ndkEditor);
    private static native void ndkSetInitStartPlaySec(long ndkEditor, double sec);
    private static native void ndkSetNoteSoundStartSec(long ndkEditor, double sec);
    private static native void ndkSetNoteSoundEndSec(long ndkEditor, double sec);

    private static native void ndkWait(long ndkEditor);
    private static native void ndkContinue(long ndkEditor);
    private static native boolean ndkIsWait(long ndkEditor);
    private static native boolean ndkIsWaitKey(long ndkEditor, int key);
    private static native boolean IsWaitNoteOnEvent(long ndkEditor, long ndkNoteOnEvPtr);
    private static native int ndkGetWaitKeyCount(long ndkEditor, int key);
    private static native boolean ndkHavWaitKey(long ndkEditor);
    private static native void ndkOnWaitKeysSignal(long ndkEditor);


    private static native void ndkPlay(long ndkEditor);
    private static native void ndkPause(long ndkEditor);
    private static native void ndkStop(long ndkEditor);
    private static native void ndkRemove(long ndkEditor);
    private static native void ndkGoto(long ndkEditor, double sec);
    private static native int ndkGetSecTickCount(long ndkEditor, double sec);
    private static native float ndkGetCurtBPM(long ndkEditor);
    private static native double ndkGetTickSec(long ndkEditor, int tick);
    private static native double ndkGetPlaySec(long ndkEditor);
    private static native double ndkGetEndSec(long ndkEditor);
    private static native void ndkSetSpeed(long ndkEditor, float speed);
    private static native int ndkGetPlayState(long ndkEditor);
    private static native int ndkGetCacheState(long ndkEditor);
    private static native int ndkGetSampleStreamFreqSpectrums(long ndkEditor, int channel, double[] outLeft, double[] outRight);
}
