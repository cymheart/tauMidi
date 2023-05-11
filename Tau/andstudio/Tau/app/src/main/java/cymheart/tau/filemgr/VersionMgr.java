package cymheart.tau.filemgr;

import org.json.JSONException;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import cymheart.tau.editor.PlayHistory;
import cymheart.tau.editor.PlayRecorder;
import cymheart.tau.editor.Track;


/**版本管理*/
public class VersionMgr {
    protected FileMgr fileMgr;


    /**获取midi文件的所有版本名称列表*/
    public List<String> GetVersionNameList(String midiFilePath, String userName){
        String midiDirPath = fileMgr.GetUserMidiFileDir(midiFilePath, userName);

        List<String> vers = new ArrayList<>();
        File file = new File(midiDirPath);
        File[] mfiles = file.listFiles();
        if(mfiles == null)
            return null;

        for (File mfile : mfiles) {
            if (!mfile.isDirectory())
                continue;
            vers.add(mfile.getName());
        }
        return vers;
    }


    /**获取指定midi文件的版本文件路径*/
    public String GetVersionFilePath(String midiFilePath, String verName, String userName){
        String midiDirPath = fileMgr.GetUserMidiFileDir(midiFilePath, userName);
        return midiDirPath +"/" + verName + "/" + "version_data";
    }


    /**载入指定版本文件数据*/
    public VersionData LoadVersionData(String verFilePath, Track[] tracks){

        VersionData versionData = new VersionData(verFilePath);
        try {
            versionData.Parse();
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return versionData;
    }


    /**获取指定midi文件的游戏文件路径*/
    public String GetGameFilePath(String midiFilePath, String verName, String userName){
        String midiDirPath = fileMgr.GetUserMidiFileDir(midiFilePath, userName);
        return midiDirPath +"/" + verName + "/" + "game_data";
    }


    /**载入指定游戏文件数据*/
    public GameData LoadGameData(String gameFilePath, Track[] tracks){

        GameData gameData = new GameData(gameFilePath);
        try {
            gameData.Parse();
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return gameData;
    }


    /**获取指定midi文件的版本历史记录文件路径*/
    public String GetHistoryFilePath(String midiFilePath, String verName, int gameMode, int handType, String userName){
        String midiDirPath = fileMgr.GetUserMidiFileDir(midiFilePath, userName);
        return midiDirPath +"/" + verName + "/History/" + gameMode + "/" + handType + "/history_data";
    }


    /**载入指定历史记录文件数据*/
    public HistoryData LoadHistoryData(String hisFilePath){

        HistoryData historyData = new HistoryData(hisFilePath);
        try {
            historyData.Parse();
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return historyData;

    }

    /**获取指定midi文件的弹奏录制目录路径*/
    public String GetPlayRecorderDirPath(String midiFilePath, String verName, int gameMode, int handType, String userName){
        String midiDirPath = fileMgr.GetUserMidiFileDir(midiFilePath, userName);
        return midiDirPath +"/" + verName + "/History/" + gameMode + "/" + handType;
    }

    /**保存弹奏录制文件*/
    public void SavePlayRecorderFile(PlayRecorder playRecorder, String dirPath, String recorderName)
    {
        playRecorder.SaveToFile(dirPath + "/" + recorderName + ".playrec");
    }

    /**加载弹奏录制文件*/
    public void LoadPlayRecorderFile(String recorderFilePath, PlayRecorder outPlayRecorder)
    {
        outPlayRecorder.LoadFromFile(recorderFilePath);
    }


    /**获取指定midi文件的弹奏历史目录路径*/
    public String GetPlayHistoryDirPath(String midiFilePath, String verName, int gameMode, int handType, String userName){
        String midiDirPath = fileMgr.GetUserMidiFileDir(midiFilePath, userName);
        return midiDirPath +"/" + verName + "/History/" + gameMode + "/" + handType;
    }

    /**保存弹奏历史文件*/
    public void SavePlayHistoryFile(PlayHistory playHistory, String dirPath, String historyName)
    {
        playHistory.SaveToFile(dirPath + "/" + historyName + ".playHis");
    }

    /**加载弹奏历史文件*/
    public void LoadPlayHistoryFile(String historyFilePath, PlayRecorder outPlayHistory)
    {
        outPlayHistory.LoadFromFile(historyFilePath);
    }

}
