package cymheart.tau.filemgr;

import android.content.Context;

import org.json.JSONException;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import cymheart.tau.editor.Track;
import cymheart.tau.utils.FileUtils;

public class FileMgr {

    /**用户数据根路径*/
    protected String userDatasRootPath;

    /**用户名称*/
    protected String user;

    /**获取用户数据根目录*/
    public String GetUserRootDir(String userName){
        return userDatasRootPath + userName;
    }

    /**获取用户数据midi根目录*/
    public String GetUserMidiRootDir(String userName){
        return GetUserRootDir(userName) + "/Midi";
    }

    /**获取midi文件相关的用户目录*/
    public String GetUserMidiFileDir(String midiFilePath, String userName){
        if(userName == null || userName.isEmpty())
            userName = "global";

        int end = midiFilePath.lastIndexOf('.');
        String midiDirPath = midiFilePath.substring(0, end);
        return GetUserMidiRootDir(userName) + midiDirPath;
    }

    /**获取midi目录相关的用户目录*/
    public String GetUserMidiDirDir(String midiDirPath, String userName){
        if(userName == null || userName.isEmpty())
            userName = "global";

        return GetUserMidiRootDir(userName) + midiDirPath;
    }

    protected VersionMgr versionMgr;

    public FileMgr(Context context){
        userDatasRootPath = FileUtils.getInstance().GetExtralFilePath(context) + "/UserDatas/";
    }



}














