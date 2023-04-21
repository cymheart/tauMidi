#include <jni.h>
#include<FX/Compressor.h>
#include"../../JniUtils/JniUtils.h"
#include <jni.h>

using namespace tauFX;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_effect_Compressor_ndkCreateCompressor(JNIEnv *env, jclass clazz) {
    Compressor* compressor = new Compressor();
    return (int64_t)compressor;
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetSampleFreq(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                     jfloat freq) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetSampleFreq(freq);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetAttackSec(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                    jfloat sec) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetAttackSec(sec);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetReleaseSec(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                     jfloat sec) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetReleaseSec(sec);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetRadio(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                jfloat radio) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetRadio(radio);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetThreshold(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                    jfloat threshold) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetThreshold(threshold);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetKneeWidth(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                    jfloat width) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetKneeWidth(width);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetMakeupGain(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                     jfloat gain) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetMakeupGain(gain);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkSetAutoMakeupGain(JNIEnv *env, jclass clazz,
                                                         jlong ndk_effect, jboolean is_auto) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->SetAutoMakeupGain(is_auto);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Compressor_ndkCalculateCoefficients(JNIEnv *env, jclass clazz,
                                                             jlong ndk_effect) {
    Compressor* compressor = (Compressor*)ndk_effect;
    compressor->CalculateCoefficients();
}