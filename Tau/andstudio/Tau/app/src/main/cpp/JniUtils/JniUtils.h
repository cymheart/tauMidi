#ifndef VENTRUE_JNIUTILS_H
#define VENTRUE_JNIUTILS_H

#include <jni.h>
#include <string>
#include <dsignal/Filter.h>

#define  LOGw(...) __android_log_print(ANDROID_LOG_ERROR,"wtest",__VA_ARGS__)

std::string jstring2str(JNIEnv* env, jstring jstr);
jstring str2jstring(JNIEnv* env, const char* pat);

int SetStringField(JNIEnv *jenv, jobject obj, const char* field, jstring str);
jobjectArray CreateJFilters(JNIEnv *env, vector<dsignal::Filter*>& filters);

#endif //VENTRUE_JNIUTILS_H
