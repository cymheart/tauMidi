package cymheart.tau.filemgr;

/**历史记录数据*/

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import cymheart.tau.utils.FileUtils;

/**
 * ----JsonHistoryData 结构----
 *{
 *  ver_name: string,
 *  game_mode: int,
 *  hand_type: int,
 *
 *  items:
 * [
 *   item0:
 *   {
 *      points: int
 *      play_note_count: int
 *      play_error_count: int
 *      score: float
 *      record_name: string
 *      date: string
 *      diff: int
 *      key_type: int
 *   },
 *       ...,
 *   itemn:
 *   {
 *     ...
 *   }
 * ]
 *
 *}
 */
public class HistoryData {

    protected JSONObject jsonHisData;
    protected JSONArray jsonItems;
    protected String historyFilePath;

    public  String ver_name;
    public int game_mode;
    public int hand_type;
    public List<HistoryItem> historyItems = new ArrayList<>();

    /**保存hisdata文件*/
    public void SaveToFile()
    {
        if(jsonHisData == null)
            return;

        String jsonStr = jsonHisData.toString();
        InputStream jsonStream = new ByteArrayInputStream(jsonStr.getBytes(StandardCharsets.UTF_8));
        FileUtils.getInstance().WriteToFile(historyFilePath, jsonStream);
    }

    public HistoryData(String filePath){
        historyFilePath = filePath;
    }

    protected void CreateJson() throws JSONException
    {
        jsonHisData = new JSONObject();

        jsonHisData.put("ver_name", "");
        jsonHisData.put("game_mode", 0);
        jsonHisData.put("hand_type", 0);

        jsonItems = new JSONArray();
        jsonHisData.put("items", jsonItems);
    }



    public void Parse() throws JSONException {
        String jsonStr = FileUtils.getInstance().ReadFileToString(historyFilePath);
        if (jsonStr == null) {
            CreateJson();
            return;
        }

        //
        jsonHisData = new JSONObject(jsonStr);
        ver_name = jsonHisData.getString("ver_name");
        game_mode = jsonHisData.getInt("game_mode");
        hand_type = jsonHisData.getInt("hand_type");
        //
        jsonItems = jsonHisData.getJSONArray("items");

        HistoryItem item;
        for(int i = 0;i < jsonItems.length();i++)
        {
            item = new HistoryItem();
            //
            JSONObject jsonItem = jsonItems.getJSONObject(i);
            item.points = jsonItem.getInt("points");
            item.play_note_count = jsonItem.getInt("play_note_count");
            item.play_error_count = jsonItem.getInt("play_error_count");
            item.score = jsonItem.getInt("score");
            item.record_name = jsonItem.getString("record_name");
            item.date = jsonItem.getString("date");
            item.diff = jsonItem.getInt("diff");
            item.key_type = jsonItem.getInt("key_type");

            //
            historyItems.add(item);
        }
    }


    public void AddHistory(HistoryItem historyItem){
        historyItems.add(historyItem);
        try {
            JSONObject jsonItem = new JSONObject();
            jsonItem.put("points", historyItem.points);
            jsonItem.put("play_note_count", historyItem.play_note_count);
            jsonItem.put("play_error_count", historyItem.play_error_count);
            jsonItem.put("score", historyItem.score);
            jsonItem.put("record_name", historyItem.record_name);
            jsonItem.put("date", historyItem.date);
            jsonItem.put("diff", historyItem.diff);
            jsonItem.put("key_type", historyItem.key_type);
            jsonItems.put(jsonItem);

        }catch (JSONException e) {
            e.printStackTrace();
        }
    }




}
