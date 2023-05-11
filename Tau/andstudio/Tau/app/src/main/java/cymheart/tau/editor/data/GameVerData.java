package cymheart.tau.editor.data;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import cymheart.tau.editor.MidiMarker;
import cymheart.tau.editor.Track;
import cymheart.tau.utils.FileUtils;

/**
 * 游戏版本数据
 * ----JsonGameVerData 结构----
 *{
 *
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
 *      noteOnEvents:
 *      [
 *          noteOnEvent0:
 *          {
 *             index:int,
 *
 *             fingerIdxs:
 *             [
 *                fingerIdx0:
 *                {
 *                    hand : int
 *                    finger : int
 *                },
 *                    ...,
 *                fingerIdxN:
 *                {
 *                    ...
 *                },
 *             ]
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
 * */
public class GameVerData {

    protected String gameFilePath;
    protected JSONObject jsonVerData;

    protected JSONObject jsonLoop = null;
    protected JSONObject jsonMarkerSetting = null;
    protected JSONArray jsonMyMarkers = null;
    /**tracks的json*/
    protected JSONArray jsonTracks = null;

    //
    public double loopStartSec = -1;
    public double loopEndSec = -1;
    public double loopDelaySec = 0;
    public int loopErrorCount  = 0;
    //
    public boolean isEnableMyMarker = true;
    public boolean isEnableKeySign = true;
    public boolean isEnableMidiMarker = true;
    //
    public List<MidiMarker> myMarkers = new ArrayList<>();

    public Track[] tracks;

    /**保存verdata文件*/
    public void SaveToFile()
    {
        if(jsonVerData == null)
            return;

        String jsonStr = jsonVerData.toString();
        InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
        FileUtils.getInstance().WriteToFile(gameFilePath, jsonStream);
    }


    public GameVerData(String filePath){
        gameFilePath = filePath;
    }


