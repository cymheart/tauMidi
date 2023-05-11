#ifndef _Tau_h_
#define _Tau_h_

#include "TauTypes.h"
#include"Midi/MidiTypes.h"
#include <Audio/Audio.h>
#include"Editor/EditorTypes.h"
#include"scutils/Semaphore.h"
#include"FX/FxTypes.h"

using namespace tauFX;


namespace tau
{

	/*
	* Tau是一个soundfont合成器
	* by cymheart, 2020--2023.
	*/
	class DLL_CLASS Tau
	{
	public:
		Tau();
		~Tau();

		//打开
		void Open();

		//关闭
		void Close();

		//是否已开启
		inline bool IsOpened()
		{
			return isOpened;
		}

		//设置是否静音
		void SetSilence(bool is)
		{
			isSilence = is;
		}

		void ClearRecordPCM();
		void StartRecordPCM();
		void StopRecordPCM();
		void SaveRecordPCM(string& path);
		void SaveRecordPCMToWav(string& path);
		//保存录制pcm的到mp3文件
		void SaveRecordPCMToMp3(string& path);


		//设置SoundFont
		inline void SetSoundFont(SoundFont* sf)
		{
			soundFont = sf;
		}

		inline SoundFont* GetSoundFont()
		{
			return soundFont;
		}

		//获取编辑器
		Editor* GetEditor()
		{
			return editor;
		}

		//设置是否使用多线程
		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		void SetUseMulThread(bool use);

		//设置Audio引擎
		inline void SetAudioEngine(Audio::EngineType type)
		{
			if (isOpened)
				return;

			audioEngineType = type;
		}

		//设置声道模式(立体声，单声道设置)
		inline void SetChannelOutputMode(ChannelOutputMode outputMode)
		{
			channelOutputMode = outputMode;
		}


		//获取声道模式(立体声，单声道)
		inline ChannelOutputMode GetChannelOutputMode()
		{
			return channelOutputMode;
		}

		//设置采样流缓存最大时长(单位:秒， 默认值:0s)
		void SetSampleStreamCacheSec(float sec)
		{
			sampleStreamCacheSec = sec;
		}

		//设置初始化开始播放时间点
		void SetInitStartPlaySec(double sec);


		//设置渲染品质
		inline void SetRenderQuality(RenderQuality quality)
		{
			if (isOpened)
				return;

			renderQuality = quality;
		}

		//获取渲染品质
		inline RenderQuality GetRenderQuality()
		{
			return renderQuality;
		}

		//设置是否开启精简合并音符
		inline void SetEnableMergeNotesOptimize(bool enable)
		{
			enableMergeNotesOptimize = enable;
		}

		//设置是否开启MidiEvent数量优化
		inline void SetEnableMidiEventCountOptimize(bool enable)
		{
			enableMidiEventCountOptimize = enable;
		}

		//设置是否开启拷贝相同通道控制事件(默认:开启)
		inline void SetEnableCopySameChannelControlEvents(bool enable)
		{
			enableCopySameChannelControlEvents = enable;
		}


		//设置midi文件中保持同时按键的数量 (默认值:-1 无限制)
		inline void SetMidiKeepSameTimeNoteOnCount(int count)
		{
			midiKeepSameTimeNoteOnCount = count;
		}

		//设置是否启用midi文件解析极限时间(默认值:2s)
		inline void SetEnableMidiEventParseLimitTime(bool enable, float limitSec = 2.0)
		{
			isEnableMidiEventParseLimitTime = enable;
			midiEventLimitParseSec = limitSec;
		}


		//设置是否开启生成采样频谱,频谱点采样数量(默认值: 2048)
		inline void SetEnableCreateFreqSpectrums(bool enable, int count = 2048)
		{
			isEnableCreateFreqSpectrums = enable;
			freqSpectrumsCount = count;
		}



		//设置样本处理采样率
		void SetSampleProcessRate(int rate);

		//获取样本处理采样率
		inline float GetSampleProcessRate()
		{
			return sampleProcessRate;
		}

		inline float GetInvSampleProcessRate()
		{
			return (float)invSampleProcessRate;
		}


		// 设置帧样本数量
		//这个值越小，声音的实时性越高（在实时演奏时，值最好在1024以下，最合适的值为512）,	
		//当这个值比较小时，cpu内耗增加
		void SetFrameSampleCount(int count);

		// 获取帧样本数量
		inline int GetFrameSampleCount()
		{
			return frameSampleCount;
		}

