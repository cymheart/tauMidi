package cymheart.tau.filemgr;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

import cymheart.tau.editor.Track;
import cymheart.tau.utils.FileUtils;

/**版本数据*/

/**
 * ----JsonVerData 结构----
 *{
 *
 *  play_diff: int,
 *
 *  tracks:
 * [
 *   track0:
 *   {
 *      index:int,
 *      instrument: int,
 *      NoteColor: int,
 *      PlayType: int,
 *
 *      noteOnEvents:
 *      [
 *          noteOnEvent0:
 *          {
 *             index:int,
 *             PlayType: int
 *           },
 *                ...,
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
public class VersionData {

    protected String verFilePath;
    protected JSONObject jsonVerData;
    /**tracks的json*/
    protected JSONArray jsonTracks = null;

    /**弹奏难度*/
    public int playDiff = 0;

    public Track[] tracks;

    /**保存verdata文件*/
    public void SaveToFile()
    {
        if(jsonVerData == null)
            return;

        String jsonStr = jsonVerData.toString();
        InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
        FileUtils.getInstance().WriteToFile(verFilePath, jsonStream);
    }


    public VersionData(String filePath){
        verFilePath = filePath;
    }


    protected void CreateJson() throws JSONException
    {
        jsonVerData = new JSONObject();

        jsonVerData.put("play_diff", playDiff);
        //
        jsonTracks = new JSONArray();
        jsonVerData.put("tracks", jsonTracks);

        //
        if(tracks != null) {
            JSONArray jsonNoteOnEvents;
            for (int i = 0; i < tracks.length; i++) {
                JSONObject jsonTrack = new JSONObject();
                jsonTracks.put(jsonTrack);
                //
                jsonNoteOnEvents = new JSONArray();
                jsonTrack.put("noteOnEvents", jsonNoteOnEvents);
            }
        }
    }


    public void Parse() throws JSONException
    {
        String jsonStr = FileUtils.getInstance().ReadFileToString(verFilePath);
        if(jsonStr == null){
            CreateJson();
            return;
        }

        //
        jsonVerData = new JSONObject(jsonStr);
        jsonTracks = jsonVerData.getJSONArray("tracks");

        //
        if(jsonVerData.opt("play_diff") != null)
            playDiff = jsonVerData.getInt("play_diff");

        //
        ParseToTracks();
    }

    protected void ParseToTracks() throws JSONException
    {
        if(tracks == null)
            return;

        Track track;
        JSONArray jsonNoteOnEvents;
        JSONObject jsonMidiEvent;
        JSONObject jsonTrack;
        int trackIndex;
        int midiIndex;
        int playType;

        for(int i=0; i<jsonTracks.length(); i++)
        {
            jsonTrack = jsonTracks.getJSONObject(i);
            trackIndex = jsonTrack.getInt("index");
            track = tracks[trackIndex];
            track.SetPlayType(jsonTrack.getInt("PlayType"));
            track.SetNoteColor(jsonTrack.getInt("NoteColor"));
            //
            jsonNoteOnEvents = jsonTrack.getJSONArray("noteOnEvents");
            for (int m = 0; m < jsonNoteOnEvents.length(); m++) {
                jsonMidiEvent = jsonNoteOnEvents.getJSONObject(m);
                midiIndex = jsonMidiEvent.getInt("index");
                track.noteOnEvents[midiIndex].ClearHandFingers();
                playType = jsonMidiEvent.getInt("PlayType");
                track.noteOnEvents[midiIndex].SetPlayType(playType);
            }
        }

    }



    public void SetTrackPlayType(int trackIdx, int playType){
        try {
            tracks[trackIdx].SetPlayType(playType);

            //
            int idx;
            JSONObject jsonTrack;
            for (int i = 0; i < jsonTracks.length(); i++) {
                jsonTrack = jsonTracks.getJSONObject(trackIdx);
                idx = jsonTrack.getInt("index");
                if(idx == trackIdx) {
                    jsonTrack.put("PlayType", playType);
                    break;
                }
            }

        }catch (JSONException e) {
            e.printStackTrace();
        }

    }

    public void SetTrackNoteColor(int trackIdx, int noteColor){
        try {
            tracks[trackIdx].SetNoteColor(noteColor);

            //
            int idx;
            JSONObject jsonTrack;
            for (int i = 0; i < jsonTracks.length(); i++) {
                jsonTrack = jsonTracks.getJSONObject(trackIdx);
                idx = jsonTrack.getInt("index");
                if(idx == trackIdx) {
                    jsonTrack.put("NoteColor", noteColor);
                    break;
                }
            }
        }catch (JSONException e) {
            e.printStackTrace();
        }
    }






}
