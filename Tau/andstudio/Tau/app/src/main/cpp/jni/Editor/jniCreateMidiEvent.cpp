//
// Created by cym on 2021/10/6.
//
#include"jniCreateMidiEvent.h"

const int jMidiEventStartTick = 0;
const int jMidiEventStartSec = 1;
const int jMidiEventEndSec= 2;
const int jMidiEventChannel = 3;
void SaveJMidiEventClassFields(JNIEnv *env,  jclass midiEventClass, jfieldID* jMidiEventFields)
{
    jMidiEventFields[jMidiEventStartTick] = env->GetFieldID(midiEventClass, "startTick", "I");
    jMidiEventFields[jMidiEventStartSec] = env->GetFieldID(midiEventClass, "startSec", "F");
    jMidiEventFields[jMidiEventEndSec] = env->GetFieldID(midiEventClass, "endSec", "F");
    jMidiEventFields[jMidiEventChannel] = env->GetFieldID(midiEventClass, "channel", "I");
}

void SetJMidiEventFields(JNIEnv *env, jobject jMidiEvent, jfieldID* jMidiEventFields, MidiEvent* midiEvent)
{
    env->SetIntField(jMidiEvent, jMidiEventFields[jMidiEventStartTick], (int)midiEvent->startTick);
    env->SetFloatField(jMidiEvent, jMidiEventFields[jMidiEventStartSec], midiEvent->startSec);
    env->SetFloatField(jMidiEvent, jMidiEventFields[jMidiEventEndSec], midiEvent->endSec);
    env->SetIntField(jMidiEvent, jMidiEventFields[jMidiEventChannel], midiEvent->channel);
}


//
jclass jNoteOnEventClass;
jmethodID jNoteOnEventClassInitMethod;
const int jNoteOnEventNote = 4;
jfieldID jNoteOnEventFields[5];
void CreateJNoteOnEventClassInfo(JNIEnv *env)
{
    jNoteOnEventClass =  env->FindClass("cymheart/tau/midi/NoteOnEvent");
    jNoteOnEventClassInitMethod = env->GetMethodID(jNoteOnEventClass, "<init>", "()V");
    SaveJMidiEventClassFields(env, jNoteOnEventClass, jNoteOnEventFields);
    jNoteOnEventFields[jNoteOnEventNote] = env->GetFieldID(jNoteOnEventClass, "note", "I");
}

void DeleteJNoteOnEventClassInfo(JNIEnv *env)
{
    env->DeleteLocalRef(jNoteOnEventClass);
}

jobject CreateJNoteOnEvent(JNIEnv *env, NoteOnEvent* noteOnEvent)
{
    jobject jNoteOnEvent = env->NewObject(jNoteOnEventClass, jNoteOnEventClassInitMethod);
    SetJMidiEventFields(env, jNoteOnEvent, jNoteOnEventFields, noteOnEvent);
    env->SetIntField(jNoteOnEvent, jNoteOnEventFields[jNoteOnEventNote], noteOnEvent->note);
    return jNoteOnEvent;
}


//
jclass jNoteOffEventClass;
jmethodID jNoteOffEventClassInitMethod;
const int jNoteOffEventNote = 4;
jfieldID jNoteOffEventFields[5];
void CreateJNoteOffEventClassInfo(JNIEnv *env)
{
    jNoteOffEventClass = env->FindClass("cymheart/tau/midi/NoteOffEvent");
    jNoteOffEventClassInitMethod = env->GetMethodID(jNoteOffEventClass, "<init>", "()V");
    SaveJMidiEventClassFields(env, jNoteOffEventClass, jNoteOffEventFields);
    jNoteOffEventFields[jNoteOffEventNote] = env->GetFieldID(jNoteOffEventClass, "note", "I");
}

void DeleteJNoteOffEventClassInfo(JNIEnv *env)
{
    env->DeleteLocalRef(jNoteOffEventClass);
}

jobject CreateJNoteOffEvent(JNIEnv *env, NoteOffEvent* noteOffEvent) {
    jobject jNoteOffEvent = env->NewObject(jNoteOffEventClass, jNoteOffEventClassInitMethod);
    SetJMidiEventFields(env, jNoteOffEvent, jNoteOffEventFields, noteOffEvent);
    env->SetIntField(jNoteOffEvent, jNoteOffEventFields[jNoteOffEventNote], noteOffEvent->note);

    return jNoteOffEvent;
}