		// 设置子帧样本数量
		//这个值最好固定在64
		inline void SetChildFrameSampleCount(int count)
		{
			if (isOpened)
				return;

			childFrameSampleCount = count;
			if (childFrameSampleCount > frameSampleCount) childFrameSampleCount = frameSampleCount;
			else if (childFrameSampleCount < 1)childFrameSampleCount = 1;
		}

		// 获取子帧样本数量
		inline int GetChildFrameSampleCount()
		{
			return childFrameSampleCount;
		}

		// 获取处理每子帧样本所花费的时间(单位:秒)
		inline double GetPerChildFrameSampleSec()
		{
			return invSampleProcessRate * childFrameSampleCount;
		}

		//设置极限发声区域数量(默认值:600)
		//当播放有卡顿现象时，把这个值调小，会提高声音的流畅度
		inline void SetLimitRegionSounderCount(int count)
		{
			if (isOpened)
				return;

			limitRegionSounderCount = count;
		}

		//设置极限按键速率(默认值:800)
		//当播放有卡顿现象时，把这个值调小，会提高声音的流畅度
		inline void SetLimitOnKeySpeed(float speed)
		{
			if (isOpened)
				return;

			limitOnKeySpeed = speed;
		}

		//判断是否使用外部普通调制器
		inline bool UseCommonModulator()
		{
			return useCommonModulator;
		}

		//设置是否使用区域内部和声效果
		inline void SetUseRegionInnerChorusEffect(bool use)
		{
			if (isOpened)
				return;

			useRegionInnerChorusEffect = use;
		}

		//判断是否使用区域内部和声效果
		inline bool UseRegionInnerChorusEffect()
		{
			return useRegionInnerChorusEffect;
		}


		//增加效果器
		void AddEffect(TauEffect* effect);

		// 按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst, int id = 0);
		// 释放按键 
		void OffKey(int key, float velocity, VirInstrument* virInst, int id = 0);

		//添加替换乐器
		void AppendReplaceInstrument(
			int orgBankMSB, int orgBankLSB, int orgInstNum,
			int repBankMSB, int repBankLSB, int repInstNum);

		//移除替换乐器
		void RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum);


		//获取当前Midi文件结束时间
		inline float GetMidiEndSec()
		{
			return 0;
		}


		//判断是否载入完成
		bool IsLoadCompleted();

		//判断是否全部解析了midiFile
		bool IsFullParsedMidiFile();

		//载入
		//在非阻塞模式isWaitLoadCompleted = false下，
		//Load前面不能有任何与之相冲突的调用（play(), pasue(),stop()等,因为这些函数出于效率考虑没有加互斥锁）
		void Load(string& midiFilePath, bool isWaitLoadCompleted = true);

		//设置简单模式下, 白色按键的数量
		void SetSimpleModePlayWhiteKeyCount(int count);

		//生成简单模式音符轨道
		void CreateSimpleModeTrack();

		//新建轨道
		void NewTracks(int count);

		//是否等待中
		bool IsWait();

		//获取播放模式
		EditorPlayMode GetPlayMode();

		// 按下按键
		void OnKey(int key, float velocity, int trackIdx, int id = 0);

		// 释放按键 
		void OffKey(int key, float velocity, int trackIdx, int id = 0);

		// 释放指定轨道的所有按键 
		void OffAllKeys(int trackIdx);

		// 释放所有按键 
		void OffAllKeys();

		//播放
		void Play();

		//停止
		void Stop();

		//暂停
		void Pause();

		//移除
		void Remove();

		//进入到步进播放模式
		void EnterStepPlayMode();

		//进入到等待播放模式
		void EnterWaitPlayMode();

		//进入到静音模式
		void EnterMuteMode();

		//离开播放模式
		void LeavePlayMode();

		//设置事件演奏方式
		void SetPlayType(MidiEventPlayType playType);

		//等待(区别于暂停，等待相当于在原始位置播放)
		void Wait();
		//继续，相对于等待命令
		void Continue();

		//设置编辑器排除需要等待的按键
		void EditorSetExcludeNeedWaitKeys(int* excludeKeys, int size);

		//设置编辑器排除需要等待的按键
		void EditorSetExcludeNeedWaitKey(int key);

		//设置编辑器包含需要等待的按键
		void  EditorSetIncludeNeedWaitKey(int key);

