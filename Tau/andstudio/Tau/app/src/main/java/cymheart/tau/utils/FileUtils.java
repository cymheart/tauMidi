package cymheart.tau.utils;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;


/**
 * <p>文件工具类<p>
 */
public class FileUtils
{
    private static class SingletonClassInstance{
        private static final FileUtils instance = new FileUtils();
    }

    public static FileUtils getInstance(){
        return SingletonClassInstance.instance;
    }

    /**
     * 获取sd卡外部存储文件目录
     该目录下的文件数据是属于外部存储卡存储当前应用的文件目录  该目录需要申请权限
     * 对应的目录  /sdCard/Androida/data/applicaion包名/files
     * 对于外部存储的内置TF卡，应用一旦被卸载，对应sdCard中该目录的文件将全部被清除
     * 对于外部存储的外置TF卡，应用一旦被卸载，数据任然不会被删除
     */
    public String GetExtralFilePath(Context context){
        File extralFile = context.getExternalFilesDir(null);
        return extralFile.getAbsolutePath();
    }

    /**创建文件*/
    public File CreateFile(String path) {
        int end = path.lastIndexOf('/');
        String p = path.substring(0, end+1);
        String n = path.substring(end+1);
        return CreateFile(p, n);
    }

    /**创建文件*/
    public File CreateFile(String path, String fileName){

        File file;
        try {
            CreateDir(path);
            file = new File(path + fileName);
            file.createNewFile();
        } catch (Exception e) {
            return null;
        }

        return file;
    }

    /**创建目录*/
    public File CreateDir(String path){
        File dir = new File(path);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        return dir;
    }

    /**判断SD卡上的文件是否存在*/
    public boolean IsFileExist(String path){
        try{
            File f = new File(path);
            return f.exists();
        }
        catch (Exception e) {
            return false;
        }
    }

    /**将一个InputStream里面的数据写入到指定路径文件中
     * 将stream写到path/这个目录中的fileName文件上*/
    public File WriteToFile(String path, InputStream stream)
    {
        int end = path.lastIndexOf('/');
        String p = path.substring(0, end + 1);
        String n = path.substring(end + 1);
        return WriteToFile(p, n, stream);
    }

