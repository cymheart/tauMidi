//
// Created by cym on 2023/7/23.
//

#include <jni.h>
#include<FX/PcmRecorder.h>
#include"../../JniUtils/JniUtils.h"
#include <jni.h>

using namespace tauFX;


extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkCreatePcmRecorder(JNIEnv *env, jclass clazz) {
    PcmRecorder* recorder = new PcmRecorder();
    return (int64_t)recorder;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkClearRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    PcmRecorder* pcmRecorder = (PcmRecorder*)ndk_effect;
    pcmRecorder->ClearRecordPCM();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkStartRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    PcmRecorder* pcmRecorder = (PcmRecorder*)ndk_effect;
    pcmRecorder->StartRecordPCM();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkStopRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    PcmRecorder* pcmRecorder = (PcmRecorder*)ndk_effect;
    pcmRecorder->StopRecordPCM();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkSaveRecordPCM(JNIEnv *env, jclass clazz, jlong ndk_effect, jstring path) {
    PcmRecorder* pcmRecorder = (PcmRecorder*)ndk_effect;
    string p =jstring2str(env, path);
    pcmRecorder->SaveRecordPCM(p);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkSaveRecordPCMToWav(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                       jstring path, jint sampleRate, jint numChannels) {
    PcmRecorder* pcmRecorder = (PcmRecorder*)ndk_effect;
    string p =jstring2str(env, path);
    pcmRecorder->SaveRecordPCMToWav(p, sampleRate, numChannels);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_PcmRecorder_ndkSaveRecordPCMToMp3(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                       jstring path, jint sampleRate, jint numChannels) {
    PcmRecorder* pcmRecorder = (PcmRecorder*)ndk_effect;
    string p = jstring2str(env, path);
    pcmRecorder->SaveRecordPCMToMp3(p, sampleRate, numChannels);
}
