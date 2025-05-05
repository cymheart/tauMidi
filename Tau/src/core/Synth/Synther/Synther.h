#ifndef _Synther_h_
#define _Synther_h_

#include "Synth/TauTypes.h"
#include "Synth/Editor/EditorTypes.h"
#include"Midi/MidiTypes.h"
#include"FX/TauEffect.h"
#include"FX/EffectList.h"
#include"Audio/Audio.h"
#include"task/MsgProcesser.h"
#include "task/Task.h"
#include"scutils/RingBuffer.h"
#include "Synth/Editor/EditorTypes.h"
#include <scutils/ArrayPool.h>

using namespace task;
using namespace tauFX;
using namespace scutils;
using namespace dsignal;

namespace tau
{
	struct FadeSamplesInfo
	{
		double gain = 1;
		double gainStep = 0;
		float* samples = nullptr;
		int size = 0;
		int pos = 0;
	};

	class Synther
	{
	public:
		Synther(Tau* tau);
		virtual ~Synther();

		//投递任务
		void PostTask(TaskCallBack taskCallBack, void* data, int delay = 0);

		//投递任务
		void PostTask(Task* task, int delay = 0);

	public:

		virtual void Open();
		virtual void Close();

		//设置当前缓存播放时间
		void SetCurtCachePlaySec(double sec);

		float GetSampleProcessRate();

		//是否已开启
		inline bool IsOpened()
		{
			return isOpened;
		}

	private:
		static void FillAudioSample(void* udata, uint8_t* stream, int len);

	protected:

		void Lock();
		void UnLock();

		// 开启声音播放引擎
		void OpenAudio();


		//获取当前时间点
		inline double GetCurtSec()
		{
			return sec;
		}

		double GetPlaySec();

		//获取midi结束时间(单位:秒)
		double GetEndSec();

		//设置缓存状态
		void SetCachePlayState(EditorState s);

		//设置缓存的缓存状态
		void SetCacheState(CacheState cs);

		//设置帧样本数量
		//这个值越小，声音的实时性越高（在实时演奏时，值最好在1024以下，最合适的值为512）,
		//当这个值比较小时，cpu内耗增加
		void SetFrameSampleCount(int count);

		//设置是否使用多线程
		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		void SetUseMulThread(bool use);

		//是否包含指定的虚拟乐器
		bool IsHavVirInstrument(VirInstrument* virInst);

		//添加替换乐器
		void AppendReplaceInstrument(
			int orgBankMSB, int orgBankLSB, int orgInstNum,
			int repBankMSB, int repBankLSB, int repInstNum);

		//移除替换乐器
		void RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum);

