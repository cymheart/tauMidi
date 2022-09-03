#include <jni.h>
#include<FX/Reverb.h>
#include"../../JniUtils/JniUtils.h"
#include <jni.h>

using namespace tauFX;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_effect_Reverb_ndkCreateReverb(JNIEnv *env, jclass clazz) {

    Reverb* reverb = new Reverb();
    return (int64_t)reverb;
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Reverb_ndkSetRoomSize(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                               jfloat value) {
    Reverb* reverb = (Reverb*)ndk_effect;
    reverb->SetRoomSize(value);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Reverb_ndkSetWidth(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                            jfloat value) {
    Reverb* reverb = (Reverb*)ndk_effect;
    reverb->SetWidth(value);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Reverb_ndkSetDamping(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                              jfloat value) {
    Reverb* reverb = (Reverb*)ndk_effect;
    reverb->SetDamping(value);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_effect_Reverb_ndkSetEffectMix(JNIEnv *env, jclass clazz, jlong ndk_effect,
                                                jfloat value) {
    Reverb* reverb = (Reverb*)ndk_effect;
    reverb->SetEffectMix(value);
}