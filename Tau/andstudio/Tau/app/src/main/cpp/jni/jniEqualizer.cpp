//
// Created by cym on 2021/8/2.
//
#include <jni.h>
#include<FX/Equalizer.h>
#include"../JniUtils/JniUtils.h"

using namespace tauFX;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_effect_Equalizer_ndkCreateEqualizer(JNIEnv *env, jclass clazz) {
    Equalizer* eq = new Equalizer();
    eq->SetFreqBandGain(0, 1);
    return (int64_t)eq;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Equalizer_ndkSetFreqBandGain(JNIEnv *env, jclass clazz,
        jlong ndk_effect, jint band_idx,
        jfloat gain_db) {
    Equalizer* eq = (Equalizer*)ndk_effect;
eq->SetFreqBandGainTask(band_idx, gain_db);
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_cymheart_tau_effect_Equalizer_ndkGetFilter(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    Equalizer* eq = (Equalizer*)ndk_effect;
    vector<dsignal::Filter*> filters = eq->GetFilters();
    return CreateJFilters(env, filters);
}

