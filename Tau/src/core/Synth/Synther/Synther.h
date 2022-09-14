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

namespace tau
{
	// Synther类型
	enum class SyntherType
	{
		//未知
		Unknown,
		//实时
		Realtime,
		//可midi编辑
		MidiEditor
	};

	enum class CacheState
	{
		CacheStop,
		//停止中
		CacheStoping,

		//缓存并读取中
		CachingAndRead,
		//缓存中但不读取
		CachingNotRead,
		//仅读取
		OnlyRead,

		//缓存中但暂停
		CachingPauseRead,
		//暂停等待读取
		PauseWaitRead,

		CacheReadTail,

		//进入步进播放模式
		EnterStep,

		//离开步进播放模式
		LeaveStep,

		Remove,
		Removing
	};


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

		virtual SyntherType GetType() { return SyntherType::Unknown; }

		//投递任务
		void PostTask(TaskCallBack taskCallBack, void* data, int delay = 0);

		//投递任务
		void PostTask(Task* task, int delay = 0);

	public:

		virtual void Open();
		virtual void Close();

		float GetSampleProcessRate();

		//是否已开启
		inline bool IsOpened()
		{
			return isOpened;
		}

		//设置为主合成器
		inline void SetMainSynther(bool isMain)
		{
			isMainSynther = isMain;
		}

		//添加替换乐器
		void AppendReplaceInstrumentTask(
			int orgBankMSB, int orgBankLSB, int orgInstNum,
			int repBankMSB, int repBankLSB, int repInstNum);

		//移除替换乐器
		void RemoveReplaceInstrumentTask(int orgBankMSB, int orgBankLSB, int orgInstNum);


		//增加效果器
		void AddEffectTask(TauEffect* effect);

		//设置是否开启所有乐器效果器
		void SetEnableAllVirInstEffectsTask(bool isEnable);


		// 设置乐器Bend值
		void SetVirInstrumentPitchBendTask(VirInstrument* virInst, int value);

		// 设置乐器PolyPressure值
		void SetVirInstrumentPolyPressureTask(VirInstrument* virInst, int key, int pressure);


		// 设置乐器Midi控制器值
		void SetVirInstrumentMidiControllerValueTask(VirInstrument* virInst, MidiControllerType midiController, int value);

