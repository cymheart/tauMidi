#ifndef _Editor_h_
#define _Editor_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"
#include"EditorTypes.h"
#include"MidiMarkerList.h"
#include"scutils/Semaphore.h"
using namespace scutils;


namespace tau
{

	/// <summary>
	/// 编辑类
	/// by cymheart, 2020--2021.
	/// </summary> 
	class DLL_CLASS Editor
	{
	public:
		Editor(Tau* tau);
		~Editor();

		//设置简单模式下, 白色按键的数量
		void SetSimpleModePlayWhiteKeyCount(int count);
		//获取简单模式下, 白色按键的数量
		int GetSimpleModePlayWhiteKeyCount();

		//被合并音符的最大时长
		void SetMergeSimpleSrcNoteLimitSec(float sec);
		//合并到目标音符的最大时长
		void SetMergeSimpleDestNoteLimitSec(float sec);
		//获取被合并音符的最大时长
		float GetMergeSimpleSrcNoteLimitSec();
		//获取合并到目标音符的最大时长
		float GetMergeSimpleDestNoteLimitSec();
		//生成简单模式音符轨道
		void CreateSimpleModeTrack();
		//获取简单模式轨道音符
		LinkedList<MidiEvent*>& GetSimpleModeTrackNotes();

		//是否读取完成
		bool IsLoadCompleted();

		void SetMidiEditor(MidiEditor* midiEditor)
		{
			this->midiEditor = midiEditor;
		}

		// 按下按键
		void OnKey(int key, float velocity, int trackIdx, int id = 0);
		// 释放按键 
		void OffKey(int key, float velocity, int trackIdx, int id = 0);
		// 释放指定轨道的所有按键 
		void OffAllKeys(int trackIdx);
		// 释放所有按键 
		void OffAllKeys();

		//载入
		//在非阻塞模式isWaitLoadCompleted = false下，
		//Load前面不能有任何与之相冲突的调用（play(), pasue(),stop()等,因为这些函数出于效率考虑没有加互斥锁）
		void Load(string& midiFilePath, bool isWaitLoadCompleted = true);

		//移除
		void Remove();

		//播放
		void Play();

		//停止
		void Stop();

		//暂停
		void Pause();

		//进入到步进播放模式
		void EnterStepPlayMode();

		//进入到等待播放模式
		void EnterWaitPlayMode();

		//进入到静音模式
		void EnterMuteMode();

		//离开当前播放模式
		void LeavePlayMode();

		//等待(区别于暂停，等待相当于在原始位置播放)
		void Wait();
		//继续，相对于等待命令
		void Continue();

		//按键信号
		void OnKeySignal(int key);
		//所有等待按键信号
		void OnWaitKeysSignal();

		//松开按键信号
		void OffKeySignal(int key);

		//移动到指定时间点
		void Runto(double sec);

		//设置播放时间点
		void Goto(double sec);

		//获取状态
		EditorState GetPlayState();

		//获取缓存状态
		CacheState GetCacheState();

		//获取初始化开始播放时间点
		double GetInitStartPlaySec()
		{
			return initStartPlaySec;
		}

		//设置初始化开始播放时间点
		void SetInitStartPlaySec(double sec)
		{
			initStartPlaySec = sec;
		}

		//是否等待中
		bool IsWait()
		{
			return isWait;
		}

		//是否有等待中的按键
		bool HavWaitKey()
		{
			return needOnKeyCount > 0;
		}

		//是否是等待中的按键
		bool IsWaitKey(int key)
		{
			return needOnkey[key] > 0;
		}

		//获取播放模式
		EditorPlayMode GetPlayMode()
		{
			return playMode;
		}


		//获取当前播放时间点
		double GetPlaySec();

		//获取结束时间
		inline double GetEndSec()
		{
			return endSec;
		}

		//获取当前bpm
		float GetCurtBPM();

		//根据指定秒数获取tick数
		uint32_t GetSecTickCount(double sec);

		//根据指定tick数秒数获取时间点
		double GetTickSec(uint32_t tick);

		//设置音符发音开始时间点
		void SetNoteSoundStartSec(double sec)
		{
			noteSoundStartSec = sec;
		}

