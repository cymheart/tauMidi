//
// Created by cym on 2021/8/2.
//

#include"../JniUtils/JniUtils.h"
#include<Synth/Tau.h>
#include<Midi/MidiFile.h>
#include<Midi/MidiEvent.h>
#include<Synth/Preset.h>
#include<Synth/SoundFont.h>
#include<Synth/Editor/Editor.h>
#include <unistd.h>

using namespace tau;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_Tau_ndkCreateTau(JNIEnv *env, jclass clazz, jobject jtau, jobject jeditor) {
    Tau* tau = new Tau();
    Editor* editor = tau->GetEditor();

    jclass jEditorClass = env->GetObjectClass(jeditor);
    jfieldID ndkEditorField = env->GetFieldID(jEditorClass, "ndkEditor", "J");
    env->SetLongField(jeditor, ndkEditorField, (jlong)editor);

    jclass cls = env->FindClass("cymheart/tau/editor/Editor");
    jmethodID mid = env->GetMethodID(cls, "Init", "()V");
    env->CallVoidMethod(jeditor, mid);


    env->DeleteLocalRef(jEditorClass);
    env->DeleteLocalRef(cls);

    return (int64_t)tau;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkDeleteTau(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    delete tau;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOpen(JNIEnv *env, jclass clazz,  jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Open();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkClose(JNIEnv *env, jclass clazz,  jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Close();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetSilence(JNIEnv *env, jclass clazz, jlong ndk_tau, jboolean is) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetSilence(is);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetSoundFont(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                      jlong ndk_sound_font) {
    Tau* tau = (Tau*)ndk_tau;
    SoundFont* sf = (SoundFont*)ndk_sound_font;
    tau->SetSoundFont(sf);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetUseMulThread(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                             jboolean is_use) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetUseMulThread(is_use);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetFrameSampleCount(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                 jint sample_count) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetFrameSampleCount(sample_count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetSampleProcessRate(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                              jint sample_rate) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetSampleProcessRate(sample_rate);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetChannelCount(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                             jint channel_count) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetChannelOutputMode((ChannelOutputMode)channel_count);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetSampleStreamCacheSec(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                              jfloat sec) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetSampleStreamCacheSec(sec);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetLimitRegionSounderCount(JNIEnv *env, jclass clazz,
                                                        jlong ndk_tau, jint count) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetLimitRegionSounderCount(count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetSetLimitOnKeySpeed(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                   jfloat speed) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetLimitOnKeySpeed(speed);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetEnableMergeNotesOptimize(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                     jboolean enable) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetEnableMergeNotesOptimize(enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetEnableMidiEventCountOptimize(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                         jboolean enable) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetEnableMidiEventCountOptimize(enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetEnableCopySameChannelControlEvents(JNIEnv *env, jclass clazz,
                                                               jlong ndk_tau, jboolean enable) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetEnableCopySameChannelControlEvents(enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetMidiKeepSameTimeNoteOnCount(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                        jint count) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetMidiKeepSameTimeNoteOnCount(count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetEnableMidiEventParseLimitTime(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                          jboolean enable, jfloat limit_sec) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetEnableMidiEventParseLimitTime(enable, limit_sec);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetEnableCreateFreqSpectrums(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                      jboolean enable, jint count) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetEnableCreateFreqSpectrums(enable, count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetEnableAllVirInstEffects(JNIEnv *env, jclass clazz,
                                                               jlong ndk_tau,
                                                               jboolean is_enable) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetEnableAllVirInstEffects(is_enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkAppendReplaceInstrument(JNIEnv *env, jclass clazz,
                                                            jlong ndk_tau,
                                                            jint org_bank_msb, jint org_bank_lsb,
                                                            jint org_inst_num, jint rep_bank_msb,
                                                            jint rep_bank_lsb, jint rep_inst_num) {

    Tau* tau = (Tau*)ndk_tau;

    tau->AppendReplaceInstrument(
            org_bank_msb, org_bank_lsb, org_inst_num,
            rep_bank_msb, rep_bank_lsb, rep_inst_num);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkRemoveReplaceInstrument(JNIEnv *env, jclass clazz,
                                                            jlong ndk_tau,
                                                            jint org_bank_msb, jint org_bank_lsb,
                                                            jint org_inst_num) {

    Tau* tau = (Tau*)ndk_tau;
    tau->RemoveReplaceInstrument(org_bank_msb, org_bank_lsb, org_inst_num);
}


extern "C"
JNIEXPORT jobject JNICALL
Java_cymheart_tau_Tau_ndkEnableVirInstrument(JNIEnv *env, jclass clazz,
                                                        jlong ndk_tau,
                                                        jint device_channel_num,
                                                        jint bank_select_msb, jint bank_select_lsb,
                                                        jint instrument_num) {

    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = tau->EnableVirInstrument(
            device_channel_num, bank_select_msb, bank_select_lsb ,instrument_num);

    jclass jVirInstClass = env->FindClass("cymheart/tau/VirInstrument");
    // 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jVirInstClass, "<init>", "()V");
    // 创建一个新的对象
    jobject jVirInst = env->NewObject(jVirInstClass, id);
    jfieldID intPtrField = env->GetFieldID(jVirInstClass, "ndkVirInstrument", "J");
    env->SetLongField(jVirInst, intPtrField, (int64_t)virInst);

    env->DeleteLocalRef(jVirInstClass);

    return jVirInst;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetVirInstrumentProgram(JNIEnv *env, jclass clazz,
                                                            jlong ndk_tau,
                                                            jlong ndk_vir_inst,
                                                            jint bank_select_msb,
                                                            jint bank_select_lsb,
                                                            jint instrument_num) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* inst = (VirInstrument*)ndk_vir_inst;
    tau->SetVirInstrumentProgram(
            inst, bank_select_msb, bank_select_lsb ,instrument_num);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetVirInstrumentPitchBend(JNIEnv *env, jclass clazz,
                                                              jlong ndk_tau,
                                                              jlong ndk_vir_inst, jint value) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* inst = (VirInstrument*)ndk_vir_inst;
    tau->SetVirInstrumentPitchBend(inst, value);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSetVirInstrumentMidiControllerValue(JNIEnv *env, jclass clazz,
                                                                        jlong ndk_tau,
                                                                        jlong ndk_vir_inst,
                                                                        jint type, jint value) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* inst = (VirInstrument*)ndk_vir_inst;
    tau->SetVirInstrumentMidiControllerValue(inst, (MidiControllerType)type, value);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkRemoveVirInstrument(JNIEnv *env, jclass clazz,
                                                        jlong ndk_tau,
                                                        jlong ndk_vir_instrument,
                                                        jboolean is_fade) {
    Tau* tau = (Tau*)ndk_tau;
    tau->RemoveVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOnVirInstrument(JNIEnv *env, jclass clazz,
                                                    jlong ndk_tau,
                                                    jlong ndk_vir_instrument,
                                                    jboolean is_fade) {
    Tau* tau = (Tau*)ndk_tau;
    tau->OpenVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOffVirInstrument(JNIEnv *env, jclass clazz,
                                                     jlong ndk_tau,
                                                     jlong ndk_vir_instrument,
                                                     jboolean is_fade) {
    Tau* tau = (Tau*)ndk_tau;
    tau->CloseVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOnKey(JNIEnv *env, jclass clazz, jlong ndk_tau, jint key,
                                       jfloat velocity, jlong ndk_vir_inst, jint id) {

    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    tau->OnKey(key, velocity, virInst, id);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOffKey(JNIEnv *env, jclass clazz, jlong ndk_tau, jint key,
                                        jfloat velocity, jlong ndk_vir_inst, jint id) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    tau->OffKey(key, velocity, virInst, id);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkAddEffect(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                              jlong ndk_effect) {
    Tau* tau = (Tau*)ndk_tau;
    TauEffect* effect = (TauEffect*)ndk_effect;
    tau->AddEffect(effect);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkClearRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->ClearRecordPCM();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkStartRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->StartRecordPCM();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkStopRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->StopRecordPCM();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSaveRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_tau, jstring path) {
    Tau* tau = (Tau*)ndk_tau;
    string p =jstring2str(env, path);
    tau->SaveRecordPCM(p);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSaveRecordPCMToWav(JNIEnv *env, jclass clazz, jlong ndk_tau, jstring path) {
    Tau* tau = (Tau*)ndk_tau;
    string p =jstring2str(env, path);
    tau->SaveRecordPCMToWav(p);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkSaveRecordPCMToMp3(JNIEnv *env, jclass clazz, jlong ndk_tau, jstring path) {
    Tau* tau = (Tau*)ndk_tau;
    string p =jstring2str(env, path);
    tau->SaveRecordPCMToMp3(p);
}