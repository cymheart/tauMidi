package cymheart.tau.editor;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import cymheart.tau.utils.FileUtils;
import cymheart.tau.utils.ScPool;
import cymheart.tau.utils.Utils;

/**弹奏录制器*/

/**
 * ----- JsonRecordNotes结构-------
 * [
 *   recodenote0:
 *   {
 *        type:int
 *        note:int
 *        velocity:int
 *        track:int
 *        sec:float
 *   }
 *      ...
 *   recodenoten:
 *   {
 *      ...
 *   }
 * ]
 *
 * */
public class PlayRecorder {

    /**录制中*/
    static public final int STATE_RECORDING = 1;
    /**播放中*/
    static public final int STATE_PLAYING = 2;

    protected Editor editor;
    private final ScPool<RecordNote> notePool = new ScPool<>(null);

    public Utils.Action<RecordNote> PlayRecordOnKey = null;
    public Utils.Action<RecordNote> PlayRecordOffKey = null;

    private RecordNote NewRecordNote(Object obj)
    {
        return new RecordNote();
    }

    protected RecordNote PopNote()
    {
        return notePool.Pop();
    }

    protected void PushNote(RecordNote note)
    {
        notePool.Push(note);
    }

    protected List<RecordNote> notes = new ArrayList<>();
    public List<RecordNote> GetNotes(){
        return notes;
    }

    public boolean IsEmpty()
    {
        return notes.isEmpty();
    }

    public int noteOffsetIdx = 0;
    public void ResetOffset()
    {
        noteOffsetIdx = 0;
    }

    private boolean havWaitEvent = false;

    /**录制状态<P>
     * recordState = STATE_RECORDING : 录制中<P>
     * recordState = STATE_PLAYING : 播放中<P>
     * */
    protected int recordState = STATE_RECORDING;
    /**设置录制状态*/
    public void SetRecordState(int state){
        recordState = state;
    }

    public PlayRecorder(Editor editor)
    {
        this.editor = editor;
        notePool.NewMethod = this::NewRecordNote;
        notePool.createCount = 10;
        notePool.SetUseOpLock(true);
        notePool.CreatePool(50);
    }


    public void Clear()
    {
        havWaitEvent = false;
        noteOffsetIdx = 0;
        recordState = 1;
    }

    /**录制按压按键动作*/
    public void OnKey(int note)
    {
        double sec = editor.isWaitForGraph ? curtWaitLateSec : editor.curtPlaySec;
        OnKey(note, 127, 0, (float)sec);
    }

    /**录制松开按键动作*/
    public void OffKey(int note)
    {
        double sec = editor.isWaitForGraph ? curtWaitLateSec : editor.curtPlaySec;
        OffKey(note,0, (float)sec);
    }

    /**录制按下按键*/
    public void OnKey(int note, int vel, int track, float sec)
    {
        if(recordState != STATE_RECORDING)
            return;

        RecordNote recordNote = PopNote();
        recordNote.type = RecordNote.On;
        recordNote.note = note;
        recordNote.velocity = vel;
        recordNote.track = track;
        recordNote.sec = sec;

        notes.add(recordNote);
    }

    /**录制松开按键*/
    public void OffKey(int note, int track, float sec)
    {
        if(recordState != STATE_RECORDING)
            return;

        RecordNote recordOffNote = PopNote();
        recordOffNote.type = RecordNote.Off;
        recordOffNote.note = note;
        recordOffNote.velocity = 127;
        recordOffNote.track = track;
        recordOffNote.sec = sec;

        notes.add(recordOffNote);
    }

    /**录制继续事件*/
    protected void Continue(float sec)
    {
        RecordNote recordNote = PopNote();
        recordNote.type = RecordNote.Continue;
        recordNote.sec = sec;
        notes.add(recordNote);
    }

    /**录制等待事件*/
    protected void Wait(float sec)
    {
        havWaitEvent = true;
        RecordNote recordNote = PopNote();
        recordNote.type = RecordNote.Wait;
        recordNote.sec = sec;
        notes.add(recordNote);
    }

    /**移除所有note*/
    public void RemoveAll()
    {
        RemoveAfterSec(-1);
        notes.clear();
        havWaitEvent = false;
        noteOffsetIdx = 0;
    }


    private final boolean[] havRecordNotes = new boolean[127];
    private final List<RecordNote> saveRecordNotes = new ArrayList<>();

    /**移除指定时间点之后的所有note*/
    public void RemoveAfterSec(float sec)
    {
        if(recordState == STATE_PLAYING)
            return;

        saveRecordNotes.clear();
        for(int i=0; i<127; i++)
            havRecordNotes[i] = false;

        //
        RecordNote recordNote;
        int end = 0;
        for(int i = 0; i<notes.size(); i++)
        {
            recordNote = notes.get(i);
            if(havWaitEvent && recordNote.type != RecordNote.Wait)
                continue;

            end = i;
            if(havWaitEvent)
            {
                if (recordNote.sec >= sec)
                    break;
            } else {

                if (recordNote.sec >= sec)
                {
                    for (i++; i < notes.size(); i++) {
                        recordNote = notes.get(i);
                        if (recordNote.type == RecordNote.Off && havRecordNotes[recordNote.note]) {
                            saveRecordNotes.add(recordNote);
                            notes.set(i, null);
                        }
                    }

                    break;
                }

                if (recordNote.type == RecordNote.On)
                    havRecordNotes[recordNote.note] = true;
            }
        }

        //
        for(int j = notes.size() - 1; j >= end; j--)
        {
            recordNote = notes.get(j);
            if(recordNote != null)
                PushNote(recordNote);
            notes.remove(j);
        }

        //
        notes.addAll(saveRecordNotes);
        saveRecordNotes.clear();
    }


