//
// Created by cym on 2021/10/6.
//

#ifndef TAU_JNICREATEMIDIEVENT_H
#define TAU_JNICREATEMIDIEVENT_H

#include"../../JniUtils/JniUtils.h"
#include<Midi/MidiEvent.h>
using namespace  tau;

void SaveJMidiEventClassFields(JNIEnv *env, jclass midiEventClass, jfieldID* jMidiEventFields);
void SetJMidiEventFields(JNIEnv *env, jobject jMidiEvent, jfieldID* jMidiEventFields, MidiEvent* midiEvent);

void CreateJNoteOnEventClassInfo(JNIEnv *env);
void DeleteJNoteOnEventClassInfo(JNIEnv *env);
jobject CreateJNoteOnEvent(JNIEnv *env, NoteOnEvent* noteOnEvent);

void CreateJNoteOffEventClassInfo(JNIEnv *env);
void DeleteJNoteOffEventClassInfo(JNIEnv *env);
jobject CreateJNoteOffEvent(JNIEnv *env, NoteOffEvent* noteOffEvent);

#endif //TAU_JNICREATEMIDIEVENT_H
