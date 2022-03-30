//
// Created by cym on 2021/9/17.
//
#include <jni.h>
#include"../../../JniUtils/JniUtils.h"
#include<dsignal/MorphLBHFilter.h>
using namespace dsignal;


extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_utils_dsignal_MorphLBHFilter_ndkCreateMorphLBHFilter(JNIEnv *env, jclass clazz) {
    MorphLBHFilter* eq = new MorphLBHFilter();
    return (int64_t)eq;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_utils_dsignal_MorphLBHFilter_ndkSetFreq(JNIEnv *env, jclass clazz, jlong ndk_filter,
                                                    jfloat freq_hz) {
    MorphLBHFilter* lbh = (MorphLBHFilter*)ndk_filter;
    lbh->SetFreq(freq_hz);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_utils_dsignal_MorphLBHFilter_ndkSetQ(JNIEnv *env, jclass clazz, jlong ndk_filter,
                                                 jfloat q) {
    MorphLBHFilter* lbh = (MorphLBHFilter*)ndk_filter;
    lbh->SetQ(q);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_utils_dsignal_MorphLBHFilter_ndkSetAlpha(JNIEnv *env, jclass clazz, jlong ndk_filter,
                                                       jfloat a) {
    MorphLBHFilter* lbh = (MorphLBHFilter*)ndk_filter;
    lbh->SetAlpha(a);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_utils_dsignal_MorphLBHFilter_ndkCompute(JNIEnv *env, jclass clazz, jlong ndk_filter) {
    MorphLBHFilter* lbh = (MorphLBHFilter*)ndk_filter;
    lbh->Compute();
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_cymheart_tau_utils_dsignal_MorphLBHFilter_ndkGetFilter(JNIEnv *env, jclass clazz, jlong ndk_filter)
{
    MorphLBHFilter* lbh = (MorphLBHFilter*)ndk_filter;
    vector<dsignal::Filter*> filters = lbh->GetFilters();
    return CreateJFilters(env, filters);
}





