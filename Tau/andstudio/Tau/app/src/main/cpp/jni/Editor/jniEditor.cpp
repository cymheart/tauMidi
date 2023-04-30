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
#include<Synth/Editor/MeasureInfo.h>

using namespace tau;


struct CBData{
    JNIEnv* env;
    jobject jeditor;
};


void LoadStart(Editor* editor);
void LoadCompleted(Editor* editor);
void Release(Editor* editor);
void CreateDatas(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor);
void CreateTracks(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor);
void CreateMidiMarkers(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor);
void CreateMeasureInfo(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor);

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
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetPlayType(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                               jint play_type) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetPlayType((MidiEventPlayType)play_type);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkEnterWaitPlayMode(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->EnterWaitPlayMode();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkEnterMuteMode(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->EnterMuteMode();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkLeavePlayMode(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->LeavePlayMode();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetTrackPlayType(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                    jint track_idx, jint play_type) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetTrackPlayType(track_idx, (MidiEventPlayType)play_type);
}


extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_editor_Editor_ndkGetCurtNeedOnKeyTrackIdx(JNIEnv *env, jclass clazz,
                                                            jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetCurtNeedOnKeyTrackIdx();
}
extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_editor_Editor_ndkGetCurtNeedOnKeyVel(JNIEnv *env, jclass clazz,
                                                       jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetCurtNeedOnKeyVel();
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetExcludeNeedWaitKey(JNIEnv *env, jclass clazz,
                                                         jlong ndk_editor, jint key) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetExcludeNeedWaitKey(key);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetIncludeNeedWaitKey(JNIEnv *env, jclass clazz,
                                                            jlong ndk_editor, jint key) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetIncludeNeedWaitKey(key);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetLateNoteSec(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                  jfloat sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetLateNoteSec(sec);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkOnKeySignal(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                               jint key) {
    Editor* editor = (Editor*)ndk_editor;
    editor->OnKeySignal(key);
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkOffKeySignal(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                 jint key) {
    Editor* editor = (Editor*)ndk_editor;
    editor->OffKeySignal(key);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_cymheart_tau_editor_Editor_ndkIsLoadCompleted(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->IsLoadCompleted();
}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetInitStartPlaySec(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                       jdouble sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetInitStartPlaySec(sec);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetNoteSoundStartSec(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                        jdouble sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetNoteSoundStartSec(sec);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetNoteSoundEndSec(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                      jdouble sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetNoteSoundEndSec(sec);
}



extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkWait(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Wait();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkContinue(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->Continue();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_cymheart_tau_editor_Editor_ndkIsWait(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->IsWait();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_cymheart_tau_editor_Editor_ndkIsWaitKey(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                             jint key) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->IsWaitKey(key);
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_cymheart_tau_editor_Editor_ndkHavWaitKey(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->HavWaitKey();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkOnWaitKeysSignal(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->OnWaitKeysSignal();
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetSimpleModePlayWhiteKeyCount(JNIEnv *env, jclass clazz,
                                                                  jlong ndk_editor, jint count) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetSimpleModePlayWhiteKeyCount(count);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetMergeSimpleSrcNoteLimitSec(JNIEnv *env, jclass clazz,
                                                                 jlong ndk_editor, jfloat sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetMergeSimpleSrcNoteLimitSec(sec);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkSetMergeSimpleDestNoteLimitSec(JNIEnv *env, jclass clazz,
                                                                  jlong ndk_editor, jfloat sec) {
    Editor* editor = (Editor*)ndk_editor;
    editor->SetMergeSimpleDestNoteLimitSec(sec);
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
Java_cymheart_tau_editor_Editor_ndkOnKeyAtTrack(JNIEnv *env, jclass clazz, jlong ndk_editor, jint key,
                                      jfloat velocity, jint track_idx, jint id) {
    Editor* editor = (Editor*)ndk_editor;
    editor->OnKey(key, velocity, track_idx, id);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkOffKeyAtTrack(JNIEnv *env, jclass clazz, jlong ndk_editor, jint key,
                                       jfloat velocity, jint track_idx, jint id) {
    Editor* editor = (Editor*)ndk_editor;
    editor->OffKey(key, velocity, track_idx, id);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkOffAllKeys(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->OffAllKeys();
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
JNIEXPORT jint JNICALL
Java_cymheart_tau_editor_Editor_ndkGetSecTickCount(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                   jdouble sec) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetSecTickCount(sec);
}


extern "C"
JNIEXPORT jfloat JNICALL
Java_cymheart_tau_editor_Editor_ndkGetCurtBPM(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetCurtBPM();
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_cymheart_tau_editor_Editor_ndkGetTickSec(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                              jint tick) {
    Editor* editor = (Editor*)ndk_editor;
    return editor->GetTickSec(tick);
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
    return (int)editor->GetPlayState();
}

extern "C"
JNIEXPORT jint JNICALL
Java_cymheart_tau_editor_Editor_ndkGetCacheState(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    return (int)editor->GetCacheState();
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


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkDisableTrack(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                                jint track_idx) {
    Editor* editor = (Editor*)ndk_editor;
    editor->DisableTrack(track_idx);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkDisableAllTrack(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->DisableAllTrack();
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkEnableTrack(JNIEnv *env, jclass clazz, jlong ndk_editor,
                                               jint track_idx) {
    Editor* editor = (Editor*)ndk_editor;
    editor->EnableTrack(track_idx);
}
extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkEnableAllTrack(JNIEnv *env, jclass clazz, jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->DisableAllTrack();
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
    CreateTracks(env, jeditorClass, jeditor, editor);
    CreateMidiMarkers(env, jeditorClass, jeditor, editor);
    CreateMeasureInfo(env, jeditorClass, jeditor, editor);
}


void CreateTracks(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor)
{
    CreateJNoteOnEventClassInfo(env);
    //CreateJNoteOffEventClassInfo(env);

    jclass jObjectClass = env->FindClass("java/lang/Object");

    //Editor
    jfieldID jEditorClassTracksField = env->GetFieldID(jeditorClass, "tracks", "[Lcymheart/tau/editor/Track;");
    jfieldID jEditorClassEndSecField = env->GetFieldID(jeditorClass, "endSec","D");


    //Track
    jclass jTrackClass = env->FindClass("cymheart/tau/editor/Track");
    jmethodID jTrackClassInitMethod = env->GetMethodID(jTrackClass, "<init>", "()V");
    jfieldID jTrackClassChannelField = env->GetFieldID(jTrackClass, "channel","Lcymheart/tau/Channel;");
    jfieldID jTrackClassEndSecField = env->GetFieldID(jTrackClass, "endSec", "D");
    jfieldID jTrackClassPlayTypeField = env->GetFieldID(jTrackClass, "playType", "I");
    jfieldID jTrackClassIdxField = env->GetFieldID(jTrackClass, "idx", "I");
    jfieldID jTrackNoteOnEventsField = env->GetFieldID(jTrackClass, "noteOnEvents","[Lcymheart/tau/midi/NoteOnEvent;");

    //Channel
    jclass jChannelClass = env->FindClass("cymheart/tau/Channel");
    jmethodID jChannelClassInitMethod = env->GetMethodID(jChannelClass, "<init>", "()V");
    jfieldID jChannelClassNumField = env->GetFieldID(jChannelClass, "channelNum", "I");
    //jfieldID jChannelClassInstField = env->GetFieldID(jChannelClass, "inst","Lcymheart/tau/VirInstrument;");

    //VirInstrument
    jclass jVirInstClass = env->FindClass("cymheart/tau/VirInstrument");
    jmethodID jVirInstClassInitMethod = env->GetMethodID(jVirInstClass, "<init>", "()V");

    //MidiEvent
    jclass jNoteOnEventClass = env->FindClass("cymheart/tau/midi/NoteOnEvent");
    jfieldID jMidiEventClassIndexField = env->GetFieldID(jNoteOnEventClass, "index", "I");
    jfieldID jMidiEventClassTrackIdxField = env->GetFieldID(jNoteOnEventClass, "trackIdx", "I");
    jfieldID jMidiEventClassTrackField = env->GetFieldID(jNoteOnEventClass, "track", "Lcymheart/tau/editor/Track;");

    //
    const vector<Track*>& tracks = editor->GetTracks();
    jobjectArray jTracks = (jobjectArray)env->NewObjectArray(tracks.size(), jTrackClass, NULL);

    for(int i=0; i<tracks.size(); i++)
    {
        jobject jTrack = env->NewObject(jTrackClass, jTrackClassInitMethod);

        Channel* channel = tracks[i]->GetChannel();
        jobject jChannel = env->NewObject(jChannelClass, jChannelClassInitMethod);
        env->SetIntField(jChannel, jChannelClassNumField, channel->GetChannelNum());

        jobject jVirInst = env->NewObject(jVirInstClass, jVirInstClassInitMethod);
      //  env->SetLongField(jVirInst, jVirInstPtrField, (int64_t)channel->GetVirInstrument());
      //  env->SetObjectField(jChannel, jChannelClassInstField, jVirInst);
        env->DeleteLocalRef(jVirInst);

        env->SetIntField(jTrack, jTrackClassIdxField, i);
        env->SetDoubleField(jTrack, jTrackClassEndSecField, tracks[i]->GetEndSec());
        env->SetIntField(jTrack, jTrackClassPlayTypeField, (int)(tracks[i]->GetPlayType()));
        env->SetObjectField(jTrack, jTrackClassChannelField, jChannel);
        env->DeleteLocalRef(jChannel);

        env->SetObjectArrayElement(jTracks, i, jTrack);

        //计算每个轨道的midiEvent数量
        int midiEventCount = 0;
        const vector<list<InstFragment*>*>& instFragmentArray = tracks[i]->GetInstFragmentBranchs();
        for(int j=0; j<instFragmentArray.size(); j++)
        {
            for (auto it = instFragmentArray[j]->begin(); it != instFragmentArray[j]->end(); it++) {
                LinkedList<MidiEvent*>& midiEvents = (*it)->GetMidiEvents();
                for(auto node = midiEvents.GetHeadNode(); node; node = node->next)
                {
                    MidiEvent* midiEvent = node->elem;
                    if(midiEvent->type == MidiEventType::NoteOn)
                        midiEventCount++;
                }
            }
        }


        jobjectArray jNoteOnEvents = (jobjectArray)env->NewObjectArray(midiEventCount, jNoteOnEventClass, NULL);

        //
        for(int j=0; j<instFragmentArray.size(); j++)
        {
            for(auto it = instFragmentArray[j]->begin(); it != instFragmentArray[j]->end(); it++)
            {
                int m = 0;
                LinkedList<MidiEvent*>& midiEvents = (*it)->GetMidiEvents();

                int midiIndex = -1;
                for(auto node = midiEvents.GetHeadNode(); node; node = node->next)
                {
                    jobject jNoteOnEv;
                    MidiEvent* midiEvent = node->elem;
                    if(midiEvent->type != MidiEventType::NoteOn)
                            continue;

                    midiIndex++;
                    jNoteOnEv = CreateJNoteOnEvent(env, (NoteOnEvent*)midiEvent);


                    //
                    env->SetIntField(jNoteOnEv, jMidiEventClassIndexField, midiIndex);
                    env->SetIntField(jNoteOnEv, jMidiEventClassTrackIdxField, i);
                    env->SetObjectField(jNoteOnEv, jMidiEventClassTrackField, jTrack);

                    env->SetObjectArrayElement(jNoteOnEvents, m++, jNoteOnEv);
                    env->DeleteLocalRef(jNoteOnEv);
                }
            }
        }

        env->SetObjectField(jTrack, jTrackNoteOnEventsField, jNoteOnEvents);
        env->DeleteLocalRef(jNoteOnEvents);

    }

    //
    env->SetDoubleField(jeditor, jEditorClassEndSecField, editor->GetEndSec());
    env->SetObjectField(jeditor, jEditorClassTracksField, jTracks);

    //
    env->DeleteLocalRef(jTracks);

    //
    DeleteJNoteOnEventClassInfo(env);
   // DeleteJNoteOffEventClassInfo(env);

    env->DeleteLocalRef(jObjectClass);
    env->DeleteLocalRef(jTrackClass);
    env->DeleteLocalRef(jChannelClass);
    env->DeleteLocalRef(jVirInstClass);
    env->DeleteLocalRef(jNoteOnEventClass);
}

extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_editor_Editor_ndkCreateSimpleModeTrack(JNIEnv *env, jclass clazz, jobject jeditor,
                                                         jlong ndk_editor) {
    Editor* editor = (Editor*)ndk_editor;
    editor->CreateSimpleModeTrack();

    //
    CreateJNoteOnEventClassInfo(env);

    //editor
    jfieldID jEditorClassSimpleModeTrackNotesField = env->GetFieldID(clazz, "simpleModeTrackNotes",
                                                                     "[Lcymheart/tau/midi/NoteOnEvent;");

    jfieldID jEditorClassWhiteKeyCountField = env->GetFieldID(clazz, "simpleModePlayWhiteKeyCount","I");
    jfieldID jEditorClassMegreSrcNoteSecField = env->GetFieldID(clazz, "mergeSimpleSrcNoteLimitSec","F");
    jfieldID jEditorClassMegreDestNoteSecField = env->GetFieldID(clazz, "mergeSimpleDestNoteLimitSec","F");


    //NoteOnEvent
    jclass jNoteOnEventClass = env->FindClass("cymheart/tau/midi/NoteOnEvent");
    jfieldID jMidiEventClassIndexField = env->GetFieldID(jNoteOnEventClass, "index", "I");
    jfieldID jMidiEventClassTrackIdxField = env->GetFieldID(jNoteOnEventClass, "trackIdx", "I");
    jfieldID jNoteOnEventClassChildNoteOnEventsField = env->GetFieldID(jNoteOnEventClass, "childNoteOnEvents",
                                                                     "[Lcymheart/tau/midi/NoteOnEvent;");

    jfieldID jNoteOnEventClassChildNoteStateField = env->GetFieldID(jNoteOnEventClass, "childNoteState",
                                                                    "[I");

    //
    LinkedList<MidiEvent*>& trackNotes = editor->GetSimpleModeTrackNotes();
    int midiIndex = -1, m = 0;
    jobjectArray jNoteOnEvents = (jobjectArray)env->NewObjectArray(trackNotes.Size()/2, jNoteOnEventClass, NULL);
    jobject jNoteOnEvent, jChildNoteOnEvent;
    MidiEvent* midiEvent;

    auto node = trackNotes.GetHeadNode();
    for(; node; node=node->next)
    {
        midiEvent = node->elem;
        if(midiEvent->type != MidiEventType::NoteOn)
            continue;

        midiIndex++;
        NoteOnEvent* noteOnEvent = (NoteOnEvent*)midiEvent;
        jNoteOnEvent = CreateJNoteOnEvent(env, noteOnEvent);
        env->SetIntField(jNoteOnEvent, jMidiEventClassIndexField, midiIndex);
        env->SetIntField(jNoteOnEvent, jMidiEventClassTrackIdxField, noteOnEvent->track);

        //
        jobjectArray jChildNoteOnEvents =
                (jobjectArray)env->NewObjectArray(noteOnEvent->childNoteOnEventCount,
                                                  jNoteOnEventClass, NULL);

        jintArray jChildNoteOnState = env->NewIntArray(noteOnEvent->childNoteOnEventCount);

        for(int j=0; j<noteOnEvent->childNoteOnEventCount; j++)
        {
            jChildNoteOnEvent = CreateJNoteOnEvent(env, noteOnEvent->childNoteOnEvents[j]);
            env->SetIntField(jChildNoteOnEvent, jMidiEventClassTrackIdxField, noteOnEvent->childNoteOnEvents[j]->track);
            env->SetObjectArrayElement(jChildNoteOnEvents, j, jChildNoteOnEvent);
            env->DeleteLocalRef(jChildNoteOnEvent);
        }

        env->SetObjectField(jNoteOnEvent, jNoteOnEventClassChildNoteOnEventsField, jChildNoteOnEvents);
        env->SetObjectField(jNoteOnEvent, jNoteOnEventClassChildNoteStateField, jChildNoteOnState);

        //
        env->SetObjectArrayElement(jNoteOnEvents, m++, jNoteOnEvent);
        env->DeleteLocalRef(jNoteOnEvent);
    }


    //
    env->SetObjectField(jeditor, jEditorClassSimpleModeTrackNotesField, jNoteOnEvents);
    env->SetIntField(jeditor, jEditorClassWhiteKeyCountField, editor->GetSimpleModePlayWhiteKeyCount());
    env->SetFloatField(jeditor, jEditorClassMegreSrcNoteSecField, editor->GetMergeSimpleSrcNoteLimitSec());
    env->SetFloatField(jeditor, jEditorClassMegreDestNoteSecField, editor->GetMergeSimpleDestNoteLimitSec());

    env->DeleteLocalRef(jNoteOnEvents);

    //
    DeleteJNoteOnEventClassInfo(env);

    //
    env->DeleteLocalRef(jNoteOnEventClass);

}

void CreateMidiMarkers(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor)
{

    //MidiMarker
    jclass jMidiMarkerClass = env->FindClass("cymheart/tau/editor/MidiMarker");
    jmethodID jMidiMarkerClassInitMethod = env->GetMethodID(jMidiMarkerClass, "<init>", "()V");
    jfieldID jTrackField = env->GetFieldID(jMidiMarkerClass, "track","I");
    jfieldID jIsEnableMarkerTextField = env->GetFieldID(jMidiMarkerClass, "isEnableMarkerText","Z");
    jfieldID jTitleNameField = env->GetFieldID(jMidiMarkerClass, "titleName", "Ljava/lang/String;");
    jfieldID jIsEnableTempoField = env->GetFieldID(jMidiMarkerClass, "isEnableTempo","Z");
    jfieldID jStartTickField = env->GetFieldID(jMidiMarkerClass, "startTick", "I");
    jfieldID jStartSecField = env->GetFieldID(jMidiMarkerClass, "startSec", "D");
    jfieldID jMicroTempoField = env->GetFieldID(jMidiMarkerClass, "microTempo", "F");
    jfieldID jTickForQuarterNoteField = env->GetFieldID(jMidiMarkerClass, "tickForQuarterNote", "I");
    jfieldID jIsEnableTimeSignatureField = env->GetFieldID(jMidiMarkerClass, "isEnableTimeSignature", "Z");
    jfieldID jNumeratorField = env->GetFieldID(jMidiMarkerClass, "numerator", "I");
    jfieldID jDenominatorField = env->GetFieldID(jMidiMarkerClass, "denominator", "I");
    jfieldID jIsEnableKeySignatureField = env->GetFieldID(jMidiMarkerClass, "isEnableKeySignature", "Z");
    jfieldID jSfField = env->GetFieldID(jMidiMarkerClass, "sf", "I");
    jfieldID jMiField = env->GetFieldID(jMidiMarkerClass, "mi", "I");

    //
    MidiMarkerList* midiMarkerList = editor->GetMidiMarkerList();
    MidiMarker* midiMarker;
    vector<MidiMarker*>& midiMarkers = midiMarkerList->GetMidiMarkers();
    jobjectArray jMidiMarkers = (jobjectArray)env->NewObjectArray(midiMarkers.size(), jMidiMarkerClass, NULL);

    //
    for (int i = 0; i < midiMarkers.size(); i++)
    {
        midiMarker = midiMarkers[i];

        jobject jMidiMarker = env->NewObject(jMidiMarkerClass, jMidiMarkerClassInitMethod);

        const char* str = midiMarker->GetTitle().data();
        jstring jstr = str2jstring(env, str);
        env->SetIntField(jMidiMarker, jTrackField, midiMarker->GetTrack());
        env->SetObjectField(jMidiMarker, jTitleNameField, jstr);
        env->SetBooleanField(jMidiMarker, jIsEnableMarkerTextField, midiMarker->IsEnableMarkerText());
        env->SetBooleanField(jMidiMarker, jIsEnableTempoField, midiMarker->IsEnableTempo());
        env->SetIntField(jMidiMarker, jStartTickField, midiMarker->GetStartTick());
        env->SetDoubleField(jMidiMarker, jStartSecField, midiMarker->GetStartSec());
        env->SetFloatField(jMidiMarker, jMicroTempoField, midiMarker->GetMicroTempo());
        env->SetIntField(jMidiMarker, jTickForQuarterNoteField, midiMarker->GetTickForQuarterNote());
        env->SetBooleanField(jMidiMarker, jIsEnableTimeSignatureField, midiMarker->IsEnableTimeSignature());
        env->SetIntField(jMidiMarker, jNumeratorField, midiMarker->GetNumerator());
        env->SetIntField(jMidiMarker, jDenominatorField, midiMarker->GetDenominator());
        env->SetBooleanField(jMidiMarker, jIsEnableKeySignatureField, midiMarker->IsEnableKeySignature());
        env->SetIntField(jMidiMarker, jSfField, midiMarker->GetSf());
        env->SetIntField(jMidiMarker, jMiField, midiMarker->GetMi());

        //
        env->SetObjectArrayElement(jMidiMarkers, i, jMidiMarker);
    }


    //Editor
    jfieldID jEditorClassNdkMidiMarkersField = env->GetFieldID(jeditorClass, "_ndkMidiMarkers", "[Lcymheart/tau/editor/MidiMarker;");
    env->SetObjectField(jeditor, jEditorClassNdkMidiMarkersField, jMidiMarkers);


    //
    env->DeleteLocalRef(jMidiMarkers);
    env->DeleteLocalRef(jMidiMarkerClass);
}


void CreateMeasureInfo(JNIEnv *env, jclass jeditorClass, jobject jeditor, Editor* editor)
{
    jclass jMeasureInfoClass = env->FindClass("cymheart/tau/editor/MeasureInfo");
    jmethodID jMeasureInfoClassInitMethod = env->GetMethodID(jMeasureInfoClass, "<init>", "()V");
    jfieldID jMeasureField = env->GetFieldID(jMeasureInfoClass, "measure", "[F");
    jfieldID jBeatField = env->GetFieldID(jMeasureInfoClass, "beat", "[F");
    jfieldID jMeasureNumField = env->GetFieldID(jMeasureInfoClass, "measureNum", "I");
    jfieldID jBIdxField = env->GetFieldID(jMeasureInfoClass, "bIdx", "I");
    jfieldID jMIdxField = env->GetFieldID(jMeasureInfoClass, "mIdx", "I");

    //
    MeasureInfo* measureInfo = editor->GetMeasureInfo();
    jobject jMeasureInfo = env->NewObject(jMeasureInfoClass, jMeasureInfoClassInitMethod);

    float* measure = measureInfo->GetMeasureDatas();
    int measureDataSize = measureInfo->GetMeasureDataSize();
    jfloatArray jMeasures = env->NewFloatArray(measureDataSize);
    env->SetFloatArrayRegion(jMeasures, 0, measureDataSize, measure);
    env->SetObjectField(jMeasureInfo, jMeasureField, jMeasures);

    //
    float* beat = measureInfo->GetBeatDatas();
    int beatDataSize = measureInfo->GetBeatDataSize();
    jfloatArray jBeats = env->NewFloatArray(beatDataSize);
    env->SetFloatArrayRegion(jBeats, 0, beatDataSize, beat);
    env->SetObjectField(jMeasureInfo, jBeatField, jBeats);

    //
    env->SetIntField(jMeasureInfo, jMeasureNumField, measureInfo->GetMeasureCount());
    env->SetIntField(jMeasureInfo, jBIdxField, beatDataSize - 1);
    env->SetIntField(jMeasureInfo, jMIdxField, measureDataSize - 1);

    //
    jfieldID jEditorClassMeasureInfoField = env->GetFieldID(jeditorClass, "measureInfo", "Lcymheart/tau/editor/MeasureInfo;");
    env->SetObjectField(jeditor, jEditorClassMeasureInfoField, jMeasureInfo);


    //
    env->DeleteLocalRef(jMeasureInfo);
    env->DeleteLocalRef(jMeasures);
    env->DeleteLocalRef(jBeats);

    env->DeleteLocalRef(jMeasureInfoClass);

}


extern "C"
JNIEXPORT void JNICALL
Java_cymheart_tau_midi_MidiEvent_ndkSetPlayType(JNIEnv *env, jclass clazz, jlong ndk_midi_event,
                                                jint play_type) {
    MidiEvent* midiEvent = (MidiEvent*)ndk_midi_event;
    midiEvent->playType = (MidiEventPlayType)play_type;
}



