
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
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("java/lang/String");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("UTF8");
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
    jclass jFilterClass = env->FindClass("cymheart/tau/dsignal/Filter");
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

