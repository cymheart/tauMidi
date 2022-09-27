//
// Created by cym on 2021/10/6.
//
#include"jniCreateMidiEvent.h"

const int jMidiEventStartTick = 0;
const int jMidiEventStartSec = 1;
const int jMidiEventEndSec= 2;
const int jMidiEventChannel = 3;
const int jMidiEventNdkPtr = 4;
void SaveJMidiEventClassFields(JNIEnv *env,  jclass midiEventClass, jfieldID* jMidiEventFields)
{
    jMidiEventFields[jMidiEventStartTick] = env->GetFieldID(midiEventClass, "startTick", "I");
    jMidiEventFields[jMidiEventStartSec] = env->GetFieldID(midiEventClass, "startSec", "F");
    jMidiEventFields[jMidiEventEndSec] = env->GetFieldID(midiEventClass, "endSec", "F");
    jMidiEventFields[jMidiEventChannel] = env->GetFieldID(midiEventClass, "channel", "I");
    jMidiEventFields[jMidiEventNdkPtr] = env->GetFieldID(midiEventClass, "ndkMidiEvent", "J");
}

void SetJMidiEventFields(JNIEnv *env, jobject jMidiEvent, jfieldID* jMidiEventFields, MidiEvent* midiEvent)
{
    env->SetIntField(jMidiEvent, jMidiEventFields[jMidiEventStartTick], (int)midiEvent->startTick);
    env->SetFloatField(jMidiEvent, jMidiEventFields[jMidiEventStartSec], midiEvent->startSec);
    env->SetFloatField(jMidiEvent, jMidiEventFields[jMidiEventEndSec], midiEvent->endSec);
    env->SetIntField(jMidiEvent, jMidiEventFields[jMidiEventChannel], midiEvent->channel);
    env->SetLongField(jMidiEvent, jMidiEventFields[jMidiEventNdkPtr], (long)midiEvent);
}


//
jclass jNoteOnEventClass;
jmethodID jNoteOnEventClassInitMethod;
const int jNoteOnEventOffNoteNdkPtr = 5;
const int jNoteOnEventNote = 6;
jfieldID jNoteOnEventFields[7];
void CreateJNoteOnEventClassInfo(JNIEnv *env)
{
    jNoteOnEventClass =  env->FindClass("cymheart/tau/midi/NoteOnEvent");
    jNoteOnEventClassInitMethod = env->GetMethodID(jNoteOnEventClass, "<init>", "()V");

    //
    SaveJMidiEventClassFields(env, jNoteOnEventClass, jNoteOnEventFields);
    jNoteOnEventFields[jNoteOnEventOffNoteNdkPtr] = env->GetFieldID(jNoteOnEventClass, "ndkNoteOffEvent", "J");
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
    env->SetIntField(jNoteOnEvent, jNoteOnEventFields[jNoteOnEventOffNoteNdkPtr], (long)noteOnEvent->noteOffEvent);
    env->SetIntField(jNoteOnEvent, jNoteOnEventFields[jNoteOnEventNote], noteOnEvent->note);
    return jNoteOnEvent;
}


//
jclass jNoteOffEventClass;
jmethodID jNoteOffEventClassInitMethod;
const int jNoteOffEventNote = 5;
jfieldID jNoteOffEventFields[6];
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