		VirInstrument* EnableVirInstrumentTask(Channel* channel, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		void SetVirInstrumentProgramTask(VirInstrument* virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 移除乐器
		void RemoveVirInstrumentTask(VirInstrument* virInst, bool isFade);

		// 移除所有乐器
		void RemoveAllVirInstrumentTask(bool isFade);

		// 打开虚拟乐器
		void OnVirInstrumentTask(VirInstrument* virInst, bool isFade);

		// 关闭虚拟乐器
		void OffVirInstrumentTask(VirInstrument* virInst, bool isFade);

		// 获取虚拟乐器列表的备份
		vector<VirInstrument*>* TakeVirInstrumentListTask();

		//获取采样流的频谱
		int GetSampleStreamFreqSpectrums(int channel, double* outLeft, double* outRight);

		// 录制所有乐器弹奏为midi
		void RecordMidiTask();

		/// <summary>
		/// 录制指定乐器弹奏为midi
		/// </summary>
		/// <param name="virInst">如果为null,将录制所有乐器</param>
		void RecordMidiTask(VirInstrument* virInst);

		/// <summary>
		/// 停止所有乐器当前midi录制
		/// </summary>
		void StopRecordMidiTask();

		/// <summary>
		/// 停止录制指定乐器弹奏midi
		/// </summary>
		/// <param name="virInst">如果为null,将录制所有乐器</param>
		void StopRecordMidiTask(VirInstrument* virInst);

		// 获取录制的midi轨道
		vector<MidiTrack*>* TakeRecordMidiTracksTask(float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos);

		// 获取录制的midi轨道
		vector<MidiTrack*>* TakeRecordMidiTracksTask(VirInstrument** virInst, int size, float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos);

	protected:

		void SlaveSyntherProcessCompletedTask();
		void AddSlaveSyntherTask(Semaphore* waitSem, Synther* slaveSynther);
		void RemoveSlaveSyntherTask(Semaphore* waitSem, Synther* slaveSynther);

	private:


		static void FillAudioSample(void* udata, uint8_t* stream, int len);

		static void _SlaveSyntherProcessCompletedTask(Task* ev);
		static void _AddSlaveSyntherTask(Task* ev);
		static void _RemoveSlaveSyntherTask(Task* ev);

		static void _AppendReplaceInstrumentTask(Task* ev);
		static void _RemoveReplaceInstrumentTask(Task* ev);
		static void _AddEffectTask(Task* ev);
		static void _SetVirInstrumentPitchBendTask(Task* ev);
		static void _SetVirInstrumentPolyPressureTask(Task* ev);
		static void _SetEnableAllVirInstEffectsTask(Task* ev);
		static void _SetVirInstrumentlMidiControllerValueTask(Task* ev);
		static void _EnableInstrumentTask(Task* ev);
		static void _SetVirInstrumentProgramTask(Task* ev);
		static void _RemoveInstrumentTask(Task* ev);
		static void _RemoveAllInstrumentTask(Task* ev);
		static void _OnInstrumentTask(Task* ev);
		static void _OffInstrumentTask(Task* ev);
		static void _TakeVirInstrumentListTask(Task* ev);

		static void _RecordMidiTask(Task* ev);
		static void _StopRecordMidiTask(Task* ev);
		static void _TakeRecordMidiTracksTask(Task* ev);


	protected:

		// 开启声音播放引擎
		void OpenAudio();

		void ReqDelete();


		void AddSlaveSynther(Synther* slaveSynther);
		void RemoveSlaveSynther(Synther* removeSlaveSynther);

		//获取当前时间点
		inline double GetCurtSec()
		{
			return sec;
		}

		virtual double GetPlaySec()
		{
			return curtCachePlaySec;
		}

		//获取midi结束时间(单位:秒)
		virtual double GetEndSec()
		{
			return 0;
		}

		//设置当前缓存播放时间
		void SetCurtCachePlaySec(double sec);

		//设置缓存状态
		void SetCachePlayState(EditorState s);

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

		// 删除虚拟乐器
		virtual void DelVirInstrument(VirInstrument* virInst);

		//打开虚拟乐器
		void OnVirInstrument(VirInstrument* virInst, bool isFade = true);

		//根据通道打开相关虚拟乐器
		void OnVirInstrument(Channel* channel, bool isFade = true);

		//关闭虚拟乐器
		void OffVirInstrument(VirInstrument* virInst, bool isFade = true);

		//根据通道关闭相关虚拟乐器
		void OffVirInstrument(Channel* channel, bool isFade = true);

		//根据通道关闭相关虚拟乐器所有按键
		void OffVirInstrumentAllKeys(Channel* channel);

		//根据指定通道获取关连虚拟乐器
		VirInstrument* GetVirInstrument(Channel* channel);

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


		//检测由发音是否完全结束
		bool _TestSoundEnd();

		//检测由发音是否完全结束
		inline void TestSoundEnd()
		{
			isSoundEnd = _TestSoundEnd();
		}

		//合成采样buffer
		void SynthSampleBuffer();
		void MainSynthBuffer();
		void SynthSlavesBuffer();

		//合并声道buffer到数据流
		void CombineChannelBufferToStream();

		void CreateChannelSamplesFreqSpectrum();

		// 渲染虚拟乐器区域发声     
		void RenderVirInstRegionSound();

		//移除需要删除的乐器
		void RemoveNeedDeleteVirInsts(bool isDirectRemove = false);

		//快速释音超过限制的区域发声
		void FastReleaseRegionSounders();

		//快速释音超过限制的区域发声2
		//比例算法
		void FastReleaseRegionSounders2();

		//  移除已完成所有区域发声处理(采样处理)的KeySounder               
		void RemoveProcessEndedKeySounder();

		//等待发声结束移除对应的从合成器
		void WaitSoundEndRemoveSlaveSynthers();

		inline void SyntherProcessCompleted()
		{
			processedSyntherCount--;
			if (processedSyntherCount <= 0)
				MainSynthBuffer();
		}

		static bool SounderCountCompare(VirInstrument* a, VirInstrument* b);

		// 请求帧渲染事件
		void ReqRender(float delay = 0);
		static void RenderTask(Task* ev);
		// 渲染每帧音频
		virtual void Render()
		{
		}

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

		void HanningWin(double* data, int len);


		/////////////////////////////////////////////////////
		//缓存处理
		void CacheProcess();

		void CachePlay();
		void CachePause();
		void CacheStop(bool isReset = false);
		bool CacheGoto(double sec, bool isMustReset = false);
		inline void CacheReset()
		{
			CacheGoto(curtCachePlaySec, true);
		}

		//在render中缓存处理
		void CacheRender();
		//在主合成器中的缓存处理
		void CacheReadToMain(Synther* mainSynther);

		//所有从合成器完成了数据采样，把从合成器采样数据合并到主合成器中
		//此时仅合成从合成器中的buffer，当合成所有从buffer后，主合成器再次开启缓存
		void CacheSynthSlavesBuffer();

		void CacheSynthToMain(Synther* mainSynther);

		//生成渐降的samples
		void CreateFallCacheSamples();

		//生成渐升的CacheGain
		inline void CreateRiseCacheGain();

		//缓存处理
		void CacheWrite();
		void CacheRead(Synther* mainSynther);
		void CacheReadFallSamples(Synther* mainSynther);
		void CacheRead();

		void CacheEnterStepPlayMode();

		//获取缓存播放时间
		inline double GetCachePlaySec()
		{
			return curtCachePlaySec;
		}

		inline EditorState GetCachePlayState()
		{
			return cachePlayState;
		}

		inline void TestCacheWriteSoundEnd()
		{
			isCacheWriteSoundEnd = _TestSoundEnd();
		}

		void TestCacheSoundEnd();


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

		inline float GetLeftCacheChannelSamples(int idx)
		{
			return cacheReadLeftChannelSamples[idx] * cacheGain;
		}

		inline float GetRightCacheChannelSamples(int idx)
		{
			return cacheReadRightChannelSamples[idx] * cacheGain;
		}

		virtual bool CanCache()
		{
			return true;
		}

		void ShowCacheInfo();
		static void _ShowCacheInfoTask(Task* task);

	protected:

		Tau* tau;
		TaskProcesser* taskProcesser = nullptr;
		RegionSounderThread* regionSounderThreadPool = nullptr;

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


		//
		double* cacheLeftChannelSampleStream = nullptr;
		double* cacheRightChannelSampleStream = nullptr;

		double* channelFFTState = nullptr;

		double* leftChannelFreqSpectrums = nullptr;
		double* rightChannelFreqSpectrums = nullptr;

		int freqSpectrumsCount = 0;
		int cacheSampleStreamIdx = 0;
		mutex createFreqSpecLocker;

		//
		Semaphore* taskWaitSem;

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
		RegionSounder* totalRegionSounders[100000] = { nullptr };
		//所有正在发声的区域数量
		int totalRegionSounderCount = 0;

		//所有发音是否结束
		bool isSoundEnd = true;

		//是否请求删除合成器
		bool isReqDelete = false;

		Semaphore waitSem;

		int instSoundCount[1000];

		//预设乐器替换
		unordered_map<uint32_t, uint32_t>* presetBankReplaceMap = nullptr;

		//
		Audio* audio = nullptr;

		Synther* mainSynther = nullptr;

		//是否为主合成器
		bool isMainSynther = false;

		//是否发声结束删除
		bool isSoundEndRemove = false;

		//完成缓存处理的合成器数量
		int cachedSyntherCount = 0;
		int processedSyntherCount = 0;
		vector<Synther*> slaveSynthers;

		//	
		//是否开启缓存
		bool isEnableCache = true;

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
		int reCacheSize = 0;

		EditorState cachePlayState;
		CacheState state = CacheState::CacheStop;

		double curtCachePlaySec = 0;

		vector<FadeSamplesInfo> fallSamples;
		ArrayPool<float>* fallSamplesPool = nullptr;


		mutex cacheLocker;
		Semaphore* cmdWait;

		//

		friend class MidiEditorSynther;
		friend class Tau;
		friend class Editor;
		friend class VirInstrument;
		friend class MidiEditor;
		friend class RegionSounderThread;
		friend class RegionSounderThreadData;
		friend class KeySounder;
		friend class Channel;
	};

}


#endif