    /**将一个InputStream里面的数据写入到指定路径文件中
     * 将stream写到path/这个目录中的fileName文件上*/
    public File WriteToFile(String path, String fileName, InputStream stream)
    {
        File file = null;
        OutputStream output = null;
        try{
            file = CreateFile(path, fileName);
            //FileInputStream是读取数据，FileOutputStream是写入数据，写入到file这个文件上去
            output = new FileOutputStream(file);
            byte[] bytes = new byte[1024];
            int i;
            while ((i = stream.read(bytes)) != -1) {
                output.write(bytes, 0, i);
            }

            output.flush();
        }
        catch(Exception e){
            e.printStackTrace();
        }
        finally{
            try{
                output.close();
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
        return file;
    }

    /**将一个Byte[]里面的数据写入到指定路径文件中
     * 将bytes写到path的文件上*/
    public File WriteToFile(String path, byte[] bytes)
    {
        int end = path.lastIndexOf('/');
        String p = path.substring(0, end);
        String n = path.substring(end+1);
        return WriteToFile(p,n, bytes);
    }

    /**将一个Byte[]里面的数据写入到指定路径文件中
     * 将bytes写到path/这个目录中的fileName文件上*/
    public File WriteToFile(String path, String fileName, byte[] bytes)
    {
        InputStream stream = new ByteArrayInputStream(bytes);
        return WriteToFile(path, fileName, stream);
    }

    //读取文件到字节流
    public byte[] ReadFileToBytes(String filePath)
    {
        byte[] bytes = null;

        try {
            FileInputStream fis = new FileInputStream(filePath);
            //读取本地小文件
            bytes = new byte[fis.available()];
            fis.read(bytes);
            fis.close();
        }catch (Exception e) {
            e.printStackTrace();
        }

        return bytes;
    }

    //读取文件到String
    public String ReadFileToString(String filePath)  {
        return ReadFileToString(filePath, "utf-8");
    }

    //读取文件到String
    public String ReadFileToString(String filePath, String charsetName)  {
        byte[] bytes = ReadFileToBytes(filePath);
        String str = null;
        try {
            str = new String(bytes, charsetName);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return str;
    }

    //读取文件到字节流
    public byte[] ReadFileToBytes(File file)
    {
        byte[] bytes = null;

        try {
            FileInputStream fis = new FileInputStream(file);
            //读取本地小文件
            bytes = new byte[fis.available()];
            fis.read(bytes);
            fis.close();
        }catch (Exception e) {
            e.printStackTrace();
        }

        return bytes;
    }

    //读取文件到String
    public String ReadFileToString(File file)  {
        return ReadFileToString(file, "utf-8");
    }

    //读取文件到String
    public String ReadFileToString(File file, String charsetName)  {
        byte[] bytes = ReadFileToBytes(file);
        String str= null;
        try {
            str = new String(bytes, charsetName);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return str;
    }

    /**
     * 普通的文件复制方法
     *
     * @param fromFile 源文件
     * @param toFile   目标文件
     */
    public void FileCopyNormal(File fromFile, File toFile) {
        InputStream inputStream = null;
        OutputStream outputStream = null;
        try {
            inputStream = new BufferedInputStream(new FileInputStream(fromFile));
            outputStream = new BufferedOutputStream(new FileOutputStream(toFile));
            byte[] bytes = new byte[1024];
            int i;
            //读取到输入流数据，然后写入到输出流中去，实现复制
            while ((i = inputStream.read(bytes)) != -1) {
                outputStream.write(bytes, 0, i);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (inputStream != null)
                    inputStream.close();
                if (outputStream != null)
                    outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 用filechannel进行文件复制
     *
     * @param fromFile 源文件
     * @param toFile   目标文件
     */
    public void FileCopyWithFileChannel(File fromFile, File toFile) {
        FileInputStream fileInputStream = null;
        FileOutputStream fileOutputStream = null;
        FileChannel fileChannelInput = null;
        FileChannel fileChannelOutput = null;
        try {
            fileInputStream = new FileInputStream(fromFile);
            fileOutputStream = new FileOutputStream(toFile);
            //得到fileInputStream的文件通道
            fileChannelInput = fileInputStream.getChannel();
            //得到fileOutputStream的文件通道
            fileChannelOutput = fileOutputStream.getChannel();
            //将fileChannelInput通道的数据，写入到fileChannelOutput通道
            fileChannelInput.transferTo(0, fileChannelInput.size(), fileChannelOutput);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (fileInputStream != null)
                    fileInputStream.close();
                if (fileChannelInput != null)
                    fileChannelInput.close();
                if (fileOutputStream != null)
                    fileOutputStream.close();
                if (fileChannelOutput != null)
                    fileChannelOutput.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }




    /** 获取文件扩展名*/
    public String GetExtensionName(String filename) {
        if ((filename != null) && (filename.length() > 0)) {
            int dot = filename.lastIndexOf('.');
            if ((dot >-1) && (dot < (filename.length() - 1))) {
                return filename.substring(dot + 1);
            }
        }
        return filename;
    }

    /** 获取不带扩展名的文件名*/
    public  String GetFileNameNoExtension(String filename) {
        if ((filename != null) && (filename.length() > 0)) {
            int dot = filename.lastIndexOf('.');
            if ((dot >-1) && (dot < (filename.length()))) {
                return filename.substring(0, dot);
            }
        }
        return filename;
    }



    /***
     * 获取文件的时间,辅助定期删除录像信息
     * **/
    @TargetApi(Build.VERSION_CODES.O)
    public Long GetFileCreateTime(String filePath) {
        File file = new File(filePath);
        try {
            Path path = Paths.get(filePath);

            BasicFileAttributeView basicView =
                    Files.getFileAttributeView(path,
                            BasicFileAttributeView.class, LinkOption.NOFOLLOW_LINKS);

            BasicFileAttributes attr = basicView.readAttributes();
            return attr.creationTime().toMillis();
        } catch (Exception e) {
            e.printStackTrace();
            return file.lastModified();
        }
    }

}
