
#include <jni.h>
#include <string>
#include"../JniUtils/JniUtils.h"
#include<Synth/Ventrue.h>
#include<Synth/VentrueCmd.h>
#include"Synth/VirInstrument.h"
#include"Effect/EffectCmd/EffectEqualizerCmd.h"
#include"dsignal/Bode.h"
using namespace ventrue;


#define  LOGw(...) __android_log_print(ANDROID_LOG_ERROR,"wtest",__VA_ARGS__)


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkTest(JNIEnv *env, jclass clazz)
{
    Ventrue* vtrue = nullptr;
    VentrueCmd* cmd;
    vtrue = new Ventrue();

    cmd = new VentrueCmd(vtrue);

    vtrue->SetFrameSampleCount(512);
   // vtrue->SetSampleProcessRate(44100);
    //vtrue->SetChildFrameSampleCount(64);
    //vtrue->SetUseMulThread(false);
   // vtrue->SetRenderQuality(RenderQuality::Fast);
    vtrue->OpenAudio();

    vtrue->ParseSoundFont("SF2", "/storage/emulated/0/GeneralUser GS MuseScore v1.442.sf2");

    cmd->AppendMidiFile("/storage/emulated/0/(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");
    //cmd->AppendMidiFile("/storage/emulated/0/QianQianQueGe.mid");
    cmd->PlayMidi(0);
//    VirInstrument* vinst = cmd->EnableVirInstrument(0, 0, 0, 20);
//
//    if(vinst!= nullptr) {
//
//        for (int j = 0; j < 3; j++) {
//
//            for (int i = 0; i < 50; i++) {
//                cmd->OnKey(i + 50, 127, vinst);
//                this_thread::sleep_for(std::chrono::milliseconds((long) 450));
//                cmd->OffKey(i + 50, 127, vinst);
//            }
//
//            VirInstrument *vinst = cmd->EnableVirInstrument(0, 0, 0, j);
//        }
//    }


   // this_thread::sleep_for(std::chrono::milliseconds((long)50000000));
   // string a = "sussecs!!!";
   //     return env->NewStringUTF(a.c_str());;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_Ventrue_ndkCreateVentrue(JNIEnv *env, jclass clazz) {
    Ventrue* ventrue = new Ventrue();
    return (int64_t)ventrue;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkSetFrameSampleCount(JNIEnv *env, jclass clazz, jlong ndk_ventrue,
                                                     jint sample_count) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    ventrue->SetFrameSampleCount(sample_count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkSetChannelCount(JNIEnv *env, jclass clazz, jlong ndk_ventrue,
                                                 jint channel_count) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    ventrue->SetChannelOutputMode((ChannelOutputMode)channel_count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkOpenAudio(JNIEnv *env, jclass clazz, jlong ndk_ventrue) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    ventrue->OpenAudio();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkParseSoundFont(JNIEnv *env, jclass clazz, jlong ndk_ventrue,
                                                jstring formatName, jstring filepath) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    string _formatName = jstring2str(env, formatName);
    string path = jstring2str(env, filepath);
    ventrue->ParseSoundFont(_formatName, path);
}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_ventrue_Ventrue_ndkGetTotalRegionSounderCount(JNIEnv *env, jclass clazz,
                                                            jlong ndk_ventrue) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    return ventrue->GetTotalRegionSounderCount();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkCreateVentrueCmd(JNIEnv *env, jclass clazz, jlong ndk_ventrue) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    VentrueCmd* ventrueCmd = new VentrueCmd(ventrue);
    return (int64_t)ventrueCmd;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkAppendMidiFile(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                                   jstring midifile) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    string _midifile = jstring2str(env, midifile);
    ventrueCmd->AppendMidiFile(_midifile);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkPlayMidi(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                             jint midi_file_idx) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->PlayMidi(midi_file_idx);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkEnableVirInstrument(JNIEnv *env, jclass clazz,
                                                        jlong ndk_ventrue_cmd,
                                                        jint device_channel_num,
                                                        jint bank_select_msb, jint bank_select_lsb,
                                                        jint instrument_num) {

    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    VirInstrument* virInst = ventrueCmd->EnableVirInstrument(
            device_channel_num, bank_select_msb, bank_select_lsb ,instrument_num);

    jclass jVirInstClass = env->FindClass("cymheart/ventrue/VirInstrument");
    // 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jVirInstClass, "<init>", "()V");
    // 创建一个新的对象
    jobject jVirInst = env->NewObject(jVirInstClass, id);
    jfieldID intPtrField = env->GetFieldID(jVirInstClass, "ndkVirInstrument", "J");
    env->SetLongField(jVirInst, intPtrField, (int64_t)virInst);
    return jVirInst;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_OnKey(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd, jint key,
                                       jfloat velocity, jlong ndk_vir_inst) {

    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    ventrueCmd->OnKey(key, velocity, virInst);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_OffKey(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd, jint key,
                                        jfloat velocity, jlong ndk_vir_inst) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    ventrueCmd->OffKey(key, velocity, virInst);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_effect_Equalizer_ndkCreateEqualizer(JNIEnv *env, jclass clazz) {
    EffectEqualizer* eq = new EffectEqualizer();
    return (int64_t)eq;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_effect_EffectEqualizerCmd_ndkCreateEffectEqualizerCmd(JNIEnv *env,
                                                                            jclass clazz,
                                                                            jlong ndk_ventrue,
                                                                            jlong ndk_equalizer) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    EffectEqualizer* eq = (EffectEqualizer*)ndk_equalizer;
    eq->SetFreqBandGain(0, 1);
    EffectEqualizerCmd* cmd = new EffectEqualizerCmd(ventrue, eq);
    return  (int64_t)cmd;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_effect_EffectEqualizerCmd_ndkSetFreqBandGain(JNIEnv *env, jclass clazz,
                                                                   jlong ndk_effect_eq_cmd,
                                                                   jint band_idx, jfloat gain_db) {
    EffectEqualizerCmd* cmd = (EffectEqualizerCmd*)ndk_effect_eq_cmd;
    cmd->SetFreqBandGain(band_idx, gain_db);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkAddEffect(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                              jlong ndk_effect) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    VentrueEffect* effect = (VentrueEffect*)ndk_effect;
    ventrueCmd->AddEffect(effect);
}
extern "C"
JNIEXPORT jobjectArray JNICALL
Java_cymheart_ventrue_effect_Equalizer_ndkGetFilter(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    EffectEqualizer* eq = (EffectEqualizer*)ndk_effect;
    vector<dsignal::Filter*> filters = eq->GetFilters();

    jclass jFilterClass = env->FindClass("cymheart/ventrue/dsignal/Filter");
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

    return jFilterArray;
}