		//获取替换乐器key
		int GetReplaceInstrumentKey(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//增加效果器
		void AddEffect(TauEffect* effect);

		//调制虚拟乐器参数
		void ModulationVirInstParams(Channel* channel);

		//调制虚拟乐器参数
		void ModulationVirInstParams(VirInstrument* virInst);

		//设置是否开启所有乐器效果器
		void SetEnableAllVirInstEffects(bool isEnable);

		//设置乐器bend
		void SetVirInstrumentPitchBend(VirInstrument* virInst, int bend);

		//设置乐器按键压力
		void SetVirInstrumentPolyPressure(VirInstrument* virInst, int key, int pressure);

		//设置乐器midicontroller
		void SetVirInstrumentMidiControllerValue(VirInstrument* virInst, MidiControllerType midiController, int value);

		// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在，将在虚拟乐器列表中自动创建它
		// 注意如果channel已经被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
		// 而不会同时在一个通道上创建超过1个的虚拟乐器
		VirInstrument* EnableVirInstrument(Channel* channel, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 设置虚拟乐器值
		void SetVirInstrumentProgram(VirInstrument* virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum);


		//移除所有虚拟乐器
		void RemoveAllVirInstrument(bool isFade = true);

		//根据通道移除虚拟乐器
		void RemoveVirInstrument(Channel* channel, bool isFade = true);

		//移除虚拟乐器
		void RemoveVirInstrument(VirInstrument* virInst, bool isFade = true);

		void AddNeedDelVirInstrument(VirInstrument* virInst);

		//打开虚拟乐器
		void OpenVirInstrument(VirInstrument* virInst, bool isFade = true);

		//根据通道打开相关虚拟乐器
		void OpenVirInstrument(Channel* channel, bool isFade = true);

		//关闭虚拟乐器
		void CloseVirInstrument(VirInstrument* virInst, bool isFade = true);

		//根据通道关闭相关虚拟乐器
		void CloseVirInstrument(Channel* channel, bool isFade = true);

		// 按下按键
		void OnKey(int key, float velocity, Track* track, int id = 0);

		//按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst, int id = 0);

		//根据通道按下相关虚拟乐器指定按键
		void OnKey(Channel* channel, int key, float velocity, int tickCount = -1, int id = 0);

		// 释放按键 
		void OffKey(int key, float velocity, Track* track, int id = 0);

		//根据通道松开相关虚拟乐器指定按键
		void OffKey(Channel* channel, int key, float velocity, int id = 0);

		// 释放按键
		void OffKey(int key, float velocity, VirInstrument* virInst, int id = 0);

		//根据通道关闭相关虚拟乐器所有按键
		void OffAllKeys(Channel* channel);

		//关闭所有虚拟乐器所有按键
		void OffAllKeys();

		//关闭与指定id匹配的所有虚拟乐器所有按键
		void OffAllKeys(int id);

		//根据指定通道获取关连虚拟乐器
		vector<VirInstrument*>* GetVirInstruments(Channel* channel);

		//获取虚拟乐器列表的备份
		vector<VirInstrument*>* TakeVirInstrumentList();

		//添加虚拟乐器到表
		void AppendToVirInstList(VirInstrument* virInst);

		// 从乐器表移除虚拟乐器
		void RemoveVirInstFromList(VirInstrument* virInst);

		//清除通道buffer
		void ClearChannelBuffer();

		//混合所有乐器中的样本到声道buffer中
		void MixVirInstsSamplesToChannelBuffer();

		//合成采样buffer
		void SynthSampleBuffer();

		//合并声道buffer到数据流
		void CombineChannelBufferToStream();

		//渐隐合成流的声音，
		//对effects效果器产生的余音进行检测，以确保没有任何余音，才结束
		void FadeSynthStream();

		// 渲染虚拟乐器区域发声     
		void RenderVirInstZoneSound();

		//移除需要删除的乐器
		void RemoveNeedDeleteVirInsts(bool isDirectRemove = false);

		//快速释音超过限制的区域发声
		void FastReleaseZoneSounders();

		//快速释音超过限制的区域发声2
		//比例算法
		void FastReleaseZoneSounders2();

		//  移除已完成所有区域发声处理(采样处理)的KeySounder               
		void RemoveProcessEndedKeySounder();

		static bool SounderCountCompare(VirInstrument* a, VirInstrument* b);

		// 请求缓存渲染事件
		void ReqCacheRender();
		static void CacheRenderTask(Task* ev);

		// 请求帧渲染事件
		void ReqRender();
		static void RenderTask(Task* ev);
		// 渲染每帧音频
		void Render();

		// 帧渲染
		void FrameRender(uint8_t* stream, int len);

		// 录制所有乐器弹奏为midi
		void RecordMidi();

		// 录制指定乐器弹奏为midi
		void RecordMidi(VirInstrument* virInst);

		/// 停止所有乐器midi的录制
		void StopRecordMidi();

		// 停止指定乐器midi的录制
		// 如果为null,将停止录制所有乐器
		void StopRecordMidi(VirInstrument* virInst);

		// 获取录制的midi轨道
		vector<MidiTrack*>* TakeRecordMidiTracks(
			VirInstrument** virInst, int size, float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos);


		/// <summary>
		/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在将在虚拟乐器列表中自动创建它
		/// </summary>
		/// <param name="deviceChannel">乐器所在设备通道</param>
		/// <param name="bankSelectMSB">声音库选择0</param>
		/// <param name="bankSelectLSB">声音库选择1</param>
		/// <param name="instrumentNum">乐器编号</param>
		/// <returns></returns>
		VirInstrument* EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);


		//删除所有虚拟乐器
		void DelAllVirInstrument(int type = 0);

		// 删除虚拟乐器
		void DelVirInstrument(VirInstrument* virInst);

		//停止所有虚拟乐器
		void StopAllVirInstrument(int type = 0);


		//根据设备通道号获取设备通道
		//查找不到将返回空值
		Channel* GetDeviceChannel(int deviceChannelNum);


		//建立MidiEditor
		void CreateMidiEditor();

		//播放midi
		void Play();

		//停止播放midiEditor
		void Stop();

		//暂停播放midiEditor
		void Pause();

		//midi快进到指定位置
		void Goto(double sec);

		//重新缓存
		void ReCache();

		//移除midiEditor
		void Remove();

		void Runto(double sec);

		void SetPlayType(MidiEventPlayType playType);
		void EnterPlayMode(EditorPlayMode playMode);
		void LeavePlayMode();

		//获取播放状态(通用)
		EditorState GetPlayStateCommon();

		//获取播放时间(通用)
		double GetPlaySecCommon();

		//获取midi状态
		EditorState GetPlayState();

		//设置midi结束时间(单位:秒)
		void SetEndSec(double sec);

		//设置midi播放速度
		void SetSpeed(float speed);

		// 禁止播放指定的轨道
		void DisableTrack(Track* track);

		// 禁止播放指定的轨道
		void DisableTrack(int trackIdx);

		// 启用播放指定的轨道
		void EnableTrack(Track* track);

		// 启用播放指定的轨道
		void EnableTrack(int trackIdx);

		// 禁止播放指定编号Midi文件通道
		void DisableChannel(int channelIdx);

		// 启用播放指定编号Midi文件通道
		void EnableChannel(int channelIdx);

		//设置通道声音增益(单位:dB)
		void SetChannelVolumeGain(int channelIdx, float gainDB);

		//设置轨道事件演奏方式
		void SetTrackPlayType(Track* track, MidiEventPlayType playType);

		// 设置midi对应轨道通道的乐器
		void SetVirInstrument(Track* track,
			int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 设置midi对应轨道通道的乐器
		void SetVirInstrument(int trackIdx,
			int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//为midi文件设置打击乐号
		void SetBeatVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		//设置标记
		void SetMarkerList(MidiMarkerList* mrklist);

		//新建轨道
		void NewTrack();

		//删除轨道
		void DeleteTrack(Track* track);

		//移动乐器片段到目标轨道的指定时间点
		void MoveInstFragment(InstFragment* instFragment, Track* dstTrack, int dstBranchIdx, float sec);

		//移除乐器片段
		void RemoveInstFragment(InstFragment* instFragment);

		//计算指定轨道所有事件的实际时间点
		void ComputeTrackEventsTime(Track* track);

		//计算结束时间点
		void ComputeEndSec();

		// 处理播放midi文件事件
		void ProcessMidiEvents();


		/////////////////////////////////////////////////////
		bool CanCache();

		//缓存帧渲染
		void CacheRender();

		void CachePlay();
		void CachePause();
		void CacheStop();
		void CacheReadTail();
		bool CacheGoto(double sec, bool isMustReset = false);
		inline void CacheReset()
		{
			CacheGoto(curtCachePlaySec, true);
		}

		//缓存输入
		void CacheInput();
		//在主合成器中的缓存处理
		void CacheReadByState();

		//生成渐降的samples
		void CreateFallCacheSamples();

		//生成渐升的CacheGain
		inline void CreateRiseCacheGain();

		//缓存处理
		void CacheWrite();
		void CacheRead();
		void CacheReadFallSamples();

		void CacheEnterStepPlayMode();

		void ClearCacheBuffer();


		//获取缓存播放时间
		inline double GetCachePlaySec()
		{
			return curtCachePlaySec;
		}

		inline EditorState GetCachePlayState()
		{
			return cachePlayState;
		}

		void TestCacheWriteSoundEnd();

		//缓存是否被启用
		inline bool IsCacheEnable()
		{
			return (maxCacheSize > 0 && isEnableCache ? true : false);
		}


		inline void CacheGainFade()
		{
			if (cacheGain == dstCacheGain)
				return;

			if (cacheGain < dstCacheGain) {
				cacheGain += cacheGainStep;
				if (cacheGain > dstCacheGain)
					cacheGain = dstCacheGain;
			}
			else if (cacheGain > dstCacheGain) {
				cacheGain -= cacheGainStep;
				if (cacheGain < dstCacheGain)
					cacheGain = dstCacheGain;
			}
		}

		void ShowCacheInfo();
		static void _ShowCacheInfoTask(Task* task);

	protected:

		Tau* tau;
		TaskProcesser* cacheTaskProcesser = nullptr;
		TaskProcesser* taskProcesser = nullptr;
		ZoneSounderThread* ZoneSounderThreadPool = nullptr;

		//是否开启
		bool isOpened = false;

		//此处不能修改为普通bool类型，如果改成普通bool类型
		//由于多线程同时会检测和修改这个量，会导致检测不准确，声音渲染会有几率触发不正常停止
		atomic_bool isFrameRenderCompleted;

		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		bool useMulThreads = false;


		//使用中的虚拟乐器列表
		vector<VirInstrument*> virInstList;
		//使用中的虚拟乐器列表(此列表用于乐器发声数量排序)
		vector<VirInstrument*> virInsts;
		unordered_set<VirInstrument*> virInstSet;
		//需要移除的乐器
		vector<VirInstrument*> needDeleteVirInsts;

		//帧时间
		double frameSec = 0;

		// 帧采样数量
		int frameSampleCount = 1024;
		//通道数量
		int channelCount = 2;

		// 左通道已处理采样点
		float* leftChannelSamples = nullptr;
		// 右通道已处理采样点
		float* rightChannelSamples = nullptr;

		float* cacheReadLeftChannelSamples = nullptr;
		float* cacheReadRightChannelSamples = nullptr;

		//给发音区域存储计算帧样本值的左通道buf
		float leftChannelFrameBuf[4096 * 10] = { 0 };
		//给发音区域存储计算帧样本值的右通道buf
		float rightChannelFrameBuf[4096 * 10] = { 0 };

		//是否开启乐器效果器
		bool isEnableVirInstEffects = true;

		//效果器
		EffectList* effects;

		//内部效果器
		EffectList* innerEffects;

		//是否使用内部效果器
		bool isEnableInnerEffects = false;

		// 合成后的最终采样流
		uint8_t* synthSampleStream = nullptr;
		int synthStreamBufferSize = 0;

		//目前渲染子帧位置
		int childFramePos = 0;

		//当前采样点个数
		int curtSampleCount = 0;

		//当前处理采样的总时间点(单位:秒),这个时间点只是以实际处理采样点数来计算的，
		//不受真实时间流逝影响，比如已处理了2000个点采样点，每个采样点的采样时间为1.0f/44100.0f
		//则当前sec = 2000 * 1.0f/44100.0f = 0.045s = 45ms,
		//即表示目前播放时间已流逝0.045s
		double sec = 0;

		//所有正在发声的区域
		ZoneSounder* totalZoneSounders[100000] = { nullptr };
		//所有正在发声的区域数量
		int totalZoneSounderCount = 0;

		//所有发音是否结束
		bool isSoundEnd = true;

		//是否请求删除合成器
		bool isReqDelete = false;

		Semaphore waitSem;

		int instSoundCount[1000];

		//预设乐器替换
		unordered_map<uint32_t, uint32_t> presetBankReplaceMap;
		Audio* audio = nullptr;
		float soundEndGain = 1;

		//	
		//是否开启缓存
		bool isEnableCache = false;

		//是否为步进播放模式
		bool isStepPlayMode = false;

		double cacheGainStep = 0;
		double cacheGain = 1;
		double dstCacheGain = 1;
		RingBuffer* cacheBuffer = nullptr;

		bool isCacheWriteSoundEnd = true;

		//
		int minCacheSize = 0;
		int reReadCacheSize = 0;
		int maxCacheSize = 0;

		//需要重新开始缓存的尺寸
		int reCacheSize = 0;

		EditorState cachePlayState;
		//缓存状态
		CacheState cacheState = CacheState::CacheStop;

		double curtCachePlaySec = 0;

		//剩余缓存样本的消隐样本处理(使得发音可以渐渐减弱直到消失)
		vector<FadeSamplesInfo> fallSamples;

		ArrayPool<float>* fallSamplesPool = nullptr;

		mutex cacheLocker;

		//设备通道列表
		unordered_map<int, Channel*> deviceChannelMap;

		//
		MidiEditor* midiEditor = nullptr;

		//
		friend class MidiEditorSynther;
		friend class Tau;
		friend class Editor;
		friend class VirInstrument;
		friend class MidiEditor;
		friend class ZoneSounderThread;
		friend class ZoneSounderThreadData;
		friend class KeySounder;
		friend class Channel;
	};

}


#endif
