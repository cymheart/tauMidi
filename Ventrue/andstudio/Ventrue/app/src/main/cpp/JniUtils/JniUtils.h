#ifndef VENTRUE_JNIUTILS_H
#define VENTRUE_JNIUTILS_H

#include <jni.h>
#include<string>

std::string jstring2str(JNIEnv* env, jstring jstr);
jstring str2jstring(JNIEnv* env, const char* pat);

int SetStringField(JNIEnv *jenv, jobject obj, const char* field, jstring str);


#endif //VENTRUE_JNIUTILS_H