    /**播放录制goto动作*/
    public void Goto()
    {
        if(recordState != STATE_PLAYING)
            return;

        boolean isRecordWait = false;
        RecordNote recordNote;

        for(int i = noteOffsetIdx; i < notes.size(); i++)
        {
            recordNote = notes.get(i);

            if(recordNote.type == RecordNote.Wait) {
                isRecordWait = true;
                if(recordNote.sec >= editor.curtPlaySec) {
                    noteOffsetIdx = i;
                    return;
                }
            }
            else if(recordNote.type == RecordNote.Continue)
            {
                isRecordWait = false;
            }
            else if(!isRecordWait && recordNote.sec >= editor.curtPlaySec) {
                noteOffsetIdx = i;
                return;
            }
        }

        noteOffsetIdx = notes.size();
    }


    /**更新播放*/
    private void UpdatePlay()
    {
        if(recordState != STATE_PLAYING)
            return;

        RecordNote recordNote;
        int i = noteOffsetIdx;
        for(; i < notes.size(); i++)
        {
            recordNote = notes.get(i);

            if(!((editor.isWaitForGraph && recordNote.sec <= curtWaitLateSec ) ||
                    (!editor.isWaitForGraph && recordNote.sec <= editor.curtPlaySec))) {
                break;
            }

            if (recordNote.type == RecordNote.Wait) {
                editor.Wait();
                editor.isWaitForGraph = true;
                i++;
                break;
            } else if (recordNote.type == RecordNote.Continue) {
                editor.Continue();
                editor.isWaitForGraph = false;
                i++;
                break;
            } else if (recordNote.type == RecordNote.On) {
                if(PlayRecordOnKey != null)
                    PlayRecordOnKey.Execute(recordNote);
            } else {
                if(PlayRecordOffKey != null)
                    PlayRecordOffKey.Execute(recordNote);
            }
        }

        noteOffsetIdx = i;
    }


    protected boolean oldIsWait = false;

    /**当前等待延迟时长*/
    protected double curtWaitLateSec = 0;

    /**更新*/
    public void Update()
    {
        UpdatePlay();

        //
        if (editor.isWaitForGraph)
        {
            if(!oldIsWait) {
                curtWaitLateSec = 0;
                if(recordState == 1)
                    Wait((float) (editor.curtPlaySec - editor.perFrameCostSec));
            }
            curtWaitLateSec += editor.perFrameCostSec;
        } else {
            if(oldIsWait && recordState == STATE_RECORDING)
               Continue((float)(curtWaitLateSec - editor.perFrameCostSec));
            curtWaitLateSec = 0;
        }

        oldIsWait = editor.isWaitForGraph;
    }


    /**保存录制到文件*/
    public void SaveToFile(String filePath)
    {
        if(notes.isEmpty())
            return;

        try {
            JSONArray jsonNotes = new JSONArray();
            JSONObject jsonNote;
            RecordNote recordNote;
            for(int i = 0; i < notes.size(); i++)
            {
                recordNote = notes.get(i);
                jsonNote = new JSONObject();
                jsonNote.put("type", recordNote.type);
                jsonNote.put("note", recordNote.note);
                jsonNote.put("velocity", recordNote.velocity);
                jsonNote.put("track", recordNote.track);
                jsonNote.put("sec", recordNote.sec);
                jsonNotes.put(jsonNote);
            }

            //
            String jsonStr = jsonNotes.toString();
            InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
            FileUtils.getInstance().WriteToFile(filePath, jsonStream);

        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

    /**从文件中加载录制*/
    public void LoadFromFile(String filePath)
    {
        try {

            RemoveAll();
            //
            String jsonStr = FileUtils.getInstance().ReadFileToString(filePath);
            JSONArray jsonNotes = new JSONArray(jsonStr);

            RecordNote recordNote;
            JSONObject jsonNote;
            for(int i=0; i<jsonNotes.length(); i++)
            {
                jsonNote = jsonNotes.getJSONObject(i);
                recordNote = PopNote();
                //
                recordNote.type = jsonNote.getInt("type");
                recordNote.note = jsonNote.getInt("note");
                recordNote.velocity = jsonNote.getInt("velocity");
                recordNote.track = jsonNote.getInt("track");
                recordNote.sec = (float)jsonNote.getDouble("sec");
                notes.add(recordNote);
            }

        }catch (JSONException e) {
            e.printStackTrace();
        }
    }

}
