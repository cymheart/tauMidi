//
// Created by cym on 2021/10/6.
//
#include"../../JniUtils/JniUtils.h"
#include<Synth/Editor/Editor.h>
#include<Synth/Editor/Track.h>
#include<Synth/Channel.h>
#include<Midi/MidiFile.h>
#include<Midi/MidiEvent.h>
#include<Synth/VirInstrument.h>
#include"jniCreateMidiEvent.h"

using namespace tau;


struct CBData{
    JNIEnv* env;
    jobject jeditor;
};


void LoadStart(Editor* editor);
void LoadCompleted(Editor* editor);
void Release(Editor* editor);
void CreateDatas(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor);


static JavaVM* g_vm = nullptr;
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv * env = nullptr;
    if(g_vm == nullptr){
        g_vm = vm;
    }

    if( g_vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK ){
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkInit(JNIEnv *env, jclass clazz, jobject jeditor,
                                        jlong ndk_editor) {

    Editor* editor = (Editor*)ndk_editor;

    editor->loadStartCallBack = LoadStart;
    editor->loadCompletedCallBack = LoadCompleted;
    editor->releaseCallBack = Release;


    CBData* cbData = new CBData();
    cbData->env = env;
    cbData->jeditor = env->NewGlobalRef(jeditor);
    editor->SetUserData((void*)cbData);
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_cymheart_tau_editor_Editor_ndkIsLoadCompleted(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->IsLoadCompleted();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkLoad(JNIEnv *env, jclass clazz,
                                        jobject jeditor, jlong ndk_editor,
                                        jstring midifile, jboolean is_wait_read_completed) {

    Editor* editor = (Editor*)ndk_editor;
    string _midifile = jstring2str(env, midifile);
    editor->Load(_midifile, is_wait_read_completed);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkPlay(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Play();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkPause(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkStop(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkRemove(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Remove();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkGoto(JNIEnv *env, jclass clazz, jlong ndk_editor, jdouble sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Goto(sec);
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_cymheart_tau_editor_Editor_ndkGetPlaySec(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetPlaySec();
}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_editor_Editor_ndkGetPlayState(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return (int)editor->GetState();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetSpeed(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                            jfloat speed) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetSpeed(speed);
}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_editor_Editor_ndkGetSampleStreamFreqSpectrums(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                                jint channel, jdoubleArray out_left,
                                                                jdoubleArray out_right) {
    Editor* editor = (Editor*)ndk_editor;
    double *left = env->GetDoubleArrayElements(out_left, 0);
    double *right = env->GetDoubleArrayElements(out_right, 0);
    int count = editor->GetSampleStreamFreqSpectrums(channel, left, right);
    return count;
}


extern "C"
JNIEXPORT jdouble JNICALL
Java_cymheart_tau_editor_Editor_ndkGetEndSec(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetEndSec();
}

void LoadStart(Editor* editor)
{
    CBData* cbData = (CBData*)editor->GetUserData();
    JNIEnv* env = cbData->env;

    jclass jclsProcess = env->GetObjectClass(cbData->jeditor);
    jmethodID method = env->GetMethodID(jclsProcess, "_JniLoadStart", "()V");
    env->CallVoidMethod(cbData->jeditor, method);
}


void LoadCompleted(Editor* editor)
{
    CBData* cbData = (CBData*)editor->GetUserData();
    jobject jeditorRef = cbData->jeditor;

    JNIEnv *env;
    g_vm->AttachCurrentThread(&env,NULL);
    jclass jclsProcess = env->GetObjectClass(jeditorRef);
    jmethodID mid = env->GetMethodID(jclsProcess, "_JniLoadCompleted", "()V");
    env->CallVoidMethod(jeditorRef, mid);

    g_vm->DetachCurrentThread();
}

void Release(Editor* editor)
{
    CBData* cbData = (CBData*)editor->GetUserData();
    editor->SetUserData(nullptr);
    jobject jeditorRef = cbData->jeditor;
    cbData->env->DeleteGlobalRef(jeditorRef);
    delete cbData;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkCreateDatas(JNIEnv *env, jclass clazz, jobject jeditor,
                                          jlong ndk_editor){

    CreateDatas(env,clazz, jeditor, (Editor*)ndk_editor);
}


void CreateDatas(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor)
{
    CreateJNoteOnEventClassInfo(env);
    CreateJNoteOffEventClassInfo(env);

    jclass jObjectClass = env->FindClass("java/lang/Object");

    //Editor
    jfieldID jEditorClassNdkTracksField = env->GetFieldID(jeditorClass, "_ndkTracks", "[Lcymheart/tau/editor/Track;");
    jfieldID jEditorClassNdkInstFragArrayField = env->GetFieldID(jeditorClass, "_ndkInstFragmentArray","[Ljava/lang/Object;");
    jfieldID jEditorClassEndSecField = env->GetFieldID(jeditorClass, "endSec","D");


    //Track
    jclass jTrackClass = env->FindClass("cymheart/tau/editor/Track");
    jmethodID jTrackClassInitMethod = env->GetMethodID(jTrackClass, "<init>", "()V");
    jfieldID jTrackClassChannelField = env->GetFieldID(jTrackClass, "channel","Lcymheart/tau/Channel;");
    jfieldID jTrackClassEndSecField = env->GetFieldID(jTrackClass, "endSec", "D");

    //Channel
    jclass jChannelClass = env->FindClass("cymheart/tau/Channel");
    jmethodID jChannelClassInitMethod = env->GetMethodID(jChannelClass, "<init>", "()V");
    jfieldID jChannelClassNumField = env->GetFieldID(jChannelClass, "channelNum", "I");
    jfieldID jChannelClassInstField = env->GetFieldID(jChannelClass, "inst","Lcymheart/tau/VirInstrument;");

    //VirInstrument
    jclass jVirInstClass = env->FindClass("cymheart/tau/VirInstrument");
    jmethodID jVirInstClassInitMethod = env->GetMethodID(jVirInstClass, "<init>", "()V");
    jfieldID jVirInstPtrField = env->GetFieldID(jVirInstClass, "ndkVirInstrument", "J");

    //InstFragment
    jclass jInstFragmentClass = env->FindClass("cymheart/tau/editor/InstFragment");
    jmethodID jInstFragmentClassInitMethod = env->GetMethodID(jInstFragmentClass, "<init>", "()V");
    jfieldID jInstFragmentClassNdKMidiEventField = env->GetFieldID(jInstFragmentClass, "_ndkMidiEvent", "[Lcymheart/tau/midi/MidiEvent;");
    jfieldID jInstFragmentClassStartTickField = env->GetFieldID(jInstFragmentClass, "startTick", "I");
    jfieldID jInstFragmentClassStartSecField = env->GetFieldID(jInstFragmentClass, "startSec", "D");
    jfieldID jInstFragmentClassEndSecField = env->GetFieldID(jInstFragmentClass, "endSec", "D");

    //MidiEvent
    jclass jMidiEventClass = env->FindClass("cymheart/tau/midi/MidiEvent");
    jfieldID jMidiEventClassInstFragField = env->GetFieldID(jMidiEventClass, "instFragment", "Lcymheart/tau/editor/InstFragment;");

    //
    const vector<Track*>& tracks = editor->GetTracks();
    jobjectArray jTracks = (jobjectArray)env->NewObjectArray(tracks.size(), jTrackClass, NULL);
    jobjectArray jInstFragmentArrays = (jobjectArray)env->NewObjectArray(tracks.size(), jObjectClass, NULL);

    for(int i=0; i<tracks.size(); i++)
    {
        jobject jTrack = env->NewObject(jTrackClass, jTrackClassInitMethod);

        Channel* channel = tracks[i]->GetChannel();
        jobject jChannel = env->NewObject(jChannelClass, jChannelClassInitMethod);
        env->SetIntField(jChannel, jChannelClassNumField, channel->GetChannelNum());

        jobject jVirInst = env->NewObject(jVirInstClass, jVirInstClassInitMethod);
        env->SetLongField(jVirInst, jVirInstPtrField, (int64_t)channel->GetVirInstrument());
        env->SetObjectField(jChannel, jChannelClassInstField, jVirInst);
        env->DeleteLocalRef(jVirInst);

        env->SetDoubleField(jTrack, jTrackClassEndSecField, tracks[i]->GetEndSec());
        env->SetObjectField(jTrack, jTrackClassChannelField, jChannel);
        env->DeleteLocalRef(jChannel);

        env->SetObjectArrayElement(jTracks, i, jTrack);

        //
        const vector<list<InstFragment*>*>& instFragmentArray = tracks[i]->GetInstFragmentBranchs();
        jobjectArray jInstFragmentArray = (jobjectArray)env->NewObjectArray(instFragmentArray.size(), jObjectClass, NULL);
        for(int j=0; j<instFragmentArray.size(); j++)
        {
            int k = 0;
            jobjectArray jInstFragments = (jobjectArray)env->NewObjectArray(instFragmentArray[j]->size(), jInstFragmentClass, NULL);
            for(auto it = instFragmentArray[j]->begin(); it != instFragmentArray[j]->end(); it++)
            {
                InstFragment* instFragment = *it;
                jobject jInstFragment = env->NewObject(jInstFragmentClass, jInstFragmentClassInitMethod);
                env->SetIntField(jInstFragment, jInstFragmentClassStartTickField, instFragment->GetStartTick());
                env->SetDoubleField(jInstFragment, jInstFragmentClassStartSecField, instFragment->GetStartSec());
                env->SetDoubleField(jInstFragment, jInstFragmentClassEndSecField, instFragment->GetEndSec());

                //
                int m = 0;
                LinkedList<MidiEvent*>& midiEvents =  (*it)->GetMidiEvents();
                jobjectArray jMidiEvents = (jobjectArray)env->NewObjectArray(midiEvents.Size(), jMidiEventClass, NULL);
                for(auto node = midiEvents.GetHeadNode(); node; node = node->next)
                {
                    jobject jMidiEvent;
                    MidiEvent* midiEvent = node->elem;


                    switch (midiEvent->type) {
                        case MidiEventType::NoteOn:
                        {
                            jMidiEvent = CreateJNoteOnEvent(env, (NoteOnEvent*)midiEvent);
                        }
                        break;

                        case MidiEventType::NoteOff:
                        {
                            jMidiEvent = CreateJNoteOffEvent(env, (NoteOffEvent*)midiEvent);
                        }
                        break;

                        default:
                            continue;
                    }

                    env->SetObjectField(jMidiEvent, jMidiEventClassInstFragField, jInstFragment);

                    env->SetObjectArrayElement(jMidiEvents, m++, jMidiEvent);
                    env->DeleteLocalRef(jMidiEvent);
                }

                env->SetObjectField(jInstFragment, jInstFragmentClassNdKMidiEventField, jMidiEvents);
                env->DeleteLocalRef(jMidiEvents);

                env->SetObjectArrayElement(jInstFragments, k++, jInstFragment);
                env->DeleteLocalRef(jInstFragment);
            }

            env->SetObjectArrayElement(jInstFragmentArray, j, jInstFragments);
            env->DeleteLocalRef(jInstFragments);
        }

        env->SetObjectArrayElement(jInstFragmentArrays, i, jInstFragmentArray);
        env->DeleteLocalRef(jInstFragmentArray);


    }

    //
    env->SetDoubleField(jeditor, jEditorClassEndSecField, editor->GetEndSec());
    env->SetObjectField(jeditor, jEditorClassNdkTracksField, jTracks);
    env->SetObjectField(jeditor, jEditorClassNdkInstFragArrayField, jInstFragmentArrays);


    env->DeleteLocalRef(jTracks);
    env->DeleteLocalRef(jInstFragmentArrays);

    //
    DeleteJNoteOnEventClassInfo(env);
    DeleteJNoteOffEventClassInfo(env);

    env->DeleteLocalRef(jObjectClass);
    env->DeleteLocalRef(jTrackClass);
    env->DeleteLocalRef(jChannelClass);
    env->DeleteLocalRef(jVirInstClass);
    env->DeleteLocalRef(jInstFragmentClass);
    env->DeleteLocalRef(jMidiEventClass);
}


