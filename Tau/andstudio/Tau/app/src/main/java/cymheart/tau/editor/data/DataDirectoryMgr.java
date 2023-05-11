package cymheart.tau.editor.data;

import android.content.Context;

import cymheart.tau.filemgr.VersionMgr;
import cymheart.tau.utils.FileUtils;
/**数据目录管理*/
public class DataDirectoryMgr {

    /**用户数据根路径*/
    protected String userDatasRootPath;

    /**用户名称*/
    protected String user;

    /**获取用户数据目录 <P>
     *  ../UserDatas/userName/
     * */
    public String GetUserDataDir(String userName){
        if(userName == null || userName.isEmpty())
            userName = "global";

        return userDatasRootPath + userName + "/";
    }

    /**获取用户数据midi目录 <P>
     * ../UserDatas/userName/Midi/
     * */
    public String GetUserMidiDir(String userName){
        return GetUserDataDir(userName) + "/Midi/";
    }

    /**获取用户数据NativeMidi目录 <P>
     * ../UserDatas/userName/NativeMidi/
     * */
    public String GetUserNativeMidiDir(String userName){
        return GetUserDataDir(userName) + "/NativeMidi/";
    }

    /**获取Midi文件相关的用户目录 <P>
     * ../UserDatas/userName/Midi/midiName/
     * */
    public String GetMidiFileDir(String midiName, String userName){
        return GetUserMidiDir(userName) + midiName + "/";
    }


    /**获取NativeMidi文件相关的用户目录 <P>
     * ../UserDatas/userName/NativeMidi/midiFilePath/
     * */
    public String GetNativeMidiFileDir(String midiFilePath, String userName){
        int end = midiFilePath.lastIndexOf('.');
        String midiDirPath = midiFilePath.substring(0, end);
        return GetUserNativeMidiDir(userName) + midiDirPath;
    }

    /**获取NativeMidi文件目录相关的用户目录 <P>
     * ../UserDatas/userName/NativeMidi/midiDirPath/
     * */
    public String GetNativeMidiDirDir(String midiDirPath, String userName){
        return GetUserNativeMidiDir(userName) + midiDirPath;
    }

    public DataDirectoryMgr(Context context){
        userDatasRootPath = FileUtils.getInstance().GetExtralFilePath(context) + "/UserDatas/";
    }

}














