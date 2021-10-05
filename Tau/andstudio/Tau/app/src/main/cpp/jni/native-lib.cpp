
#include <jni.h>
#include <string>
#include <unistd.h>

#define  LOGw(...) __android_log_print(ANDROID_LOG_ERROR,"wtest",__VA_ARGS__)


//static JavaVM* g_vm = NULL;
//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
//{
//    JNIEnv * env = NULL;
//    if(g_vm == NULL){
//        g_vm = vm;
//    }
//
//    if( g_vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK ){
//        return JNI_ERR;
//    }
//    return JNI_VERSION_1_4;
//}
//



//static jobject g_virInstrumentObj = NULL;
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_cymheart_ventrue_VirInstrument_setClsRef(JNIEnv *env, jobject thiz) {
//    if (g_virInstrumentObj == NULL) {
//        g_virInstrumentObj = env->NewGlobalRef(thiz);//获取全局引用
//        if (thiz != NULL) {
//            env->DeleteLocalRef(thiz);
//        }//释放局部对象.这里可不要，调用结束后虚拟机会释放
//    }
//}

//extern "C"
//JNIEXPORT jlong JNICALL
//Java_cymheart_ventrue_Ventrue_ndkSetSoundEndVirInstCallBack(JNIEnv *env, jobject thiz,
//                                                            jlong ndk_ventrue, jlong ndk_soundend_cb, jobject cb){
//
//    jobject oldCB = (jobject)ndk_soundend_cb;
//    if(oldCB != NULL)
//        env->DeleteGlobalRef(oldCB);
//
//    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
//    int64_t cbPtr = 0;
//    if(cb != NULL) {
//        ventrue->soundEndVirInstCallBack = SoundEndVirInstCB;
//        cbPtr = (int64_t)(env->NewGlobalRef(cb));
//    } else{
//        ventrue->soundEndVirInstCallBack = NULL;
//    }
//
//    return cbPtr;
//}
//
//
//void SoundEndVirInstCB(Ventrue* ventrue, VirInstrument** virInst, int size)
//{
//    JNIEnv* env;
//    bool isAttached = false;
//    int status = g_vm->GetEnv((void**)&env, JNI_VERSION_1_4);
//    if(status < 0)
//    {
//        g_vm->AttachCurrentThread(&env,NULL);
//        isAttached = true;
//    }
//
//    //
//    jobject cbObj = NULL;
//    auto it = jVentrueMap.find(ventrue);
//    if (it != jVentrueMap.end()) {
//        jobject jVentrue = it->second;
//        jclass jVentrueClass = env->GetObjectClass(jVentrue);
//        jfieldID cbPtrField = env->GetFieldID(jVentrueClass, "ndkSoundEndCB", "J");
//        jlong ptr = env->GetLongField(jVentrue, cbPtrField);
//        cbObj = (jobject)ptr;
//    }
//
//    if(cbObj == NULL)
//        return;
//
//    //
//    jclass jclsProcess = env->GetObjectClass(cbObj);
//    if (jclsProcess == NULL)
//        return;
//
//    jmethodID jmidProcess = env->GetMethodID(jclsProcess,"Execute","(Ljava/lang/Object;)V");
//    if (jmidProcess == NULL)
//        return;
//
//    jclass jVirInstClass = env->GetObjectClass(g_virInstrumentObj);
//    // 获取类的构造函数，记住这里是调用无参的构造函数
//    jmethodID id = env->GetMethodID(jVirInstClass, "<init>", "()V");
//    jfieldID intPtrField = env->GetFieldID(jVirInstClass, "ndkVirInstrument", "J");
//
//    env->PushLocalFrame(size);
//    jobjectArray jVirInstArray = (jobjectArray)env->NewObjectArray(size, jVirInstClass, NULL);
//
//    for(int i=0; i<size; i++) {
//        // 创建一个新的对象
//        jobject jVirInst = env->NewObject(jVirInstClass, id);
//        env->SetLongField(jVirInst, intPtrField, (int64_t)(virInst[i]));
//
//        //
//        env->SetObjectArrayElement(jVirInstArray, i, jVirInst);
//    }
//
//    env->CallVoidMethod(cbObj, jmidProcess, jVirInstArray);
//
//    env->PopLocalFrame(jVirInstArray);
//    env->DeleteLocalRef(jVirInstClass);
//
//    if(isAttached)
//        g_vm->DetachCurrentThread();
//}

