//
// Created by cym on 2022/3/8.
//
#include <jni.h>
#include"kissfft/kiss_fft.h"

extern "C"
JNIEXPORT jdoubleArray JNICALL
Java_cymheart_tau_utils_fft_FFT_dofft(JNIEnv *env, jobject thiz, jdoubleArray inArray,
                                      jint is_inverse) {
    if (inArray == NULL) {
        return NULL;
    }

    int n = env->GetArrayLength(inArray)/2;
    if (n < 1) {
        return NULL;
    }

    kiss_fft_cfg cfg = kiss_fft_alloc(n, is_inverse);

    double *inValues = env->GetDoubleArrayElements(inArray, 0);

    jdoubleArray outArray = env->NewDoubleArray(n*2);
    double *outValues = env->GetDoubleArrayElements(outArray, 0);

    kiss_fft(cfg, (kiss_fft_cpx*)inValues, (kiss_fft_cpx*)outValues);

    env->ReleaseDoubleArrayElements(outArray, outValues, 0);
    env->ReleaseDoubleArrayElements(inArray, inValues, 0);

    free(cfg);

    return outArray;
}