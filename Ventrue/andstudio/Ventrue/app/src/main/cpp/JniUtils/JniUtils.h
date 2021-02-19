#ifndef VENTRUE_JNIUTILS_H
#define VENTRUE_JNIUTILS_H

#include <jni.h>
#include<string>

std::string jstring2str(JNIEnv* env, jstring jstr);

#endif //VENTRUE_JNIUTILS_H