		//发送编辑器按键信号
		void EditorOnKeySignal(int key);
		//发送编辑器松开按键信号
		void EditorOffKeySignal(int key);

		//移动到指定时间点
		void Runto(double sec);

		// 指定播放的起始时间点
		void Goto(float sec);

		//获取midi播放状态
		EditorState GetPlayState();

		//获取当前播放时间点
		double GetPlaySec();

		//获取结束时间点
		double GetEndSec();

		//获取当前bpm
		float GetCurtBPM();

		//根据指定tick数秒数获取时间点
		double GetTickSec(uint32_t tick);

		// 设定播放速度
		void SetSpeed(float speed);

		// 禁止播放指定编号的轨道
		void DisableTrack(int trackIdx);
		// 禁止播放所有轨道
		void DisableAllTrack();

		// 启用播放指定编号的轨道
		void EnableTrack(int trackIdx);
		// 启用播放所有轨道
		void EnableAllTrack();

		// 禁止播放指定编号通道
		void DisableChannel(int channelIdx);

		// 启用播放指定编号通道
		void EnableChannel(int channelIdx);

		void SetSelectInstFragMode(SelectInstFragMode mode);
		void SelectInstFragment(int trackIdx, int branchIdx, int instFragIdx);
		void UnSelectInstFragment(int trackIdx, int branchIdx, int instFragIdx);
		void UnSelectAllInstFragment();

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveSelectedInstFragment(int dstTrack, int dstBranchIdx, float sec);

		//移动乐器片段到目标轨道分径的指定时间点
		void MoveSelectedInstFragments(vector<int>& dstTracks, vector<int>& dstBranchIdx, vector<float>& secs);

		//设置轨道事件演奏方式
		void SetTrackPlayType(int trackIdx, MidiEventPlayType playType);

		// 设置乐器Bend值
		void SetVirInstrumentPitchBend(VirInstrument* virInst, int value);

		// 设置乐器按键压力值
		void SetVirInstrumentPolyPressure(VirInstrument* virInst, int key, int pressure);

		// 设置乐器Midi控制器值
		void SetVirInstrumentMidiControllerValue(VirInstrument* virInst, MidiControllerType midiController, int value);

		//设置是否开启所有乐器效果器
		void SetEnableAllVirInstEffects(bool isEnable);

