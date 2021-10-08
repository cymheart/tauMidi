#ifndef _Tau_h_
#define _Tau_h_

#include "TauTypes.h"
#include"Midi/MidiTypes.h"
#include <Audio/Audio.h>
#include"FX/TauEffect.h"
#include"FX/EffectList.h"
using namespace tauFX;

namespace tau
{

	/*
	* Tau是一个soundfont合成器
	* by cymheart, 2020--2021.
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

		//设置SoundFont
		inline void SetSoundFont(SoundFont* sf)
		{
			if (isOpened)
				return;

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

		//设置是否仅使用1个主合成器	(默认值:true, 仅使用1个)
		//合成器模式:
		// 1.一主合成器，带多个从合成器模式，从合成器每帧的framebuf将会被同步到主合成其中，最后推送到audio流中
		// 2.全部都是主合成器模式， 每个合成器都将单独合成帧流到audio流中，之间互不相关，好处是效率高，缺点是，最后不能应用总通道效果器
		inline void SetOnlyUseOneMainSynther(bool isUseOnlyOne)
		{
			if (isOpened)
				return;

			onlyUseOneMainSynther = isUseOnlyOne;
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

		//设置每个合成器中最大轨道数量
		inline void SetPerSyntherLimitTrackCount(int count)
		{
			if (isOpened)
				return;

			perSyntherLimitTrackCount = count;
		}

		//设置最大合成器数量(默认值:12)
		inline void SetLimitSyntherCount(int count)
		{
			if (isOpened)
				return;

			limitSyntherCount = count;
		}

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

		//设置轨道通道合并模式
		inline void SetTrackChannelMergeMode(TrackChannelMergeMode mode)
		{
			if (isOpened)
				return;

			midiFileMergeMode = mode;
		}

		//获取轨道通道合并模式
		inline TrackChannelMergeMode GetTrackChannelMergeMode()
		{
			return midiFileMergeMode;
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
			return invSampleProcessRate;
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
		inline float GetPerChildFrameSampleSec()
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

		//生成MidiEditorSynther
		MidiEditorSynther* CreateMidiEditorSynther();

		//增加效果器
		void AddEffect(TauEffect* effect);

		// 按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst, int delayMS = 0);
		// 释放按键 
		void OffKey(int key, float velocity, VirInstrument* virInst, int delayMS = 0);
		// 取消按键 
		void CancelDownKey(int key, float velocity, VirInstrument* virInst, int delayMS = 0);
		// 取消释放按键 
		void CancelOffKey(int key, float velocity, VirInstrument* virInst, int delayMS = 0);

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

		//载入
		void Load(string& midiFilePath);

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

		//离开步进播放模式
		void LeaveStepPlayMode();

		//移动到指定时间点
		void Moveto(double sec);

		// 指定播放的起始时间点
		void Goto(float sec);

		//获取midi状态
		EditorState GetEditorState();

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
		/// 打开乐器
		/// </summary>
		void OnVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 关闭虚拟乐器
		/// </summary>
		void OffVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 获取虚拟乐器列表的备份
		/// </summary>
		vector<VirInstrument*>* TakeVirInstrumentList();

		// 设置midi对应轨道的乐器
		void SetMidiVirInstrument(int trackIdx, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//为midi文件设置打击乐号
		void SetMidiBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);


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

		void CreateSplitMidiTracksRenders();

	private:

#ifdef _WIN32
		Audio::EngineType audioEngineType = Audio::EngineType::SDL;
#else
		Audio::EngineType audioEngineType = Audio::EngineType::Oboe;
#endif

		//midi文件轨道通道合并模式
		TrackChannelMergeMode midiFileMergeMode = TrackChannelMergeMode::AutoMerge;

		//立体声，单声道选择
		ChannelOutputMode channelOutputMode = ChannelOutputMode::Stereo;

		//渲染品质
		RenderQuality renderQuality = RenderQuality::Fast;

		//是否开启
		bool isOpened = false;

		//是否仅使用1个主合成器
		//合成器模式:
		// 一主合成器，带多个从合成器模式，从合成器每帧的framebuf将会被同步到主合成其中，最后推送到audio流中
		// 全部都是主合成器模式， 每个合成器都将单独合成帧流到audio流中，之间互不相关，好处是效率高，缺点是，最后不能应用总通道效果器
		bool onlyUseOneMainSynther = true;

		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		bool useMulThreads = false;

		//是否使用外部普通调制器
		bool useCommonModulator = false;

		//使用区域内部和声效果
		bool useRegionInnerChorusEffect = true;

		//发声区域最大限制数量
		int limitRegionSounderCount = 500;

		//按键速率最大限制
		float limitOnKeySpeed = 600;

		//每个合成器中最大轨道数量
		int perSyntherLimitTrackCount = 20;

		//最大合成器数量
		int limitSyntherCount = 12;

		//是否开启乐器效果器
		bool isEnableVirInstEffects = true;

		//声音处理时的采样周期，不同于样本采样率
		//样本采样率和声音处理采样率的频率矫正倍率计算为: rateAdjustMul = sampleRate / sampleProcessRate
		//如每秒钟处理采样点44100个采样点
		float sampleProcessRate = 44100;

		// 帧采样数量(默认值:512)
		int frameSampleCount = 512;

		//子帧细分采样数量,会影响midi发音的流畅度和每个按键播放时间的精度
		int childFrameSampleCount = 64;

		//采样率的倒数，表示1个采样点所花费的时间
		float invSampleProcessRate = 1.0f / 44100.0f;

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


		Editor* editor;


		MidiEditorSynther* mainEditorSynther = nullptr;
		MidiEditorSynther* midiEditorSynthers[500] = { nullptr };
		int syntherCount = 1;

		friend class MidiEditorSynther;
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
