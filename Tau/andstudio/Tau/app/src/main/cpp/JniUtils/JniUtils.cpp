
#include"JniUtils.h"

std::string jstring2str(JNIEnv* env, jstring jstr)
{
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("UTF8");
    jmethodID mid = env->GetMethodID(clsstring,   "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr,JNI_FALSE);
    if(alen > 0)
    {
        rtn = (char*)malloc(alen+1);
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }

    env->ReleaseByteArrayElements(barr,ba,0);

    if(rtn != NULL) {
        std::string stemp(rtn);
        free(rtn);
        return stemp;
    }

    return "";
}

jstring str2jstring(JNIEnv* env,const char* pat)
{
    char* encodingStr = "UTF8";
    if(!is_str_utf8(pat)) {
        if (is_str_gbk(pat))
            encodingStr = "GBK";
    }

    //定义java String类 strClass
    jclass strClass = (env)->FindClass("java/lang/String");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF(encodingStr);

   // jstring encoding2 = (env)->NewStringUTF(reinterpret_cast<const char *>(bytes));

    //将byte数组转换为java String,并输出
    return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
}


int SetStringField(JNIEnv *jenv, jobject obj, const char* field, jstring str)
{
    jclass cls;
    jfieldID f;
    if(!jenv || !field || !obj || !str)
        return -1;
    cls = jenv->GetObjectClass(obj);
    f = jenv->GetFieldID(cls,field,"Ljava/lang/String;");
    if(!f)
        return -1;
    jenv->SetObjectField(obj,f,str);
    return 0;
}



jobjectArray CreateJFilters(JNIEnv *env, vector<dsignal::Filter*>& filters)
{
    jclass jFilterClass = env->FindClass("cymheart/tau/utils/dsignal/Filter");
// 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jFilterClass, "<init>", "()V");
    jfieldID intPtrField = env->GetFieldID(jFilterClass, "ndkFilter", "J");

//
    jobjectArray jFilterArray = (jobjectArray)env->NewObjectArray(filters.size(), jFilterClass, NULL);

    for(int i=0; i<filters.size(); i++) {
// 创建一个新的对象
        jobject jFilter = env->NewObject(jFilterClass, id);
        env->SetLongField(jFilter, intPtrField, (int64_t) filters[i]);
//
        env->SetObjectArrayElement(jFilterArray, i, jFilter);
    }

    env->DeleteLocalRef(jFilterClass);
    return jFilterArray;
}


bool is_str_gbk(const char* str)
{
    unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
    unsigned char chr = *str;
    bool bAllAscii = true; //如果全部都是ASCII,
    for (unsigned int i = 0; str[i] != '\0'; ++i) {
        chr = *(str + i);
        if ((chr & 0x80) != 0 && nBytes == 0) {// 判断是否ASCII编码,如果不是,说明有可能是GBK
            bAllAscii = false;
        }
        if (nBytes == 0) {
            if (chr >= 0x80) {
                if (chr >= 0x81 && chr <= 0xFE) {
                    nBytes = +2;
                }
                else {
                    return false;
                }
                nBytes--;
            }
        }
        else {
            if (chr < 0x40 || chr>0xFE) {
                return false;
            }
            nBytes--;
        }//else end
    }
    if (nBytes != 0) {   //违返规则
        return false;
    }
    if (bAllAscii) { //如果全部都是ASCII, 也是GBK
        return true;
    }
    return true;
}

bool is_str_utf8(const char* str)
{
    unsigned int nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
    unsigned char chr = *str;
    bool bAllAscii = true;
    for (unsigned int i = 0; str[i] != '\0'; ++i){
        chr = *(str + i);
        //判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
        if (nBytes == 0 && (chr & 0x80) != 0){
            bAllAscii = false;
        }
        if (nBytes == 0) {
            //如果不是ASCII码,应该是多字节符,计算字节数
            if (chr >= 0x80) {
                if (chr >= 0xFC && chr <= 0xFD){
                    nBytes = 6;
                }
                else if (chr >= 0xF8){
                    nBytes = 5;
                }
                else if (chr >= 0xF0){
                    nBytes = 4;
                }
                else if (chr >= 0xE0){
                    nBytes = 3;
                }
                else if (chr >= 0xC0){
                    nBytes = 2;
                }
                else{
                    return false;
                }
                nBytes--;
            }
        }
        else{
            //多字节符的非首字节,应为 10xxxxxx
            if ((chr & 0xC0) != 0x80){
                return false;
            }
            //减到为零为止
            nBytes--;
        }
    }
    //违返UTF8编码规则
    if (nBytes != 0) {
        return false;
    }
    if (bAllAscii){ //如果全部都是ASCII, 也是UTF8
        return true;
    }
    return true;
}