		// 设置虚拟乐器值
		void SetVirInstrumentProgram(VirInstrument* virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		/// <summary>
		/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在，将在虚拟乐器列表中自动创建它
		/// 注意如果deviceChannelNum已经被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
		/// 而不会同时在一个通道上创建超过1个的虚拟乐器
		/// </summary>
		/// <param name="deviceChannel">乐器所在的设备通道</param>
		/// <param name="bankSelectMSB">声音库选择0</param>
		/// <param name="bankSelectLSB">声音库选择1</param>
		/// <param name="instrumentNum">乐器编号</param>
		/// <returns></returns>
		VirInstrument* EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		/// <summary>
		/// 移除乐器
		/// </summary>
		void RemoveVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 移除所有乐器
		/// </summary>
		void RemoveAllVirInstrument(bool isFade = true);

		/// <summary>
		/// 打开乐器
		/// </summary>
		void OpenVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 关闭虚拟乐器
		/// </summary>
		void CloseVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 获取虚拟乐器列表的备份
		/// </summary>
		vector<VirInstrument*>* TakeVirInstrumentList();

		// 设置midi对应轨道的乐器
		void SetMidiVirInstrument(int trackIdx, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//为midi文件设置打击乐号
		void SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		void ResetVirInstruments();

		/// <summary>
		/// 录制所有乐器弹奏为midi
		/// </summary>
		/// <param name="bpm">录制的BPM</param>
		/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
		void RecordMidi(float bpm, float tickForQuarterNote);

		/// <summary>
		/// 录制指定乐器弹奏为midi
		/// </summary>
		/// <param name="virInst">如果为null,将录制所有乐器</param>
		/// <param name="bpm">录制的BPM</param>
		/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
		void RecordMidi(VirInstrument* virInst, float bpm, float tickForQuarterNote);

		/// <summary>
		/// 停止所有乐器当前midi录制
		/// </summary>
		void StopRecordMidi();

		/// <summary>
		/// 停止录制指定乐器弹奏midi
		/// </summary>
		/// <param name="virInst">如果为null,将录制所有乐器</param>
		void StopRecordMidi(VirInstrument* virInst);


		// 生成所有乐器已录制的midi到midiflie object中
		MidiFile* CreateRecordMidiFile();

		// 根据给定的乐器组，生成它们已录制的midi到midiflie object中
		MidiFile* CreateRecordMidiFile(VirInstrument** virInsts, int size);

		//保存midiFile到文件
		void SaveMidiFileToDisk(MidiFile* midiFile, string saveFilePath);

	private:

		// 获取乐器预设
		Preset* GetInstrumentPreset(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 获取乐器预设
		Preset* GetInstrumentPreset(int key);


	private:

		//是否开启
		bool isOpened = false;

		//是否静音
		bool isSilence = false;


#ifdef _WIN32
		Audio::EngineType audioEngineType = Audio::EngineType::SDL;
#else
		Audio::EngineType audioEngineType = Audio::EngineType::Oboe;
#endif

		//采样流缓存最大时长(单位:秒， 默认值:0s)
		float sampleStreamCacheSec = 0;

		//是否开启精简合并音符
		bool enableMergeNotesOptimize = true;

		//是否开启MidiEvent数量优化
		bool enableMidiEventCountOptimize = true;

		//设置是否开启拷贝相同通道控制事件
		bool enableCopySameChannelControlEvents = true;

		//midi文件中保持同时按键的数量 (默认值:15,  -1表示无限制)
		int midiKeepSameTimeNoteOnCount = 15;

		//是否启用MidiEvent解析极限时间
		bool isEnableMidiEventParseLimitTime = false;
		//MidiEvent极限解析时间(默认值:2s)
		float midiEventLimitParseSec = 2.0f;


		//立体声，单声道选择
		ChannelOutputMode channelOutputMode = ChannelOutputMode::Stereo;

		//渲染品质
		RenderQuality renderQuality = RenderQuality::Fast;

		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		bool useMulThreads = false;

		//是否使用外部普通调制器
		bool useCommonModulator = false;

		//使用区域内部和声效果
		bool useRegionInnerChorusEffect = false;

		//发声区域最大限制数量
		int limitRegionSounderCount = 500;

		//按键速率最大限制
		float limitOnKeySpeed = 600;

		//是否开启乐器效果器
		bool isEnableVirInstEffects = true;

		//是否开启生成采样频谱
		bool isEnableCreateFreqSpectrums = false;
		//频谱点采样数量(默认值: 2048)
		int freqSpectrumsCount = 2048;

		//声音处理时的采样周期，不同于样本采样率
		//样本采样率和声音处理采样率的频率矫正倍率计算为: rateAdjustMul = sampleRate / sampleProcessRate
		//如每秒钟处理采样点44100个采样点
		float sampleProcessRate = 48000;

		// 帧采样数量(默认值:512)
		int frameSampleCount = 512;

		//子帧细分采样数量,会影响midi发音的流畅度和每个按键播放时间的精度
		int childFrameSampleCount = 64;

		//采样率的倒数，表示1个采样点所花费的时间
		double invSampleProcessRate = 1.0f / 48000.0f;

		//单位采样时间
		double unitSampleSec = 0;

		/// <summary>
		/// 在音乐中我们一般用BPM来表述乐曲的速度，BPM(Beat per Minute)的意思是每分钟的拍子数。
		/// 例如，BPM=100，表示该歌曲的速度是每分钟100拍。注意，对于音乐家来说，BPM中的一拍是指一个四分音符所发音的时间，
		/// 而不管歌曲的拍号是多少.例如，假设歌曲的拍号是3/8拍(以八分音符为一拍，每小节3拍)，BPM=100，
		/// 那么，音乐家依然会把歌曲的速度认为是以四分音符(非八分音符)为一拍，每分钟100拍。
		/// 因此，BPM被称为是“音乐速度(Musical Tempo)”
		/// </summary>
		float recordMidiBPM = 120;

		//录制midi时一个四分音符所要弹奏的tick数,默认值120
		float recordMidiTickForQuarterNote = 120;

		SoundFont* soundFont;

		//预设乐器替换
		unordered_map<uint32_t, uint32_t>* presetBankReplaceMap = nullptr;


		mutex lockMutex;

		Editor* editor;

		Synther* synther = nullptr;


		friend class VirInstrument;
		friend class MidiEditor;
		friend class RegionSounderThread;
		friend class KeySounder;
		friend class Synther;
		friend class RealtimeSynther;
		friend class Editor;
	};
}

#endif