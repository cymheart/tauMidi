//
// Created by cym on 2023/7/23.
//

#include <jni.h>
#include<FX/SpectrumVisual.h>
#include"../../JniUtils/JniUtils.h"
#include <jni.h>

using namespace tauFX;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_FX_SpectrumVisual_ndkCreateSpectrumVisual(JNIEnv *env, jclass clazz) {
    auto* sp = new SpectrumVisual();
    return (int64_t)sp;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_SpectrumVisual_ndkInit(JNIEnv *env, jclass clazz,  jlong ndk_effect, jint number_of_bars,
                                            jint rate, jint channels,
                                            jint start_freq, jint end_freq,
                                            jint low_cut_off,jint high_cut_off,
                                            jint skip_frame){

    auto* sp = (SpectrumVisual*)ndk_effect;

    sp->Init(number_of_bars, rate, channels,
            start_freq, end_freq,
            low_cut_off, high_cut_off,
            skip_frame);

}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_FX_SpectrumVisual_ndkGetAmpBars(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                  jdoubleArray amp_of_bars,
                                                  jintArray freq_of_bars) {
    auto* sp = (SpectrumVisual*)ndk_effect;

    jdouble* amps = env->GetDoubleArrayElements(amp_of_bars, 0);
    jint* freqs = env->GetIntArrayElements(freq_of_bars, 0);
    int count = sp->GetAmpBars(amps, freqs);

    env->SetDoubleArrayRegion(amp_of_bars, 0, count * sp->GetChannels(), amps);
    env->SetIntArrayRegion(freq_of_bars, 0, count, freqs);

    env->ReleaseDoubleArrayElements(amp_of_bars, amps,JNI_ABORT);
    env->ReleaseIntArrayElements(freq_of_bars, freqs,JNI_ABORT);

    return count;
}
extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_FX_SpectrumVisual_ndkCreateSmoothAmpBars(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                           jdoubleArray smooth_amps, jfloat min_db,
                                                           jint interp_count, jfloat smooth_coffe,
                                                           jint channel) {
    auto* sp = (SpectrumVisual*)ndk_effect;
    jdouble* amps = env->GetDoubleArrayElements(smooth_amps, 0);
    int count = sp->CreateSmoothAmpBars(amps, min_db, interp_count, smooth_coffe, channel);
    env->SetDoubleArrayRegion(smooth_amps, 0, count, amps);
    env->ReleaseDoubleArrayElements(smooth_amps, amps,JNI_ABORT);
    return count;
}
