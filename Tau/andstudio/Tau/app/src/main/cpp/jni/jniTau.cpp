//
// Created by cym on 2021/8/2.
//

#include"../JniUtils/JniUtils.h"
#include<Synth/Tau.h>
#include<Midi/MidiFile.h>
#include<Midi/MidiEvent.h>
#include<Synth/Preset.h>
#include<Synth/SoundFont.h>

using namespace tau;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_Tau_ndkCreateTau(JNIEnv *env, jclass clazz, jobject tau) {
    Tau* v = new Tau();
    return (int64_t)v;
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
Java_cymheart_tau_Tau_ndkSetUnitProcessMidiTrackCount(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                                      jint count) {
    Tau* tau = (Tau*)ndk_tau;
    tau->SetUnitProcessMidiTrackCount(count);
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
    tau->OnVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOffVirInstrument(JNIEnv *env, jclass clazz,
                                                     jlong ndk_tau,
                                                     jlong ndk_vir_instrument,
                                                     jboolean is_fade) {
    Tau* tau = (Tau*)ndk_tau;
    tau->OffVirInstrument((VirInstrument*)ndk_vir_instrument, is_fade);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOnKey(JNIEnv *env, jclass clazz, jlong ndk_tau, jint key,
                                       jfloat velocity, jlong ndk_vir_inst, jint delayMS) {

    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    tau->OnKey(key, velocity, virInst, delayMS);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkOffKey(JNIEnv *env, jclass clazz, jlong ndk_tau, jint key,
                                        jfloat velocity, jlong ndk_vir_inst, jint delayMS) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    tau->OffKey(key, velocity, virInst, delayMS);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkCancelOnKey(JNIEnv *env, jclass clazz, jlong ndk_tau, jint key,
                                     jfloat velocity, jlong ndk_vir_inst, jint delay_ms) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    tau->CancelDownKey(key, velocity, virInst, delay_ms);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkCancelOffKey(JNIEnv *env, jclass clazz, jlong ndk_tau, jint key,
                                      jfloat velocity, jlong ndk_vir_inst, jint delay_ms) {
    Tau* tau = (Tau*)ndk_tau;
    VirInstrument* virInst = (VirInstrument*)ndk_vir_inst;
    tau->CancelOffKey(key, velocity, virInst, delay_ms);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkAddEffect(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                              jlong ndk_effect) {
    Tau* tau = (Tau*)ndk_tau;
    TauEffect* effect = (TauEffect*)ndk_effect;
    tau->AddEffect(effect);
}

