#include <jni.h>
#include"dsignal/Filter.h"
using namespace dsignal;


extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_dsignal_Filter_ndkCreateFilter(JNIEnv *env, jclass clazz) {
    Filter* filter = new Filter();
    return (int64_t)filter;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Filter_ndkReleaseFilter(JNIEnv *env, jclass clazz, jlong ndk_filter) {
    Filter* filter = (Filter*)ndk_filter;
    delete filter;
}
