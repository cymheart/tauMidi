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

/**弹奏录制器*/

/**
 * ----- JsonRecordNotes结构-------
 * [
 *   recodenote0:
 *   {
 *        type:int
 *        note:int
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

    private ScPool<RecordNote> notePool = new ScPool<>(null);

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


    public PlayRecorder()
    {
        notePool.NewMethod = this::NewRecordNote;
        notePool.createCount = 10;
        notePool.SetUseOpLock(true);
        notePool.CreatePool(50);
    }

    public void Clear()
    {
        havWaitEvent = false;
        noteOffsetIdx = 0;
    }


    /**录制继续事件*/
    public void Continue(float sec)
    {
        RecordNote recordNote = PopNote();
        recordNote.type = RecordNote.Continue;
        recordNote.sec = sec;
        notes.add(recordNote);
    }

    /**录制等待事件*/
    public void Wait(float sec)
    {
        havWaitEvent = true;
        RecordNote recordNote = PopNote();
        recordNote.type = RecordNote.Wait;
        recordNote.sec = sec;
        notes.add(recordNote);
    }


    /**录制按下按键*/
    public void OnKey(int note, int vel, int track, float sec)
    {
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
        RecordNote recordOffNote = PopNote();
        recordOffNote.type = RecordNote.Off;
        recordOffNote.note = note;
        recordOffNote.velocity = 127;
        recordOffNote.track = track;
        recordOffNote.sec = sec;

        notes.add(recordOffNote);
    }

    /**移除所有note*/
    public void RemoveAll()
    {
        RemoveAfterSec(-1);
        notes.clear();
        havWaitEvent = false;
        noteOffsetIdx = 0;
    }

    /**移除指定时间点之后的所有note*/
    public void RemoveAfterSec(float sec)
    {
        RecordNote recordNote;
        int i=0;
        for(; i<notes.size(); i++) {
            recordNote = notes.get(i);
            if(havWaitEvent && recordNote.type != RecordNote.Wait)
                continue;

            if(recordNote.sec >= sec)
                break;
        }

        //
        for(int j= notes.size() - 1; j>=i; j--)
        {
            recordNote = notes.get(j);
            PushNote(recordNote);
            notes.remove(j);
        }
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
