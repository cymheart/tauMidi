package cymheart.tau.editor;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

import cymheart.tau.midi.NoteOnEvent;
import cymheart.tau.utils.FileUtils;


/**
 * 弹奏记录
 * ----JsonPlayHistory结构----
 *{
 *  accounts:
 *  {
 *    playNoteCount:int
 *    errorNoteCount:int
 *    missNoteCount:int
 *    pausePlayCount:int
 *    gamePoint:int
 *    totalGamePoint:int
 *    totalNoteCount:int
 *    score:int
 *  },
 *
 *  tracks:
 *  [
 *    track0:
 *    {
 *       index:int;
 *       notes:
 *      [
 *        note0:
 *       {
 *          index:int
 *          lateSec:float
 *          isMiss:boolean
 *          gamePoint:float
 *       },
 *          ...,
 *
 *       noten:
 *       {
 *         ...
 *       }
 *     ]
 *   },
 *    ...,
 *   trackn:
 *   {
 *     ...
 *   }
 *  ]
 * }
 *
 * */
public class PlayHistory {

    protected Editor editor;


    /**已弹奏音符数量*/
    protected int playNoteCount = 0;
    /**弹奏错误音符数量*/
    protected int errorNoteCount = 0;
    /**错过弹奏的音符数量*/
    protected int missNoteCount = 0;

    /**游戏获取点数*/
    protected int gamePoint = 0;
    /**总游戏点数*/
    protected int totalGamePoint = 0;
    /**总音符数量*/
    protected int totalNoteCount = 0;
    /**得分*/
    protected int score = 0;

    public PlayHistory(Editor editor){
        this.editor = editor;
    }


    /**从文件中加载历史记录*/
    public void LoadFromFile(String filePath)
    {
        try {
            String jsonStr = FileUtils.getInstance().ReadFileToString(filePath);
            JSONObject jsonHistory = new JSONObject(jsonStr);

            //accounts(结算信息)
            JSONObject jsonAccounts = jsonHistory.getJSONObject("accounts");
            playNoteCount = jsonAccounts.getInt("playNoteCount");
            errorNoteCount = jsonAccounts.getInt("errorNoteCount");
            missNoteCount = jsonAccounts.getInt("missNoteCount");
            gamePoint = jsonAccounts.getInt("gamePoint");
            totalGamePoint = jsonAccounts.getInt("totalGamePoint");
            totalNoteCount = jsonAccounts.getInt("totalNoteCount");
            score = jsonAccounts.getInt("score");

            //tracks
            JSONArray jsonTracks = jsonHistory.getJSONArray("tracks");
            int trackIdx, noteIdx;
            JSONObject jsonTrack, jsonNote;
            JSONArray jsonNotes;
            float noteLateSec;
            NoteOnEvent[] noteOnEvents;
            Track[] tracks = editor.tracks;

            for(int i=0; i<jsonTracks.length(); i++)
            {
                jsonTrack = jsonTracks.getJSONObject(i);
                //
                trackIdx = jsonTrack.getInt("index");
                jsonNotes = jsonTrack.getJSONArray("notes");

                noteOnEvents = tracks[trackIdx].noteOnEvents;
                for(int j=0; j<jsonNotes.length(); j++)
                {
                    jsonNote = jsonNotes.getJSONObject(j);
                    noteIdx = jsonNote.getInt("index");
                    noteLateSec = (float)jsonNote.getDouble("lateSec");
                    noteOnEvents[noteIdx].recordLateSec = noteLateSec;
                    noteOnEvents[noteIdx].recordIsMiss = jsonNote.getBoolean("isMiss");
                }
            }

        }catch (JSONException e) {
            e.printStackTrace();
        }
    }


    public void SaveToFile(String filePath)
    {
        try {

            JSONObject jsonPlayHistory = new JSONObject();

            //accounts(结算信息)
            JSONObject jsonAccounts = new JSONObject();
            jsonAccounts.put("playNoteCount", editor.GetPlayedNoteCount());
            jsonAccounts.put("errorNoteCount", editor.GetPlayErrorNoteCount());
            jsonAccounts.put("missNoteCount", editor.GetMissedNoteCount());
            jsonAccounts.put("gamePoint", (int)editor.GetPlayGamePoint());
            jsonAccounts.put("totalGamePoint", (int)editor.GetTotalGamePoint());
            jsonAccounts.put("totalNoteCount", editor.GetNeedPlayNoteCount());
            jsonAccounts.put("score", editor.GetScore());
            jsonPlayHistory.put("accounts", jsonAccounts);

            //tracks
            JSONArray jsonTracks = null;
            JSONObject jsonTrack, jsonNote;
            JSONArray jsonNotes;

            Track[] tracks = editor.tracks;
            Track track;
            NoteOnEvent ev;
            for (int i = 0; i < tracks.length; i++)
            {
                jsonTrack = null;
                jsonNotes = null;

                track = tracks[i];
                if (!editor.IsPointerPlayTrack(track))
                    continue;

                for (int j = 0; j < track.noteOnEvents.length; j++) {
                    ev = track.noteOnEvents[j];
                    if (!editor.IsPointerPlayNote(ev))
                        continue;

                    jsonNote = new JSONObject();
                    jsonNote.put("index", j);
                    jsonNote.put("lateSec", ev.lateSec);
                    jsonNote.put("isMiss", ev.isMiss);
                    jsonNote.put("gamePoint", ev.gamePoint);

                    //
                    if (jsonNotes == null) {
                        jsonNotes = new JSONArray();

                        if (jsonTrack == null) {
                            jsonTrack = new JSONObject();
                            jsonTrack.put("index", i);
                        }

                        jsonTrack.put("notes", jsonNotes);
                    }

                    jsonNotes.put(jsonNote);
                }

                if (jsonTrack != null && jsonTracks == null)
                    jsonTracks = new JSONArray();

                jsonTracks.put(jsonTrack);
            }

            //
            if(jsonTracks != null)
            {
                jsonPlayHistory.put("tracks", jsonTracks);
            }

            //
            String jsonStr = jsonPlayHistory.toString();
            InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
            FileUtils.getInstance().WriteToFile(filePath, jsonStream);

        }catch (JSONException e) {
            e.printStackTrace();
        }
    }
}