		//设置音符发音结束时间点
		void SetNoteSoundEndSec(double sec)
		{
			noteSoundEndSec = sec;
		}

		void SetLateNoteSec(float sec)
		{
			lateNoteSec = sec;
		}


		//获取播放速率(相对于正常播放速率1.0的倍率)
		inline float GetSpeed()
		{
			return speed;
		}


		//判断是否全部解析了midiFile
		inline bool IsFullParsedMidiFile()
		{
			return isFullParsedMidiFile;
		}


		// 设定速度
		void SetSpeed(float speed_);

		// 禁止播放指定编号的轨道
		void DisableTrack(int trackIdx);

		// 禁止播放指定编号的轨道
		void DisableTracks(vector<int>& trackIdxs);

		// 禁止播放所有轨道
		void DisableAllTrack();

		// 启用播放指定编号的轨道
		void EnableTrack(int trackIdx);

		// 启用播放指定编号的轨道
		void EnableTracks(vector<int>& trackIdxs);

		// 启用播放所有轨道
		void EnableAllTrack();

		// 禁止播放指定编号通道
		void DisableChannel(int channelIdx);

		// 启用播放指定编号通道
		void EnableChannel(int channelIdx);

		//设置排除需要等待的按键
		void SetExcludeNeedWaitKeys(int* excludeKeys, int size);

		//设置排除需要等待的按键
		void SetExcludeNeedWaitKey(int key);

		//设置包含需要等待的按键
		void SetIncludeNeedWaitKey(int key);

		int GetCurtNeedOnKeyTrackIdx();


		float GetCurtNeedOnKeyVel()
		{
			return curtNeedOnKeyVel;
		}

		//设置轨道事件演奏方式
		void SetTrackPlayType(int trackIdx, MidiEventPlayType playType);

		//设置演奏类型
		void SetPlayType(MidiEventPlayType playType);

		// 设置对应轨道的乐器
		void SetVirInstrument(int trackIdx, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置打击乐号
		void SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);


		void ResetVirInstruments();


		//增加轨道，来自于MidiTrackList
		void AddMidiTracks(
			vector<MidiTrack*>& midiTracks,
			int start = -1, int end = -1,
			Track* dstFirstTrack = nullptr, int dstFirstBranchIdx = 0, float sec = 0);

		//增加标记，来自于midi事件列表中
		void AddMarkers(LinkedList<MidiEvent*>& midiEvents);

		MidiMarkerList* GetMidiMarkerList()
		{
			return &midiMarkerList;
		}

		//获取小节信息
		MeasureInfo* GetMeasureInfo()
		{
			return measureInfo;
		}

		//新建轨道
		void NewTracks(int count);

		//删除轨道
		void DeleteTracks(vector<Track*>& tracks);

		/**获取轨道乐器*/
		vector<VirInstrument*>& GetTrackInst(int idx);

		void SetSelectInstFragMode(SelectInstFragMode mode)
		{
			selectInstFragMode = mode;
		}

		void SelectInstFragment(int trackIdx, int branchIdx, int instFragIdx);
		void UnSelectInstFragment(int trackIdx, int branchIdx, int instFragIdx);

		void UnSelectAllInstFragment()
		{
			selectedInstFrags.clear();
		}

		InstFragment* GetInstFragment(int trackIdx, int branchIdx, int instFragIdx);

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveSelectedInstFragment(int dstTrack, int dstBranchIdx, float sec);

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveSelectedInstFragments(vector<int>& dstTracks, vector<int>& dstBranchIdx, vector<float>& secs);


		inline void SetUserData(void* data)
		{
			userData = data;
		}

		inline void* GetUserData()
		{
			return userData;
		}

		//获取采样流的频谱
		int GetSampleStreamFreqSpectrums(int channel, double* outLeft, double* outRight);


		vector<Track*>& GetTracks();


	public:

		EditorProcessCB loadStartCallBack = nullptr;
		EditorProcessCB loadCompletedCallBack = nullptr;
		EditorProcessCB releaseCallBack = nullptr;

	private:


		// 按下按键
		void OnKey(int key, float velocity, Track* track, int id = 0);
		// 释放按键
		void OffKey(int key, float velocity, Track* track, int id = 0);

		void ResetParams();

		//载入
		void _Load();

		//移除核心
		void RemoveCore();

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveInstFragments(
			vector<InstFragment*>& instFragments,
			vector<Track*>& dstTracks, vector<int>& dstBranchIdxs, vector<float>& secs);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragments(vector<InstFragment*>& instFragments, vector<Track*>& dstTracks, vector<float>& secs);

		//移动乐器片段到指定时间点
		void MoveInstFragments(vector<InstFragment*>& instFragments, vector<float>& secs);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragment(InstFragment* instFragment, Track* dstTrack, float sec);

		//移动乐器片段到指定时间点
		void MoveInstFragment(InstFragment* instFragment, float sec);

		//需要按键信号
		void NeedOnKeySignal(int key, float velocity, Track* track);

		//需要松开按键信号
		void NeedOffKeySignal(int key, float velocity, Track* track);


		//打印工程信息
		void PrintProjectInfo();

		static void ReadMidiFileThread(void* param);
		void ReadMidiFile();

		//获取当前时间之后需要等待按键信号的notes
		void GetNeedWaitKeySignalNote(int note, float lateSec);

	private:

		Tau* tau = nullptr;
		Synther* mainSynther = nullptr;
		MidiEditor* midiEditor;
		MidiMarkerList midiMarkerList;


		//初始化播放时间点
		double initStartPlaySec = 0;

		//当前播放时间
		atomic<double> curtPlaySec;

		//播放状态
		atomic<EditorState> playState;

		//缓存状态
		atomic<CacheState> cacheState;

		//结束时间点
		double endSec = 0;

		//播放速率(相对于正常播放速率1.0的倍率)
		float speed = 1;


		//播放模式
		EditorPlayMode playMode = EditorPlayMode::Common;

		//演奏类型
		MidiEventPlayType playType = MidiEventPlayType::DoubleHand;


		//是否等待
		atomic_bool isWait;


		//排除需要等待按键信号的按键
		atomic_bool excludeNeedWaitKey[128];

		//按下的按键次数，以键号分类计数
		int onkey[128] = { 0 };
		//按下的按键总个数
		int onKeyCount = 0;

		//需要按下的按键所在轨道，以键号分类计数
		deque<Track*> needOnKeyTrack[128];
		Track* curtNeedOnKeyTrack = nullptr;

		//需要按下的按键的力度，以键号分类计数
		deque<float> needOnKeyVelocity[128];
		float curtNeedOnKeyVel = 1;

		//需要按下的按键次数，以键号分类计数
		int needOnkey[128] = { 0 };
		//需要按下的按键总个数
		int needOnKeyCount = 0;

		//需要松开的按键次数，以键号分类计数
		int needOffkey[128] = { 0 };
		//需要松开的按键总个数
		int needOffKeyCount = 0;

		//当前时间后几秒钟是否有按键存在
		LateNoteInfo lateNoteInfo;
		LateNoteInfo noteOffLateNoteInfo;
		bool needWaitKey[128] = { false };

		float lateNoteSec = 1;

		//小节信息
		MeasureInfo* measureInfo;

		//音符发音开始时间点
		double noteSoundStartSec = -1;
		//音符发音结束时间点
		double noteSoundEndSec = -1;


		//
		SelectInstFragMode selectInstFragMode = SelectInstFragMode::SingleSelect;
		vector<SelectInstFragmentInfo> selectedInstFrags;


		//
		int loadMidiFileState = 0;


		string midiName;
		string midiFilePath;
		MidiFile* midiFile = nullptr;
		mutex loadingMidiFilelocker;
		Semaphore loadingMidiFileWaitSem;

		//在非阻塞设置停止调用命令，注意内存的释放
		bool isStopLoad = false;

		//midifile是否全部解析完成
		bool isFullParsedMidiFile = true;


		//
		void* userData = nullptr;

		friend class MidiEditor;
		friend class Synther;

	};
}

#endif
