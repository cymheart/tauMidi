
#include <jni.h>
#include"dsignal/Bode.h"
using namespace dsignal;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_dsignal_Bode_ndkCreateBode(JNIEnv *env, jclass clazz) {
    Bode* bode = new Bode();
    return (int64_t)bode;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkAddFilter(JNIEnv *env, jclass clazz, jlong ndk_bode, jlong ndk_filter) {
    Bode* bode = (Bode*)ndk_bode;
    bode->AddFilter((Filter*)ndk_filter);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetFilterBetweenMorphPlot(JNIEnv *env, jclass clazz,
                                                            jlong ndk_bode,
                                                            jboolean is_morph_plot) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetFilterBetwwenMorphPlot(is_morph_plot);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetSampleFreq(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloat sfreq) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetSampleFreq(sfreq);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetPlotAreaWidth(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloat width) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetPlotAreaWidth(width);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetPlotAreaHeight(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloat height) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetPlotAreaHeight(height);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetPlotFreqAxisStart(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloat start_freq_hz) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetPlotFreqAxisStart(start_freq_hz);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetPlotFreqAxisEnd(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloat end_freq_hz) {
    Bode *bode = (Bode *) ndk_bode;
    bode->SetPlotFreqAxisEnd(end_freq_hz);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetRulerFreqs(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloatArray ruler_freqs) {
    Bode* bode = (Bode*)ndk_bode;
    jfloat* rulerFreqs = (jfloat*)env->GetFloatArrayElements(ruler_freqs, 0);
    jsize rulerFreqsSize = env->GetArrayLength(ruler_freqs);
    bode->SetRulerFreqs(rulerFreqs, rulerFreqsSize);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetRulerGainDBs(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloatArray rule_gain_db) {
    Bode* bode = (Bode*)ndk_bode;
    jfloat* gainDBs = (jfloat*)env->GetFloatArrayElements(rule_gain_db, 0);
    jsize rulerGainsSize = env->GetArrayLength(rule_gain_db);
    bode->SetRulerGainDBs(gainDBs, rulerGainsSize);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetPlotGainDbRange(JNIEnv *env, jclass clazz, jlong ndk_bode, jfloat gain_range_db) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetPlotGainDbRange(gain_range_db);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkSetGainOffsetDB(JNIEnv *env, jclass clazz, jlong ndk_bode,
                                                  jfloat offset_gain_db) {
    Bode* bode = (Bode*)ndk_bode;
    bode->SetGainOffsetDB(offset_gain_db);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_dsignal_Bode_ndkCompute(JNIEnv *env, jclass clazz, jlong ndk_bode) {
    Bode* bode = (Bode*)ndk_bode;
    bode->Compute();
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_cymheart_tau_dsignal_Bode_ndkGetPlotAreaWidth(JNIEnv *env, jclass clazz, jlong ndk_bode) {
    Bode* bode = (Bode*)ndk_bode;
    return bode->GetPlotAreaWidth();
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_cymheart_tau_dsignal_Bode_ndkGetPlotAreaHeight(JNIEnv *env, jclass clazz, jlong ndk_bode) {
    Bode* bode = (Bode*)ndk_bode;
    return bode->GetPlotAreaHeigth();
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_cymheart_tau_dsignal_Bode_ndkGetPlotFreqAxisPos(JNIEnv *env, jclass clazz, jlong ndk_bode) {
    Bode* bode = (Bode*)ndk_bode;
    int count = bode->GetPlotAxisPosCount();
    float* posData = bode->GetPlotFreqAxisPos();

    jfloatArray jPosData = env->NewFloatArray(count);
    env->SetFloatArrayRegion(jPosData, 0, count, posData);
    return jPosData;
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_cymheart_tau_dsignal_Bode_ndkGetPlotGainAxisPos(JNIEnv *env, jclass clazz, jlong ndk_bode) {
    Bode* bode = (Bode*)ndk_bode;
    int count = bode->GetPlotAxisPosCount();
    float* posData = bode->GetPlotGainAxisPos();

    jfloatArray jPosData = env->NewFloatArray(count);
    env->SetFloatArrayRegion(jPosData, 0, count, posData);
    return jPosData;
}


extern "C"
JNIEXPORT jfloatArray JNICALL
Java_cymheart_tau_dsignal_Bode_ndkGetPlotRulerFreqsPos(JNIEnv *env, jclass clazz,jlong ndk_bode) {
    Bode* bode = (Bode*)ndk_bode;
    vector<float> posData = bode->GetPlotRulerFreqsPos();

    jfloatArray jPosData = env->NewFloatArray(posData.size());
    env->SetFloatArrayRegion(jPosData, 0, posData.size(), posData.data());
    return jPosData;
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_cymheart_tau_dsignal_Bode_ndkGetPlotRulerGainDBsPos(JNIEnv *env, jclass clazz,jlong ndk_bode) {

    Bode* bode = (Bode*)ndk_bode;
    vector<float> posData = bode->GetPlotRulerGainDBsPos();

    jfloatArray jPosData = env->NewFloatArray(posData.size());
    env->SetFloatArrayRegion(jPosData, 0, posData.size(), posData.data());
    return jPosData;
}