
#include <jni.h>
#include <string>
#include"../JniUtils/JniUtils.h"
#include<Synth/Ventrue.h>
#include<Synth/VentrueCmd.h>
#include"Synth/VirInstrument.h"
#include"Effect/EffectCmd/EffectEqualizerCmd.h"
#include"dsignal/Bode.h"
#include<Synth/Preset.h>
using namespace ventrue;


#define  LOGw(...) __android_log_print(ANDROID_LOG_ERROR,"wtest",__VA_ARGS__)
void SoundEndVirInstCB(Ventrue* ventrue, VirInstrument** virInst, int size);


static JavaVM* g_vm = NULL;
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv * env = NULL;
    if(g_vm == NULL){
        g_vm = vm;
    }

    if( g_vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK ){
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkTest(JNIEnv *env, jclass clazz)
{
    Ventrue* vtrue = nullptr;
    VentrueCmd* cmd;
    vtrue = new Ventrue();

    cmd = vtrue->GetCmd();

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

static unordered_map<Ventrue*, jobject> jVentrueMap;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_Ventrue_ndkCreateVentrue(JNIEnv *env, jclass clazz, jobject ventrue) {
    Ventrue* v = new Ventrue();
    jVentrueMap[v] = env->NewGlobalRef(ventrue);
    return (int64_t)v;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkDestory(JNIEnv *env, jclass clazz, jlong ndk_ventrue,jlong ndk_soundend_cb) {

    Ventrue* ventrue = (Ventrue*)ndk_ventrue;

    auto it = jVentrueMap.find(ventrue);
    if (it != jVentrueMap.end())
    {
        env->DeleteGlobalRef(it->second);
        jVentrueMap.erase(it);

        jobject oldCB = (jobject)ndk_soundend_cb;
        if(oldCB != NULL)
            env->DeleteGlobalRef(oldCB);
    }
}

extern "C"
JNIEXPORT jobject JNICALL
Java_cymheart_ventrue_Ventrue_ndkGetCmd(JNIEnv *env, jclass clazz, jobject ventrue) {

    jclass jVentrueCmdClass = env->FindClass("cymheart/ventrue/VentrueCmd");
    // 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jVentrueCmdClass, "<init>", "(Lcymheart/ventrue/Ventrue;)V");
    // 创建一个新的对象
    jobject jVentrueCmd = env->NewObject(jVentrueCmdClass, id, ventrue);
    return jVentrueCmd;
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
JNIEXPORT void JNICALL
Java_cymheart_ventrue_Ventrue_ndkSetLimitRegionSounderCount(JNIEnv *env, jclass clazz,
                                                            jlong ndk_ventrue, jint count) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    ventrue->SetLimitRegionSounderCount(count);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkCreateVentrueCmd(JNIEnv *env, jclass clazz, jlong ndk_ventrue) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    VentrueCmd* ventrueCmd = ventrue->GetCmd();
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
JNIEXPORT jobject JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkLoadMidi(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                             jint idx, jboolean is_show_tips) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->LoadMidi(idx);

    if(!is_show_tips)
        return nullptr;

    //
    MidiPlay* midiPlay = ventrueCmd->GetMidiPlay(idx);
    MidiFile* midiFile = midiPlay->GetMidiFile();
    vector<MidiTrack*>* midiTracks = midiFile->GetTrackList();

    //
    jclass jMidiPlayClass = env->FindClass("cymheart/ventrue/MidiPlay");
    jmethodID initMethod = env->GetMethodID(jMidiPlayClass, "<init>", "()V");
    jobject jMidiPlay = env->NewObject(jMidiPlayClass, initMethod);

    jfieldID endSecField = env->GetFieldID(jMidiPlayClass, "endSec","F");
    env->SetFloatField(jMidiPlay, endSecField, midiPlay->GetEndSec());


    //
    jclass jMidiTrackClass = env->FindClass("cymheart/ventrue/midi/MidiTrack");
    jmethodID midiTrackMethod = env->GetMethodID(jMidiTrackClass, "<init>", "()V");
    jobjectArray jMidiTrackArray = (jobjectArray)env->NewObjectArray(midiTracks->size(), jMidiTrackClass, NULL);

    jclass jMidiEventClass = env->FindClass("cymheart/ventrue/midi/MidiEvent");
    jclass jNoteOnEventClass = env->FindClass("cymheart/ventrue/midi/NoteOnEvent");
    jmethodID noteOnEvnetInitMethod = env->GetMethodID(jNoteOnEventClass, "<init>", "()V");

    jfieldID midiEventsField = env->GetFieldID(jMidiTrackClass, "midiEvents",
                                               "[Lcymheart/ventrue/midi/MidiEvent;");

    //
    NoteOnEvent* noteOnEvent;
    for(int i=0; i<midiTracks->size(); i++)
    {
        // 创建一个新的对象
        jobject jMidiTrack = env->NewObject(jMidiTrackClass, midiTrackMethod);

        //
        list<MidiEvent*>* midiEventList = (*midiTracks)[i]->GetEventList();
        jobjectArray jMidiEventArray = (jobjectArray)env->NewObjectArray(midiEventList->size(), jMidiEventClass, NULL);

        list<MidiEvent*>::iterator it = midiEventList->begin();
        list<MidiEvent*>::iterator end = midiEventList->end();
        for (int j = 0; it != end; it++, j++)
        {
            if((*it)->type != MidiEventType::NoteOn)
                continue;

            noteOnEvent = (NoteOnEvent*)(*it);

            //
            jobject jNoteOnEvent = env->NewObject(jNoteOnEventClass, noteOnEvnetInitMethod);

            //
            jfieldID startSecField = env->GetFieldID(jNoteOnEventClass, "startSec", "F");
            jfieldID endSecField = env->GetFieldID(jNoteOnEventClass, "endSec", "F");
            jfieldID channelField = env->GetFieldID(jNoteOnEventClass, "channel", "I");
            jfieldID noteField = env->GetFieldID(jNoteOnEventClass, "note", "I");

            env->SetFloatField(jNoteOnEvent, startSecField, noteOnEvent->startSec);
            env->SetFloatField(jNoteOnEvent, endSecField, noteOnEvent->endSec);
            env->SetIntField(jNoteOnEvent, channelField, noteOnEvent->channel);
            env->SetIntField(jNoteOnEvent, noteField, noteOnEvent->note);


            //
            env->SetObjectArrayElement(jMidiEventArray, j, jNoteOnEvent);
        }


        env->SetObjectField(jMidiTrack, midiEventsField, jMidiEventArray);
        env->SetObjectArrayElement(jMidiTrackArray, i, jMidiTrack);

    }

    jfieldID midiTracksField = env->GetFieldID(jMidiPlayClass, "midiTracks",
                                            "[Lcymheart/ventrue/midi/MidiTrack;");

    env->SetObjectField(jMidiPlay, midiTracksField, jMidiTrackArray);

    env->DeleteLocalRef(jMidiEventClass);
    env->DeleteLocalRef(jNoteOnEventClass);
    env->DeleteLocalRef(jMidiTrackClass);
    env->DeleteLocalRef(jMidiPlayClass);

    return jMidiPlay;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkPlayMidi(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                             jint idx) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->PlayMidi(idx);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkStopMidi(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                             jint idx) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->StopMidi(idx);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkRemoveMidi(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                               jint idx) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->RemoveMidi(idx);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkMidiGoto(JNIEnv *env, jclass clazz, jlong ndk_ventrue_cmd,
                                             jint idx, jfloat sec) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->MidiGoto(idx, sec);
}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_ventrue_VirInstrument_ndkGetState(JNIEnv *env, jobject thiz,
                                                jlong ndk_vir_instrument) {
    VirInstrument* inst = (VirInstrument* )ndk_vir_instrument;
    return (int)(inst->GetState());
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkAppendReplaceInstrument(JNIEnv *env, jclass clazz,
                                                            jlong ndk_ventrue_cmd,
                                                            jint org_bank_msb, jint org_bank_lsb,
                                                            jint org_inst_num, jint rep_bank_msb,
                                                            jint rep_bank_lsb, jint rep_inst_num) {

    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;

    ventrueCmd->AppendReplaceInstrument(
            org_bank_msb, org_bank_lsb, org_inst_num,
            rep_bank_msb, rep_bank_lsb, rep_inst_num);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkRemoveReplaceInstrument(JNIEnv *env, jclass clazz,
                                                            jlong ndk_ventrue_cmd,
                                                            jint org_bank_msb, jint org_bank_lsb,
                                                            jint org_inst_num) {

    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->RemoveReplaceInstrument(org_bank_msb, org_bank_lsb, org_inst_num);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkNewVirInstrument(JNIEnv *env, jclass clazz,
                                                     jlong ndk_ventrue_cmd, jint bank_select_msb,
                                                     jint bank_select_lsb, jint instrument_num) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    VirInstrument* virInst = ventrueCmd->NewVirInstrument(bank_select_msb, bank_select_lsb ,instrument_num);

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
Java_cymheart_ventrue_VentrueCmd_ndkRemoveVirInstrument(JNIEnv *env, jclass clazz,
                                                        jlong ndk_ventrue_cmd,
                                                        jlong ndk_vir_instrument,
                                                        jboolean is_fade) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->RemoveVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkOnVirInstrument(JNIEnv *env, jclass clazz,
                                                    jlong ndk_ventrue_cmd,
                                                    jlong ndk_vir_instrument,
                                                    jboolean is_fade) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->OnVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VentrueCmd_ndkOffVirInstrument(JNIEnv *env, jclass clazz,
                                                     jlong ndk_ventrue_cmd,
                                                     jlong ndk_vir_instrument,
                                                     jboolean is_fade) {
    VentrueCmd* ventrueCmd = (VentrueCmd*)ndk_ventrue_cmd;
    ventrueCmd->OffVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
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

    env->DeleteLocalRef(jFilterClass);
    return jFilterArray;
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_cymheart_ventrue_Ventrue_ndkGetPresetList(JNIEnv *env, jclass clazz, jlong ndk_ventrue) {
    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    PresetList* presetList = ventrue->GetPresetList();
    int sz = presetList->size();

    jclass jPresetClass = env->FindClass("cymheart/ventrue/Preset");
    // 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jPresetClass, "<init>", "()V");


    //env->PushLocalFrame(presetList->size());
    jobjectArray jPresetArray = (jobjectArray)env->NewObjectArray(presetList->size(), jPresetClass, NULL);

    jfieldID nameField = env->GetFieldID(jPresetClass, "name", "Ljava/lang/String;");
    jfieldID bankSelectMSBField = env->GetFieldID(jPresetClass, "bankSelectMSB", "I");
    jfieldID bankSelectLSBField = env->GetFieldID(jPresetClass, "bankSelectLSB", "I");
    jfieldID instrumentNumField = env->GetFieldID(jPresetClass, "instrumentNum", "I");

    for(int i=0; i<sz; i++) {
        // 创建一个新的对象
        jobject jPreset = env->NewObject(jPresetClass, id);

        const char* str = (*presetList)[i]->name.data();
        jstring jstr = str2jstring(env, str);
        env->SetObjectField(jPreset, nameField, jstr);
        env->SetIntField(jPreset, bankSelectMSBField, (*presetList)[i]->bankSelectMSB);
        env->SetIntField(jPreset, bankSelectLSBField, (*presetList)[i]->bankSelectLSB);
        env->SetIntField(jPreset, instrumentNumField, (*presetList)[i]->instrumentNum);

        //
        env->SetObjectArrayElement(jPresetArray, i, jPreset);
    }


    //env->PopLocalFrame(jPresetArray);
    env->DeleteLocalRef(jPresetClass);
    return jPresetArray;

}

static jobject g_virInstrumentObj = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_ventrue_VirInstrument_setClsRef(JNIEnv *env, jobject thiz) {
    if (g_virInstrumentObj == NULL) {
        g_virInstrumentObj = env->NewGlobalRef(thiz);//获取全局引用
        if (thiz != NULL) {
            env->DeleteLocalRef(thiz);
        }//释放局部对象.这里可不要，调用结束后虚拟机会释放
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_ventrue_Ventrue_ndkSetSoundEndVirInstCallBack(JNIEnv *env, jobject thiz,
                                                            jlong ndk_ventrue, jlong ndk_soundend_cb, jobject cb){

    jobject oldCB = (jobject)ndk_soundend_cb;
    if(oldCB != NULL)
        env->DeleteGlobalRef(oldCB);

    Ventrue* ventrue = (Ventrue*)ndk_ventrue;
    int64_t cbPtr = 0;
    if(cb != NULL) {
        ventrue->soundEndVirInstCallBack = SoundEndVirInstCB;
        cbPtr = (int64_t)(env->NewGlobalRef(cb));
    } else{
        ventrue->soundEndVirInstCallBack = NULL;
    }

    return cbPtr;
}


void SoundEndVirInstCB(Ventrue* ventrue, VirInstrument** virInst, int size)
{
    JNIEnv* env;
    bool isAttached = false;
    int status = g_vm->GetEnv((void**)&env, JNI_VERSION_1_4);
    if(status < 0)
    {
        g_vm->AttachCurrentThread(&env,NULL);
        isAttached = true;
    }

    //
    jobject cbObj = NULL;
    auto it = jVentrueMap.find(ventrue);
    if (it != jVentrueMap.end()) {
        jobject jVentrue = it->second;
        jclass jVentrueClass = env->GetObjectClass(jVentrue);
        jfieldID cbPtrField = env->GetFieldID(jVentrueClass, "ndkSoundEndCB", "J");
        jlong ptr = env->GetLongField(jVentrue, cbPtrField);
        cbObj = (jobject)ptr;
    }

    if(cbObj == NULL)
        return;

    //
    jclass jclsProcess = env->GetObjectClass(cbObj);
    if (jclsProcess == NULL)
        return;

    jmethodID jmidProcess = env->GetMethodID(jclsProcess,"Execute","(Ljava/lang/Object;)V");
    if (jmidProcess == NULL)
        return;

    jclass jVirInstClass = env->GetObjectClass(g_virInstrumentObj);
    // 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jVirInstClass, "<init>", "()V");
    jfieldID intPtrField = env->GetFieldID(jVirInstClass, "ndkVirInstrument", "J");

    env->PushLocalFrame(size);
    jobjectArray jVirInstArray = (jobjectArray)env->NewObjectArray(size, jVirInstClass, NULL);

    for(int i=0; i<size; i++) {
        // 创建一个新的对象
        jobject jVirInst = env->NewObject(jVirInstClass, id);
        env->SetLongField(jVirInst, intPtrField, (int64_t)(virInst[i]));

        //
        env->SetObjectArrayElement(jVirInstArray, i, jVirInst);
    }

    env->CallVoidMethod(cbObj, jmidProcess, jVirInstArray);

    env->PopLocalFrame(jVirInstArray);
    env->DeleteLocalRef(jVirInstClass);

    if(isAttached)
        g_vm->DetachCurrentThread();
}

