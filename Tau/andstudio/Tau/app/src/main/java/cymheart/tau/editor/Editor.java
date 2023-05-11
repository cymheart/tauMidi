package cymheart.tau.editor;


import android.annotation.SuppressLint;
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
import cymheart.tau.editor.data.DataDirectoryMgr;
import cymheart.tau.editor.data.GameVerData;
import cymheart.tau.midi.MidiEvent;
import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.FileUtils;
import cymheart.tau.utils.MD5Utils;
import cymheart.tau.utils.Utils;


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

    /**判断note是否为白色*/
    static public boolean IsWhiteNote(int note)
    {
        return keyTypes88Std[note - A0] == KeyWhite;
    }

    /**判断note是否为黑色*/
    static public boolean IsBlackNote(int note)
    {
        return keyTypes88Std[note - A0] == KeyBlack;
    }

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
        if(note - A0 >= keyTypes88Std.length)
            return A0;

        for(int i = note - A0; i >=0; i--)
            if(keyTypes88Std[i] == KeyWhite)
                return i + A0;
        return A0;
    }

    /**获取下一个白色类型note*/
    static public int GetNextWhiteNote(int note)
    {
        if(note - A0 < 0)
            return C8;

        for(int i = note - A0; i < keyTypes88Std.length; i++)
            if(keyTypes88Std[i] == KeyWhite)
                return i + A0;
        return C8;
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

    private static class MarkerComparator implements Comparator<MidiMarker> {
        @Override
        public int compare(MidiMarker left, MidiMarker right) {
            return left.startSec < right.startSec ? -1: 1;
        }
    }

    private static class NoteComparator implements Comparator<NoteOnEvent> {

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
    protected List<NoteOnEvent> needPlayNoteEvsSortTime = new ArrayList<>();
    /**获取需要弹奏的音符，按时间顺序存放*/
    public List<NoteOnEvent> GetNeedPlayNoteEvsSortTime()
    {
        return needPlayNoteEvsSortTime;
    }

    /**获取第一个需要弹奏音符的时间点*/
    public float GetFirstNeedPlayNoteSec()
    {
        return needPlayNoteEvsSortTime.get(0).startSec;
    }

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
            for(int i=0; i<needPlayNoteEvsSortTime.size(); i++) {
                if(!GetKeyboardCenter(needPlayNoteEvsSortTime.get(i), visualWidth, minKeyWidth))
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
        if(minNote == -1 || ev.num <minNote)
            minNote = ev.num;
        if(maxNote == -1 || ev.num > maxNote)
            maxNote = ev.num;

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

    /**是否开启伴奏*/
    protected boolean isOpenAccompany = true;
    /**设置是否开启伴奏*/
    public void SetOpenAccompany(boolean isOpen)
    {
        isOpenAccompany = isOpen;
        ndkSetOpenAccompany(ndkEditor, isOpen);
    }

    /**获取Native midi文件相关的用户目录*/
    public String GetNativeMidiFileDir(String midiFilePath, String userName){
        return dataDirectoryMgr.GetNativeMidiFileDir(midiFilePath, userName);
    }

    /**获取Native midi目录相关的用户目录*/
    public String GetNativeMidiDirDir(String midiDirPath, String userName){
        return dataDirectoryMgr.GetNativeMidiDirDir(midiDirPath, userName);
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

    /**midi扩展文件MD5*/
    protected String midiExFileMD5;
    /**用户名称*/
    protected String user;
    /**用户数据midi文件目录路径*/
    protected String userDataMidiDirPath;
    /**midi名称*/
    protected String midiName;
    /**midi版本名称*/
    protected String midiVersion;


    /**是否为本地midi*/
    protected boolean isNativeMidi = false;
    /**设置是否为本地midi*/
    public void SetNativeMidi(boolean nativeMidi) {
        isNativeMidi = nativeMidi;
    }

    /**数据目录管理*/
    protected DataDirectoryMgr dataDirectoryMgr;


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
        int s = isPlayRecord ? PlayRecorder.STATE_PLAYING : PlayRecorder.STATE_RECORDING;
        playRecorder.SetRecordState(s);
    }

    /**判断是否弹奏录制*/
    public boolean IsPlayRecord()
    {
        return isPlayRecord;
    }

    /**弹奏历史*/
    protected PlayHistory playHistory = new PlayHistory(this);

    /**弹奏录制器*/
    protected PlayRecorder playRecorder = new PlayRecorder(this);

    /**录制按压按键动作*/
    public void RecordOnKey(int note)
    {
        playRecorder.OnKey(note);
    }

    /**录制松开按键动作*/
    public void RecordOffKey(int note)
    {
        playRecorder.OffKey(note);
    }


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

    /**当前播放时间点*/
    protected double curtPlaySec = 0;
    /**获取当前播放时间点*/
    public double GetPlaySec()
    {
        return curtPlaySec;
    }

    /**结束时间点*/
    protected double endSec = 0;
    /**获取结束时间点*/
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
    public boolean IsWaitNote(NoteOnEvent note)
    {
        return IsWaitNoteOnEvent(ndkEditor, note.getNdkMidiEvent());
    }

    //是否有等待中的按键
    public boolean HavWaitKey()
    {
        return ndkHavWaitKey(ndkEditor);
    }

    //所有等待按键信号
    protected void OnWaitKeysSignal()
    {
        ndkOnWaitKeysSignal(ndkEditor);
    }

    //
    protected VisualNotes visualNotes = new VisualNotes();

    /**乐谱弹奏的最左白色音符*/
    protected int musicScoreLeftWhiteNote = 0;
    /**获取乐谱弹奏的最左白色音符*/
    public int GetMusicScoreLeftWhiteNote(){
        return musicScoreLeftWhiteNote;
    }

    /**乐谱弹奏的最右白色音符*/
    protected int musicScoreRightWhiteNote = 0;
    /**获取乐谱弹奏的最右白色音符*/
    public int GetMusicScoreRightWhiteNote(){
        return musicScoreRightWhiteNote;
    }

    /**乐谱横向白键数量*/
    protected int musicScoreLandscapeWhiteNoteCount = 0;
    /**获取乐谱弹奏的最右白色音符*/
    public int GetMusicScoreLandscapeWhiteNoteCount(){
        return musicScoreLandscapeWhiteNoteCount;
    }

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


    /**分数*/
    protected int score = 0;
    /**获取分数*/
    public int GetScore()
    {
        return score;
    }

    /**弹对所有音符按键分值*/
    protected float noteHitScore = 0;
    /**获取弹对所有音符按键分值*/
    public int GetNoteHitScore()
    {
        return (int)noteHitScore;
    }

    /**弹对所有音符时长分值*/
    protected float notePointsScore = 0;
    /**获取弹对所有音符时长分值*/
    public int GetNotePointsScore()
    {
        return (int)notePointsScore;
    }

    /**弹错音符分值*/
    protected float noteErrorScore = 0;
    /**获取弹错音符分值*/
    public int GetNoteErrorScore()
    {
        return (int)noteErrorScore;
    }

    /**暂定弹奏倍率*/
    protected float pausePlayMul = 1;
    /**获取暂定弹奏倍率*/
    public float GetPausePlayMul()
    {
        return pausePlayMul;
    }


    /**计算分数*/
    public void ComputeScore()
    {
        //弹对所有音符按键,分值70分
        float a = (float)playedNoteCount / needPlayNoteCount * 0.7f;

        //弹对所有音符时长，分值30分
        float b = playGamePoint / totalGamePoint * 0.3f;

        //弹错每个音符，都扣分，弹错越多，每个错误音符扣分比值越大，最高每个弹错音符扣5分
        float c = 0;
        for(int i=1; i<=playErrorNoteCount; i++) {
            float x = i / 10.0f;
            c += ScoreFx(x, 7)*0.02f;
        }

        //弹奏过程中，有暂定弹奏的行为，按暂停次数，次数越多，按倍率扣分越大
        float d = 1;
//        if(pausePlayCount > 0) {
//            float x = pausePlayCount / 20.0f;
//            if (x > 1) x = 1;
//            d = 1 - ScoreFx(x, 7);
//        }

        //合分
        float s = a + b - c;
        if(s < 0) s = 0;
        s *= d;
        if(s > 1) s = 1;

        //
        noteHitScore = a*100.0f;
        notePointsScore = b*100.0f;
        noteErrorScore = c*100.0f;
        pausePlayMul = d;

        //转换为百分制
        score = Math.round(s*100);
    }

    /**
     * y = (-(((1 + k)*(2 (1 - x) - 1))/(1 + k*Abs[(2 (1 - x) - 1)])) - 1)/2 + 1;
     * */
    protected float ScoreFx(float x, float k)
    {
        float xm = 2*(1 - x) - 1;
        float m =(1 + k)*xm;
        float n = 1 + k*Math.abs(xm);
        return (-m/n - 1) *0.5f + 1;
    }

    /**丢失音符事件*/
    public NoteOnEvent[] missNotes = new NoteOnEvent[10000];
    /**音符丢失*/
    public void NoteMissed(NoteOnEvent note)
    {
        note.isMiss = true;
        missNotes[missNoteCount++] = note;
    }
    
    /**丢失音符数量*/
    public int missNoteCount = 0;
    /**获取丢失音符数量*/
    public int GetMissedNoteCount() {
        return missNoteCount;
    }

    /**已演奏的音符事件*/
    public NoteOnEvent[] playedNotes = new NoteOnEvent[10000];
    /**已演奏音符*/
    public void NotePlayed(NoteOnEvent note)
    {
        note.isPlay = true;
        playedNotes[playedNoteCount++] = note;
    }

    /**已演奏的音符事件数量*/
    public int playedNoteCount = 0;
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

    /**需要手指弹奏并等待结束的notes*/
    public NoteOnEvent[] waitEndNotes = new NoteOnEvent[10000];
    /**需要手指弹奏并等待结束的notes数量*/
    public int waitEndNoteCount = 0;


    /**首先需要弹奏的notes*/
    protected NoteOnEvent[] firstNeedPlayNotes = new NoteOnEvent[1000];
    /**是否为首先需要弹奏的note*/
    public boolean IsFirstNeedPlayNote(NoteOnEvent note)
    {
        for(int i=0; i<firstNeedPlayNoteCount; i++) {
            if(firstNeedPlayNotes[i] == note)
                return true;
        }
        return false;
    }

    /**首先需要需要弹奏的note数量*/
    protected int firstNeedPlayNoteCount = 0;
    /**首先需要需要弹奏的时间点*/
    protected float firstNeedPlayStartSec = 0;

    /**每帧花费秒数*/
    protected double perFrameCostSec = 1/60.0;
    /**设置每帧花费秒数*/
    public void SetPreFrameCostSec(double costSec)
    {
        perFrameCostSec = costSec;
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
        tracks[trackIdx].playType = playType;
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
    protected void OnKeySignal(int key)
    {
        ndkOnKeySignal(ndkEditor, key);
    }

    //按键信号
    protected void OffKeySignal(int key)
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

    private void Init()
    {
        userDatasRootPath = FileUtils.getInstance().GetExtralFilePath(tau.GetContext()) + "/UserDatas/";
        dataDirectoryMgr = new DataDirectoryMgr(tau.GetContext());
        ndkInit(this, ndkEditor);
    }

    private void Release()
    {
        ndkInit(this, ndkEditor);
    }


    //载入历史记录
    public void LoadByHistory(String midifile, boolean isWaitLoadCompleted,  String playHistoryFileName, String user)
    {
        isLoadCompleted = false;
        jsonTracks = null;
        midiFilePath = midifile;
        isLoadPlayHistory = true;
        this.playHistoryFileName = playHistoryFileName;
        this.user = user;

        //
        ndkLoad(this, ndkEditor, midifile, isWaitLoadCompleted);
    }


    //载入
    public void Load(String midifile, String version, String user, boolean isWaitLoadCompleted)
    {
        isLoadCompleted = false;
        jsonTracks = null;
        midiFilePath = midifile;
        midiVersion = version;

        //
        int end = midiFilePath.lastIndexOf('.');
        int start = midiFilePath.lastIndexOf('/', end);
        midiName = midiFilePath.substring(start, end);
        //
        if(user == null) user = "global";
        this.user = user;

        ndkLoad(this, ndkEditor, midifile, isWaitLoadCompleted);

    }

    //载入
    public void Load(String midifile, String version, boolean isWaitLoadCompleted)
    {
        Load(midifile, version, null, isWaitLoadCompleted);
    }

    //载入
    public void Load(String midifile, String version)
    {
        Load(midifile, version);
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
        midiMarkers.addAll(Arrays.asList(_ndkMidiMarkers));
        _ndkMidiMarkers = null;
    }


    private void ParseMidi() throws JSONException
    {
        if(tracks == null)
            return;

        //是否存在midi扩展信息
        needPlayNoteCount = 0;
        missNoteCount = 0;
        playedNoteCount = 0;

        ParseVerData();

        //分配轨道颜色
        AssignTrackColors();

        //重设信息
        ResetInfos();
    }


    protected GameVerData gameVerData;
    protected void ParseVerData(){

        String midiDir;
        if(!isNativeMidi)
            midiDir = dataDirectoryMgr.GetMidiFileDir(midiName, user);
        else
            midiDir = dataDirectoryMgr.GetNativeMidiFileDir(midiFilePath, user);

        String exData = midiDir + midiVersion + "/ex_data";
        gameVerData = new GameVerData(exData);
        gameVerData.Parse();

    }


    /**重设信息*/
    public void ResetInfos()
    {
        Track track;

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
                for (NoteOnEvent simpleModeTrackNote : simpleModeTrackNotes)
                    simpleModeTrackNote.track = track;
            }
        }


        //重新计算总得分点数，和需要弹奏的音符数量， 音符范围
        needPlayNoteCount = 0;
        NoteOnEvent ev;
        int leftNote = -1, rightNote = -1;
        if(!isSimpleMode)
        {
            needPlayNoteEvsSortTime.clear();
            for (Track value : tracks) {
                track = value;
                for (int j = 0; j < track.noteOnEvents.length; j++) {
                    ev = track.noteOnEvents[j];
                    if (!IsPointerPlayNote(ev))
                        continue;
                    needPlayNoteEvsSortTime.add(ev);
                }
            }

            Collections.sort(needPlayNoteEvsSortTime, noteCmp);
            for(int i=0; i<needPlayNoteEvsSortTime.size(); i++)
            {
                ev = needPlayNoteEvsSortTime.get(i);

                //计算乐谱的音符范围
                if(leftNote == -1 || ev.num < leftNote)
                    leftNote = ev.num;
                if(rightNote == -1 || ev.num > rightNote)
                    rightNote = ev.num;

                //计算总需要弹奏的音符数量
                needPlayNoteCount++;
            }

        }else if(simpleModeTrackNotes != null){
            for (NoteOnEvent simpleModeTrackNote : simpleModeTrackNotes) {
                ev = simpleModeTrackNote;

                //计算乐谱的音符范围
                if (leftNote == -1 || ev.num < leftNote)
                    leftNote = ev.num;
                if (rightNote == -1 || ev.num > rightNote)
                    rightNote = ev.num;

                //计算总需要弹奏的音符数量
                needPlayNoteCount++;
            }
        }


        //根据弹奏音符左右范围，获取左右白键
        musicScoreLeftWhiteNote = GetPrevWhiteNote(leftNote);
        musicScoreRightWhiteNote = GetNextWhiteNote(rightNote);
        musicScoreLandscapeWhiteNoteCount =
                GetWhiteNoteCount(musicScoreLeftWhiteNote, musicScoreRightWhiteNote);




        //如果需要弹奏的音符数量超出预设数量，将按照needPlayNoteCount数量生成
        if(needPlayNoteCount > playedNotes.length) {
            playedNotes = new NoteOnEvent[needPlayNoteCount];
            missNotes = new NoteOnEvent[needPlayNoteCount];
            waitEndNotes = new NoteOnEvent[needPlayNoteCount];
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
        for (Track value : tracks) {
            removeCount = 0;
            track = value;
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
            for (NoteOnEvent noteOnEvent : noteOnEvents) {
                if (noteOnEvent != null)
                    newNoteOnEvents[idx++] = noteOnEvent;
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

        playHistory.SaveToFile(historyFilePath);
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

    //获取当前bpm
    public float GetCurtBPM()
    {
        return ndkGetCurtBPM(ndkEditor);
    }

    //开始播放
    public void Play()
    {
        ndkPlay(ndkEditor);

        if (state == PLAY || tracks == null)
            return;
        state = PLAY;
    }

    //暂停播放
    public void Pause()
    {
        ndkPause(ndkEditor);
        _Pause();
    }

    //暂停播放
    protected void _Pause()
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

        for (Track track : tracks)
            track.Clear();

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
            for (Track track : tracks)
                track.Clear();
            tracks = null;
        }

        needPlayNoteEvsSortTime.clear();

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
        System.gc();
    }

    //设置播放的起始时间点
    public void Goto(double sec)
    {
        GraphGoto(sec);
        ndkGoto(ndkEditor, sec);
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

    //设置播放的起始时间点
    public void GraphGoto(double sec)
    {
        if(curtPlaySec == sec)
            return;


        if(measureInfo != null)
            measureInfo.atMeasure = measureInfo.GetSecAtMeasure((float)sec);

        if(sec < curtPlaySec) {
            isWaitForGraph = false;
            waitEndNoteCount = 0;
            //
            for (Track track : tracks)
                track.Clear();

            simpleModeNoteTrackOffset = 0;
            simpleModeNoteTrackFirst = 0;
            //
            playRecorder.ResetOffset();
            playRecorder.RemoveAfterSec((float)sec);
            //
            curtPlaySec = 0;
        }else{
            isWaitForGraph = false;
        }

        ProcessCore(sec - curtPlaySec, true);
        //
        playRecorder.Goto();
    }


    //运行到指定时间点
    public void Runto(double sec)
    {
        GraphRunto(sec);
        ndkRunto(ndkEditor, sec);
    }

    //移动到指定时间点
    public void GraphRunto(double sec)
    {
        if (state != PLAY || playMode != PlayMode_Step) {
            GraphGoto(sec);
            return;
        }

        if (sec >= curtPlaySec) {
            Process(sec - curtPlaySec, true);
        }
        else {
            GraphGoto(sec);
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
        if (state != PLAY ||
                (playMode == PlayMode_Step && !isStepOp))
            return;

        if(isWaitForGraph)
            sec = 0;

        ProcessCore(sec, false);

        playRecorder.Update();

        if(curtPlaySec > endSec && curtPlaySec > GetLastMeasureEndSec())
            state = PAUSE;
    }


    protected void ProcessCore(double sec, boolean isDirectGoto)
    {
        curtPlaySec += sec;

        for (Track track : tracks)
            ProcessTrack(track, isDirectGoto);

        if (isSimpleMode)
            ProcessSimpleModeNoteTrack(isDirectGoto);

        //
        if(isDirectGoto)
        {
            UpdateNotePlayState();
            UpdateNoteMissState();
        }

        UpdateWaitEndNotes();

        //
        StepMeasure();
    }


    protected void ProcessTrack(Track track, boolean isDirectGoto)
    {
        NoteOnEvent note;
        NoteOnEvent[] notes = track.noteOnEvents;
        if(notes == null || notes.length == 0)
            return;

        int i = track.noteOnEventsOffset;
        for(; i < notes.length; i++)
        {
            note = notes[i];
            //note的弹奏时间未到
            if (note.startSec > curtPlaySec)
                break;

            //下面处理到时间需要弹奏的note
            if(IsPointerPlayNote(note)) {
                waitEndNotes[waitEndNoteCount++] = note;
                //在等待模式下，如果播放速度太快，
                //会导致本来在等待位置的note会穿过弹奏线，而停在弹奏线下方位置
                //视觉上是不正确的，此时通过修正note图像显示的curtPlaySec到音符开始位置，
                //来修正图像步进时间跨距过大的问题
                if(playMode == PlayMode_Wait && !isDirectGoto && !note.isPlay &&
                        !note.isMiss && note.startSec <= curtPlaySec) {
                    curtPlaySec = note.startSec;
                    isWaitForGraph = true;
                }
            }
        }

        track.noteOnEventsOffset = i;
    }

    /**处理简单模式下的音符轨道*/
    protected void ProcessSimpleModeNoteTrack(boolean isDirectGoto)
    {
        if(simpleModeTrackNotes == null || simpleModeTrackNotes.length == 0)
            return;

        NoteOnEvent note;
        int i = simpleModeNoteTrackOffset;
        for(;  i<simpleModeTrackNotes.length; i++)
        {
            note = simpleModeTrackNotes[i];
            if (note.startSec > curtPlaySec)
                break;

            waitEndNotes[waitEndNoteCount++] = note;

            //在等待模式下，如果播放速度太快，
            //会导致本来在等待位置的note会穿过弹奏线，而停在弹奏线下方位置
            //视觉上是不正确的，此时通过修正note图像显示的curtPlaySec到音符开始位置，
            //来修正图像步进时间跨距过大的问题
            if(playMode == PlayMode_Wait && !isDirectGoto && !note.isPlay &&
                    !note.isMiss && note.startSec <= curtPlaySec) {
                curtPlaySec = note.startSec;
                isWaitForGraph = true;
            }
        }

        simpleModeNoteTrackOffset = i;
    }


    /**更新等待结束的notes
     * 主要工作：把waitEndNotes列表中超时未弹奏note加入到missNotes列表
     * 把超时notes从waitEndNotes中移除
     * */
    protected void UpdateWaitEndNotes()
    {
        int maxWaitEndNoteCount = waitEndNoteCount;
        //按时间排序需要等待弹奏结束的Notes
        Arrays.sort(waitEndNotes, 0, waitEndNoteCount, noteCmp);
        waitEndNoteCount = 0;

        boolean isOnKeySignal = false;
        int needWaitCount = 0;
        NoteOnEvent note;
        for (int i = 0; waitEndNotes[i] != null && waitEndNoteCount < maxWaitEndNoteCount; i++)
        {
            waitEndNoteCount++;
            note = waitEndNotes[i];

            //在等待模式下，如果检测到当前弹奏中的note被错过或被弹奏，
            //并且在c++端音符等待中，则同步图像的等待
            if(playMode == PlayMode_Wait && !note.isOnKeySignal)
            {
                if(note.isMiss || note.isPlay){
                    OnKeySignal(note.num);
                    note.isOnKeySignal = true;
                    isOnKeySignal = true;
                } else {
                    needWaitCount++;
                }
            }

            //当note的结束时间小于当前时间时，同时note未设置被弹奏丢失状态时,
            //将把note放入到missNotes中
            float missSecWidth = (note.endSec - note.startSec) * 0.3f;
            if(!note.isMiss && !note.isPlay && note.endSec - missSecWidth < curtPlaySec)
            {
                //当note的结束时间小于当前时间时,表示音符弹奏结束，将从waitEndNotes中移除
                waitEndNotes[i] = null;
                note.isMiss = true;
                note.isOnKeySignal = false;

                //note处于弹奏丢失状态，将其放入到missNotes中一个正确的时间位置上
                //相应的其它notes将后移
                if (missNoteCount > 0 && note.startSec < missNotes[missNoteCount - 1].startSec)
                {
                    for (int j = missNoteCount - 1; j >= 0; j--) {
                        if(missNotes[j].startSec <= note.startSec) {
                            missNotes[j + 1] = note;
                            break;
                        }
                        else {
                            //其它notes后移
                            missNotes[j + 1] = missNotes[j];
                            if(j == 0) {missNotes[0] = note; break;}
                        }
                    }
                    missNoteCount++;
                } else {
                    missNotes[missNoteCount++] = note;
                }
            }

            //当note的结束时间小于当前时间时,表示音符弹奏结束，将从waitEndNotes中移除
            else if (note.endSec < curtPlaySec) {
                waitEndNotes[i] = null;
                note.isOnKeySignal = false;
            }
        }

        if(needWaitCount == 0 && isOnKeySignal)
            isWaitForGraph = false;

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
        for(int i = playedNoteCount - 1; i >= 0; i--)
        {
            if(playedNotes[i].startSec > curtPlaySec) {
                playedNotes[i].isPlay = false;
                playedNotes[i].isOnKeySignal = false;
                playedNoteCount--;
            }
            else{
                break;
            }
        }
    }
    
    protected void UpdateNoteMissState()
    {
        for(int i = missNoteCount - 1; i >= 0; i--)
        {
            if(missNotes[i].startSec > curtPlaySec) {
                missNotes[i].isMiss = false;
                missNotes[i].isOnKeySignal = false;
                missNoteCount--;
            }
            else{
                break;
            }
        }
    }

    /**清空弹奏数据*/
    public void ClearPlayData()
    {
        playedNoteCount = 0;
        playErrorNoteCount = 0;
        missNoteCount = 0;
        playGamePoint = 0;
        score = 0;
        //
        NoteOnEvent ev;
        for(int i=0; i<needPlayNoteEvsSortTime.size(); i++){
            ev = needPlayNoteEvsSortTime.get(i);
            ev.isMiss = false;
            ev.isPlay= false;
            ev.lateSec = 0;
            ev.recordLateSec = 0;
            ev.recordIsMiss = false;
            ev.gamePoint = 0;
        }
    }


    /**获取当前可视midi事件*/
    public VisualNotes GetVisualNotes()
    {
        return visualNotes;
    }


    private final int[] noteCount = new int[128];

    /**
     * 根据当前时间点，生成指定时长范围的可视notes
     * @param secWidth 时长
     * @param trackFilter 需要获取的轨道标号列表
     * */
    public VisualNotes CreateVisualNotes(float secWidth, int[] trackFilter) {

        visualNotes.ClearNoteUsedMark();
        int[] usedNotes = visualNotes.GetUsedNotes();

        //
        Track track;
        NoteOnEvent note;
        double startSec = curtPlaySec - 0.2f;   //可视的起始范围，比当前时间点低一些，可以保证音符完全离开可视区域
        double endSec = curtPlaySec + secWidth + 0.1f;   //可视的结束范围，比当前时间宽度高一些
        firstNeedPlayNoteCount = 0;
        firstNeedPlayStartSec = (float) endSec;

        for (int i = 0; i < tracks.length; i++)
        {
            if (trackFilter != null && !Utils.IsContainIntValue(trackFilter, i))
                continue;

            track = tracks[i];
            if (track.noteOnEvents.length == 0 ||
                    track.GetNoteColor() == MidiEvent.NoteColor_None)
                continue;


            //往前寻找一个endSec大于curtPlaySec的事件
            int startIdx = track.noteOnEventsOffset;
            if(startIdx >= track.noteOnEvents.length)
                startIdx--;

            for(int j = startIdx; j >= 0; j--)
            {
                note = track.noteOnEvents[j];
                if (note.type != MidiEvent.NoteOn || note.endSec < startSec)
                    continue;

                if (j == track.noteOnEventsFirst)
                    break;

                //找一个endSec大于curtPlaySec的事件
                if (note.endSec >= startSec)
                    track.noteOnEventsFirst = j;
            }

            for(int j= track.noteOnEventsFirst; j<track.noteOnEvents.length;j++)
            {
                note = track.noteOnEvents[j];
                if (note == null || note.type != MidiEvent.NoteOn || note.endSec < startSec)
                    continue;

                if (note.startSec > endSec)
                    break;


                if(!isOpenAccompany && !IsPointerPlayNote(note))
                    continue;

                //保存当前时间点需要弹奏的note
                AddFirstNeedPlayNote(note);

                //
                List<NoteOnEvent> notelist = visualNotes.GetNoteList(note.num);
                notelist.add(note);

                if (!visualNotes.noteUsedMark[note.num]) {
                    usedNotes[visualNotes.usedNoteCount++] = note.num;
                    visualNotes.noteUsedMark[note.num] = true;
                }

                if (notelist.size() - noteCount[note.num] > 100)
                    break;
            }

            //
            for (int j = 0; j < visualNotes.usedNoteCount; j++)
                noteCount[usedNotes[j]] = visualNotes.GetNoteList(usedNotes[j]).size();
        }


        //对简单模式的弹奏轨道处理
        if(isSimpleMode && simpleModeTrackNotes != null && simpleModeTrackNotes.length > 0)
        {
            int startIdx = simpleModeNoteTrackOffset;
            if(startIdx >= simpleModeTrackNotes.length)
                startIdx--;

            for (int i = startIdx; i >=0; i--) {
                note = simpleModeTrackNotes[i];
                if (note.endSec < startSec)
                    continue;
                if (i == simpleModeNoteTrackFirst)
                    break;
                //找一个endSec大于curtPlaySec的事件
                if (note.endSec >= startSec)
                    simpleModeNoteTrackFirst = i;
            }

            //
            for(int i=simpleModeNoteTrackFirst; i<simpleModeTrackNotes.length; i++)
            {
                note = simpleModeTrackNotes[i];
                if (note.endSec < startSec)
                    continue;
                if (note.startSec > endSec)
                    break;

                //保存当前时间点需要弹奏的note
                AddFirstNeedPlayNote(note);

                //
                List<NoteOnEvent> noteEvlist = visualNotes.GetNoteList(note.num);
                noteEvlist.add(note);

                if (!visualNotes.noteUsedMark[note.num]) {
                    usedNotes[visualNotes.usedNoteCount++] = note.num;
                    visualNotes.noteUsedMark[note.num] = true;
                }

                if (noteEvlist.size() - noteCount[note.num] > 100)
                    break;
            }
        }


        //
        visualNotes.SortUsedNotes();


        //生成noteOn轨道分组
        List<int[]> noteOnTrackGroups = visualNotes.noteOnTrackGroups;
        int m;
        List<NoteOnEvent> notes;
        int[] noteOnTrackGroup;
        int trackidx;
        int noteCount;

        for(int j = 0; j < visualNotes.usedNoteCount; j++)
        {
            m = -1;
            trackidx = -1;
            notes = visualNotes.GetNoteList(usedNotes[j]);
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

        return visualNotes;
    }


    /**添加当前时间点需要弹奏的note*/
    protected void AddFirstNeedPlayNote(NoteOnEvent note)
    {
        if (note.isMiss || note.isPlay ||
                note.endSec <= curtPlaySec ||
                !IsPointerPlayNote(note))
            return;


        //当前note开始时间点大于已知的最低时间点0.1sec
        if(note.startSec - firstNeedPlayStartSec >= 0.1 &&
                note.startSec - curtPlaySec > 0.1f)
            return;

        //已知的最低时间点高于当前note开始时间点，说明所有已知的需要弹奏note比当前note高出0.1sec
        //将只使用当前note
        if (firstNeedPlayStartSec - note.startSec >= 0.1)
        {
            firstNeedPlayNotes[0] = note;
            firstNeedPlayStartSec = note.startSec;
            firstNeedPlayNoteCount = 1;
        }
        else
        {
            //当前note开始时间点依然是最低点，此时需要排除已存在的note和当前note大于0.1sec的
            if(firstNeedPlayStartSec - note.startSec >= 0) {
                int idx = 0;
                for (int i = 0; i < firstNeedPlayNoteCount; i++) {
                    if (firstNeedPlayNotes[i].startSec - note.startSec < 0.1)
                        firstNeedPlayNotes[idx++] = firstNeedPlayNotes[i];
                }
                firstNeedPlayNotes[idx++] = note;
                firstNeedPlayStartSec = note.startSec;
                firstNeedPlayNoteCount = idx;
            }else{//当前note被最低点包含,直接添加
                firstNeedPlayNotes[firstNeedPlayNoteCount++] = note;
            }
        }

    }


    /**是否为手指弹奏的音符*/
    public boolean IsPointerPlayNote(NoteOnEvent note)
    {
        Track track = note.track;
        //
        if(note.childNoteOnEvents != null)
            return true;

        if (track.GetPlayType() == MidiEvent.PlayType_Custom)
        {
            if ((playType == MidiEvent.PlayType_DoubleHand &&
                    note.GetPlayType() != MidiEvent.PlayType_Background) ||

                    (playType == MidiEvent.PlayType_LeftHand &&
                            note.GetPlayType() ==  MidiEvent.PlayType_LeftHand) ||

                    (playType == MidiEvent.PlayType_RightHand &&
                            note.GetPlayType() ==  MidiEvent.PlayType_RightHand))
            {
                return true;
            }

            return false;
        }

        if (playType == MidiEvent.PlayType_DoubleHand &&
                track.GetPlayType() !=  MidiEvent.PlayType_Background)
            return true;

        return playType == track.GetPlayType();
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

        return playType == track.GetPlayType();
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

    private static native void ndkSetOpenAccompany(long ndkEditor, boolean isOpen);

    private static native void ndkPlay(long ndkEditor);
    private static native void ndkPause(long ndkEditor);
    private static native void ndkStop(long ndkEditor);
    private static native void ndkRemove(long ndkEditor);
    private static native void ndkGoto(long ndkEditor, double sec);

    private static native void ndkRunto(long ndkEditor, double sec);

    private static native int ndkGetSecTickCount(long ndkEditor, double sec);
    private static native float ndkGetCurtBPM(long ndkEditor);
    private static native double ndkGetTickSec(long ndkEditor, int tick);
    private static native double ndkGetPlaySec(long ndkEditor);
    private static native double ndkGetEndSec(long ndkEditor);
    private static native void ndkSetSpeed(long ndkEditor, float speed);
    private static native int ndkGetPlayState(long ndkEditor);
    private static native int ndkGetCacheState(long ndkEditor);

}
