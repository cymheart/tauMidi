//
// Created by cym on 2021/8/11.
//
#include"../JniUtils/JniUtils.h"
#include<Synth/SoundFont.h>
#include<Midi/MidiFile.h>
#include<Midi/MidiEvent.h>
#include<Synth/Preset.h>

using namespace tau;

extern "C"
JNIEXPORT jlong JNICALL
Java_cymheart_tau_SoundFont_ndkCreateSoundFont(JNIEnv *env, jclass clazz) {
    SoundFont* ndksf = new SoundFont();
    return (int64_t)ndksf;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_SoundFont_ndkRelease(JNIEnv *env, jclass clazz, jlong nkd_sound_font) {
    SoundFont* sf = (SoundFont*)nkd_sound_font;
    DEL(sf);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_SoundFont_ndkParse(JNIEnv *env, jclass clazz, jlong nkd_sound_font,
                                     jstring format_name, jstring path) {
    SoundFont* sf = (SoundFont*)nkd_sound_font;
    string _formatName = jstring2str(env, format_name);
    string cpath = jstring2str(env, path);
    sf->Parse(_formatName, cpath);
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_cymheart_tau_SoundFont_ndkGetPresetList(JNIEnv *env, jclass clazz, jlong nkd_sound_font) {
    // TODO: implement ndkGetPresetList()
    SoundFont* sf = (SoundFont*)nkd_sound_font;
    PresetList* presetList = sf->GetPresetList();
    int sz = presetList->size();

    jclass jPresetClass = env->FindClass("cymheart/tau/Preset");
    // 获取类的构造函数，记住这里是调用无参的构造函数
    jmethodID id = env->GetMethodID(jPresetClass, "<init>", "()V");


    //env->PushLocalFrame(presetList->size());
    jobjectArray jPresetArray = (jobjectArray)env->NewObjectArray(presetList->size(), jPresetClass, NULL);

    jfieldID nameField = env->GetFieldID(jPresetClass, "name", "Ljava/lang/String;");
    jfieldID bankSelectMSBField = env->GetFieldID(jPresetClass, "bankSelectMSB", "I");
    jfieldID bankSelectLSBField = env->GetFieldID(jPresetClass, "bankSelectLSB", "I");
    jfieldID instrumentNumField = env->GetFieldID(jPresetClass, "instrumentNum", "I");

    for(int i=0; i<sz; i++) {
        // 创建一个新的对象
        jobject jPreset = env->NewObject(jPresetClass, id);

        const char* str = (*presetList)[i]->name.data();
        jstring jstr = str2jstring(env, str);
        env->SetObjectField(jPreset, nameField, jstr);
        env->SetIntField(jPreset, bankSelectMSBField, (*presetList)[i]->bankSelectMSB);
        env->SetIntField(jPreset, bankSelectLSBField, (*presetList)[i]->bankSelectLSB);
        env->SetIntField(jPreset, instrumentNumField, (*presetList)[i]->instrumentNum);

        //
        env->SetObjectArrayElement(jPresetArray, i, jPreset);
    }


    //env->PopLocalFrame(jPresetArray);
    env->DeleteLocalRef(jPresetClass);
    return jPresetArray;

}