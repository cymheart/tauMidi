//
// Created by cym on 2023/7/23.
//

#include <jni.h>
#include<FX/TauEffect.h>
#include"../../JniUtils/JniUtils.h"
#include <jni.h>

using namespace tauFX;

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_Effect_ndkLockData(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    TauEffect* effect = (TauEffect*)ndk_effect;
    effect->LockData();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_FX_Effect_ndkUnLockData(JNIEnv *env, jclass clazz, jlong ndk_effect) {
    TauEffect* effect = (TauEffect*)ndk_effect;
    effect->UnLockData();
}


extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_FX_Effect_ndkRelease(JNIEnv *env, jclass clazz, jobject effect) {
    jfieldID jFiledNdkEffect = env->GetFieldID(clazz, "ndkEffect","L");
    long ndk_effect = env->GetLongField(effect, jFiledNdkEffect);
    env->SetLongField(effect, jFiledNdkEffect, 0);
    delete (TauEffect*)ndk_effect;
}

