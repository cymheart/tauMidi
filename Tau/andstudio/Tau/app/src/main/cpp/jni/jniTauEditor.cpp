//
// Created by cym on 2021/8/2.
//
#include"../JniUtils/JniUtils.h"
#include<Synth/Tau.h>
#include<Midi/MidiFile.h>
#include<Midi/MidiEvent.h>
#include<Synth/VirInstrument.h>

using namespace tau;

jobject CreateMidiEditor(JNIEnv *env, Tau* tau);
jobject CreateJMidiEvent(JNIEnv *env, MidiEvent* midiEvent, int trackIdx);

extern "C"
JNIEXPORT jstring JNICALL
Java_cymheart_tau_Tau_ndkGetMidiFilePath(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    string path = tau->GetMidiFilePath();
    jstring jpath = str2jstring(env, path.c_str());
    return jpath;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_cymheart_tau_Tau_ndkLoad(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                             jstring midifile, jboolean is_show_tips) {
    Tau* tau = (Tau*)ndk_tau;
    string _midifile = jstring2str(env, midifile);
    tau->Load(_midifile);

    if(!is_show_tips)
        return nullptr;

    return CreateMidiEditor(env, tau);

}

jclass jMidiEditorClass;
jclass jMidiTrackClass;
jclass jMidiEventClass;
jclass jNoteOnEventClass;
jfieldID midiEventsField;
jmethodID noteOnEventInitMethod;
jmethodID initMethod;
jmethodID midiTrackMethod;
jfieldID startSecField;
jfieldID endSecField;
jfieldID channelField;
jfieldID trackField;
jfieldID noteField;
jfieldID midiEventsAtChannelField;

jobject CreateMidiEditor(JNIEnv *env, Tau* tau)
{
    jMidiEditorClass = env->FindClass("cymheart/tau/MidiEditor");
    jMidiTrackClass = env->FindClass("cymheart/tau/midi/MidiTrack");
    jMidiEventClass = env->FindClass("cymheart/tau/midi/MidiEvent");
    jNoteOnEventClass = env->FindClass("cymheart/tau/midi/NoteOnEvent");

    //
    initMethod = env->GetMethodID(jMidiEditorClass, "<init>", "()V");
    midiTrackMethod = env->GetMethodID(jMidiTrackClass, "<init>", "()V");
    noteOnEventInitMethod = env->GetMethodID(jNoteOnEventClass, "<init>", "()V");

    //
    midiEventsField = env->GetFieldID(jMidiTrackClass, "midiEvents","[Lcymheart/tau/midi/MidiEvent;");
    startSecField = env->GetFieldID(jNoteOnEventClass, "startSec", "F");
    endSecField = env->GetFieldID(jNoteOnEventClass, "endSec", "F");
    channelField = env->GetFieldID(jNoteOnEventClass, "channel", "I");
    trackField = env->GetFieldID(jNoteOnEventClass, "track", "I");
    noteField = env->GetFieldID(jNoteOnEventClass, "note", "I");
    midiEventsAtChannelField = env->GetFieldID(jMidiTrackClass, "midiEventsAtChannel",
                                                        "[[Lcymheart/tau/midi/MidiEvent;");

    //
    MidiFile* midiFile = tau->GetMidiFile();
    vector<MidiTrack*>* midiTracks = midiFile->GetTrackList();


    //
    jobject jMidiEditor = env->NewObject(jMidiEditorClass, initMethod);
    jfieldID midiEditorEndSecField = env->GetFieldID(jMidiEditorClass, "endSec","F");
    env->SetFloatField(jMidiEditor, midiEditorEndSecField, tau->GetMidiEndSec());

    jfieldID filePathField = env->GetFieldID(jMidiEditorClass, "filePath", "Ljava/lang/String;");
    jstring jFilePath = str2jstring(env, tau->GetMidiFilePath().c_str());
    env->SetObjectField(jMidiEditor, filePathField, jFilePath);

    //
    jobjectArray jMidiTrackArray = (jobjectArray)env->NewObjectArray(midiTracks->size(), jMidiTrackClass, NULL);

    //
    for(int i=0; i<midiTracks->size(); i++)
    {
        // 创建一个新的对象
        jobject jMidiTrack = env->NewObject(jMidiTrackClass, midiTrackMethod);

        //
        list<MidiEvent*>* midiEventList = (*midiTracks)[i]->GetEventList();
        jobjectArray jMidiEventArray = (jobjectArray)env->NewObjectArray(midiEventList->size(), jMidiEventClass, NULL);
        jobject jNoteOnEvent;
        list<MidiEvent*>::iterator it = midiEventList->begin();
        list<MidiEvent*>::iterator end = midiEventList->end();
        for (int j = 0; it != end; it++, j++)
        {
            jNoteOnEvent = CreateJMidiEvent(env,  (NoteOnEvent*)(*it), i);
            env->SetObjectArrayElement(jMidiEventArray, j, jNoteOnEvent);
        }

        env->SetObjectField(jMidiTrack, midiEventsField, jMidiEventArray);


        //
        vector<MidiEvent*>* midiEvents = (*midiTracks)[i]->GetEventListAtChannel();
        jobjectArray jMidiEventsArray = (jobjectArray)env->GetObjectField(jMidiTrack, midiEventsAtChannelField);
        for(int j=0; j<16; j++)
        {
            if(midiEvents[j].size() == 0)
                continue;

            jobjectArray jMidiEvents = (jobjectArray)env->NewObjectArray(midiEvents[j].size(), jMidiEventClass, NULL);
             for(int k=0; k<midiEvents[j].size(); k++) {
                 jNoteOnEvent = CreateJMidiEvent(env, midiEvents[j][k], i);
                 env->SetObjectArrayElement(jMidiEvents, k, jNoteOnEvent);
             }
             env->SetObjectArrayElement(jMidiEventsArray, j, jMidiEvents);
        }

        env->SetObjectArrayElement(jMidiTrackArray, i, jMidiTrack);

    }

    jfieldID midiTracksField = env->GetFieldID(jMidiEditorClass, "midiTracks",
                                               "[Lcymheart/tau/midi/MidiTrack;");

    env->SetObjectField(jMidiEditor, midiTracksField, jMidiTrackArray);


    env->DeleteLocalRef(jMidiEventClass);
    env->DeleteLocalRef(jNoteOnEventClass);
    env->DeleteLocalRef(jMidiTrackClass);
    env->DeleteLocalRef(jMidiEditorClass);

    return jMidiEditor;
}

jobject CreateJMidiEvent(JNIEnv *env, MidiEvent* midiEvent, int trackIdx)
{
    if(midiEvent->type != MidiEventType::NoteOn)
        return NULL;

    NoteOnEvent* noteOnEvent = (NoteOnEvent*)midiEvent;

    jobject jNoteOnEvent = env->NewObject(jNoteOnEventClass, noteOnEventInitMethod);
    env->SetFloatField(jNoteOnEvent, startSecField, noteOnEvent->startSec);
    env->SetFloatField(jNoteOnEvent, endSecField, noteOnEvent->endSec);
    env->SetIntField(jNoteOnEvent, channelField, noteOnEvent->channel);
    env->SetIntField(jNoteOnEvent, trackField, trackIdx);
    env->SetIntField(jNoteOnEvent, noteField, noteOnEvent->note);

    return jNoteOnEvent;
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkPlay(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Play();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkPause(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkStop(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkRemove(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Remove();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkGoto(JNIEnv *env, jclass clazz, jlong ndk_tau, jfloat sec) {
    Tau* tau = (Tau*)ndk_tau;
    tau->Goto(sec);
}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_Tau_ndkGetState(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    EditorState state = tau->GetEditorState();
    return (int)state;
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkDisableTrack(JNIEnv *env, jclass clazz, jlong ndk_tau,
                                          jint track_idx) {
    Tau* tau = (Tau*)ndk_tau;
    tau->DisableTrack(track_idx);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkDisableAllTrack(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->DisableAllTrack();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkEnableTrack(JNIEnv *env, jclass clazz, jlong ndk_tau, jint track_idx) {
    Tau* tau = (Tau*)ndk_tau;
    tau->EnableTrack(track_idx);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_Tau_ndkEnableAllTrack(JNIEnv *env, jclass clazz, jlong ndk_tau) {
    Tau* tau = (Tau*)ndk_tau;
    tau->EnableAllTrack();
}