    protected void CreateJson() throws JSONException
    {
        jsonVerData = new JSONObject();

        //
        jsonLoop = new JSONObject();
        jsonLoop.put("startSec", loopStartSec);
        jsonLoop.put("endSec", loopEndSec);
        jsonLoop.put("delaySec", loopDelaySec);
        jsonLoop.put("errorCount", loopErrorCount);
        jsonVerData.put("loop", jsonLoop);

        //
        jsonMarkerSetting = new JSONObject();
        jsonMarkerSetting.put("myMarker", isEnableMyMarker);
        jsonMarkerSetting.put("keySign", isEnableKeySign);
        jsonMarkerSetting.put("midiMarker", isEnableMidiMarker);
        jsonVerData.put("markerSetting", jsonLoop);

        //
        jsonMyMarkers = new JSONArray();
        jsonVerData.put("myMarkers", jsonMyMarkers);


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


    public void Parse()
    {
        try {
            String jsonStr = null;
            if(FileUtils.getInstance().IsFileExist(gameFilePath))
                jsonStr = FileUtils.getInstance().ReadFileToString(gameFilePath);

            if (jsonStr == null) {
                CreateJson();
                return;
            }

            //
            jsonVerData = new JSONObject(jsonStr);
            jsonTracks = jsonVerData.getJSONArray("tracks");

            //
            jsonLoop = jsonVerData.getJSONObject("loop");
            jsonMarkerSetting = jsonVerData.getJSONObject("markerSetting");
            jsonMyMarkers = jsonVerData.getJSONArray("myMarkers");


            //loop
            if (jsonLoop != null && jsonLoop.length() != 0) {

                if (jsonLoop.has("startSec"))
                    loopStartSec = jsonLoop.getDouble("startSec");
                if (jsonLoop.has("endSec"))
                    loopEndSec = jsonLoop.getDouble("endSec");
                if (jsonLoop.has("delaySec"))
                    loopDelaySec = jsonLoop.getDouble("delaySec");
                if (jsonLoop.has("errorCount"))
                    loopErrorCount = jsonLoop.getInt("errorCount");
            }

            //markerSetting
            if (jsonMarkerSetting != null && jsonMarkerSetting.length() != 0) {
                if (jsonMarkerSetting.has("myMarker"))
                    isEnableMyMarker = jsonMarkerSetting.getBoolean("myMarker");
                if (jsonMarkerSetting.has("keySign"))
                    isEnableKeySign = jsonMarkerSetting.getBoolean("keySign");
                if (jsonMarkerSetting.has("midiMarker"))
                    isEnableMidiMarker = jsonMarkerSetting.getBoolean("midiMarker");
            }

            //mymarkers
            MidiMarker myMarker;
            double sec = 0;
            for (int i = 0; i < jsonMyMarkers.length(); i++) {
                JSONObject jsonMyMarker = jsonMyMarkers.getJSONObject(i);
                sec = jsonMyMarker.getDouble("sec");

                myMarker = new MidiMarker();
                myMarker.isEnableMarkerText = true;
                myMarker.startSec = sec;
                myMarkers.add(myMarker);
            }

            Collections.sort(myMarkers, (left, right) -> left.startSec < right.startSec ? -1 : 1);

            ParseToTracks();

        }catch (Exception ignored){

        }
    }

    protected void ParseToTracks() throws JSONException
    {
        if(tracks == null)
            return;

        Track track;
        JSONArray jsonNoteOnEvents;
        JSONObject jsonMidiEvent;
        JSONObject jsonTrack;
        JSONArray jsonFingerIdxs;
        JSONObject jsonFingerIdx;

        for (int i = 0; i < tracks.length; i++) {
            track = tracks[i];
            jsonTrack = jsonTracks.getJSONObject(i);

            jsonNoteOnEvents = jsonTrack.getJSONArray("noteOnEvents");
            //
            int midiIndex;
            for (int m = 0; m < jsonNoteOnEvents.length(); m++) {
                jsonMidiEvent = jsonNoteOnEvents.getJSONObject(m);
                midiIndex = jsonMidiEvent.getInt("index");
                track.noteOnEvents[midiIndex].ClearHandFingers();
                jsonFingerIdxs = jsonMidiEvent.getJSONArray("fingerIdxs");
                for(int j=0; j < jsonFingerIdxs.length(); j++)
                {
                    jsonFingerIdx = jsonFingerIdxs.getJSONObject(j);
                    int hd = jsonFingerIdx.getInt("hand");
                    int fig = jsonFingerIdx.getInt("finger");
                    track.noteOnEvents[midiIndex].AddHandFingerLabel(hd, fig);
                }

            }
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
        Collections.sort(myMarkers,  (left, right) -> left.startSec < right.startSec ? -1: 1);
    }


    /**添加note的手指标签*/
    public void AddNoteHandFinger(int trackIdx, int noteIdx, int hand, int finger)
    {
        try {
            if(tracks[trackIdx].noteOnEvents[noteIdx].HavHandFingerLabel(hand, finger))
                return;

            tracks[trackIdx].noteOnEvents[noteIdx].AddHandFingerLabel(hand, finger);
            //
            JSONObject jsonTrack = jsonTracks.getJSONObject(trackIdx);
            JSONArray jsonNoteOnEvents = jsonTrack.getJSONArray("noteOnEvents");
            JSONObject jsonNote, jsonFingerIdx;
            JSONArray jsonFingerIdxs;
            int idx = -1;

            for (int i = 0; i < jsonNoteOnEvents.length(); i++) {
                jsonNote = jsonNoteOnEvents.getJSONObject(i);
                idx = jsonNote.getInt("index");
                if(idx == noteIdx) {
                    jsonFingerIdxs = jsonNote.getJSONArray("fingerIdxs");
                    jsonFingerIdx = new JSONObject();
                    jsonFingerIdx.put("hand", hand);
                    jsonFingerIdx.put("finger", finger);
                    jsonFingerIdxs.put(jsonFingerIdx);
                    break;
                }
            }

            if(idx != noteIdx){
                jsonNote = new JSONObject();
                jsonNote.put("index", noteIdx);
                jsonFingerIdxs = new JSONArray();
                jsonNote.put("fingerIdxs", jsonFingerIdxs);
                jsonFingerIdx = new JSONObject();
                jsonFingerIdx.put("hand", hand);
                jsonFingerIdx.put("finger", finger);
                jsonFingerIdxs.put(jsonFingerIdx);
                jsonNoteOnEvents.put(jsonNote);
            }


        } catch (JSONException e) {
            e.printStackTrace();
        }
    }


    /**移除note的手指标签*/
    public void RemoveNoteHandFinger(int trackIdx, int noteIdx, int hand, int finger)
    {
        try {
            if(!tracks[trackIdx].noteOnEvents[noteIdx].HavHandFingerLabel(hand, finger))
                return;

            tracks[trackIdx].noteOnEvents[noteIdx].RemoveHandFingerLabel(hand, finger);
            //
            JSONObject jsonTrack = jsonTracks.getJSONObject(trackIdx);
            JSONArray jsonNoteOnEvents = jsonTrack.getJSONArray("noteOnEvents");
            JSONObject jsonNote, jsonFingerIdx;
            JSONArray jsonFingerIdxs;
            int idx;

            for (int i = 0; i < jsonNoteOnEvents.length(); i++) {
                jsonNote = jsonNoteOnEvents.getJSONObject(i);
                idx = jsonNote.getInt("index");
                if(idx == noteIdx) {
                    jsonFingerIdxs = jsonNote.getJSONArray("fingerIdxs");

                    for(int j=0; j < jsonFingerIdxs.length(); j++)
                    {
                        jsonFingerIdx = jsonFingerIdxs.getJSONObject(j);
                        int hd = jsonFingerIdx.getInt("hand");
                        int fig = jsonFingerIdx.getInt("finger");
                        if(hd == hand && fig == finger){
                            jsonFingerIdxs.remove(j);
                            return;
                        }
                    }
                }
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }
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




}
