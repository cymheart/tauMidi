#ifndef _Synther_h_
#define _Synther_h_

#include "Synth/TauTypes.h"
#include "Synth/Editor/EditorTypes.h"
#include"Midi/MidiTypes.h"
#include"FX/TauEffect.h"
#include"FX/EffectList.h"
#include"Audio/Audio.h"
#include"task/MsgProcesser.h"
using namespace tauFX;

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

		void SetMainSynther(bool isMain)
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

		//删除乐器
		void DelVirInstrumentTask(VirInstrument* virInst);

		// 打开虚拟乐器
		void OnVirInstrumentTask(VirInstrument* virInst, bool isFade);

		// 关闭虚拟乐器
		void OffVirInstrumentTask(VirInstrument* virInst, bool isFade);

		// 获取虚拟乐器列表的备份
		vector<VirInstrument*>* TakeVirInstrumentListTask();

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
		void CombineSynthersFrameBufsTask();
		void AddAssistSyntherTask(Semaphore* waitSem, Synther* assistSynther);
		void RemoveAssistSyntherTask(Semaphore* waitSem, Synther* assistSynther);

		// 请求删除合成器
		void ReqDeleteTask();


	private:
		static void FillAudioSample(void* udata, uint8_t* stream, int len);

		static void _CombineSynthersFrameBufsTask(Task* ev);
		static void _AddAssistSyntherTask(Task* ev);
		static void _RemoveAssistSyntherTask(Task* ev);
		static void _ReqDeleteTaskTask(Task* ev);
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
		static void _DelInstrumentTask(Task* ev);
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

		void _CombineSynthersFrameBufs();
		void AddAssistSynther(Synther* assistSynther);
		void RemoveAssistSynther(Synther* assistSynther);
		void CreateAssistSyntherSem();

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

		//合并辅助合成器buffer到主buffer中
		void CombineAssistToMainBuffer();

		//应用效果器到乐器的声道buffer
		void ApplyEffectsToChannelBuffer();

		//设置过渡效果深度信息
		void SettingFadeEffectDepthInfo(float curtEffectDepth, FadeEffectDepthInfo& fadeEffectDepthInfo);

		void CombineSynthersFrameBufs();

		//合并声道buffer到数据流
		void CombineChannelBufferToStream();

		// 渲染虚拟乐器区域发声     
		void RenderVirInstRegionSound();

		//快速释音超过限制的区域发声
		void FastReleaseRegionSounders();

		//快速释音超过限制的区域发声2
		//比例算法
		void FastReleaseRegionSounders2();

		//  移除已完成所有区域发声处理(采样处理)的KeySounder               
		void RemoveProcessEndedKeySounder();

		static bool SounderCountCompare(VirInstrument* a, VirInstrument* b);

		// 请求帧渲染事件
		void ReqRender();
		static void RenderTask(Task* ev);
		// 渲染每帧音频
		virtual void Render();

		// 辅助帧渲染
		void AssistFrameRender();
		static void ThreadAssistFrameRender(void* param);


		// 帧渲染
		void FrameRender(uint8_t* stream, int len);

		// 录制所有乐器弹奏为midi
		void RecordMidi();

		// 录制指定乐器弹奏为midi
		void RecordMidi(VirInstrument* virInst);

		/// 停止所有乐器midi的录制
		void StopRecordMidi();

		/// <summary>
		/// 停止指定乐器midi的录制
		/// </summary>
		/// <param name="virInst">如果为null,将停止录制所有乐器</param>
		void StopRecordMidi(VirInstrument* virInst);

		// 获取录制的midi轨道
		vector<MidiTrack*>* TakeRecordMidiTracks(
			VirInstrument** virInst, int size, float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos);


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

		// 帧采样数量
		int frameSampleCount = 1024;

		// 左通道已处理采样点
		float leftChannelSamples[8192 * 10] = { 0 };
		// 右通道已处理采样点
		float rightChannelSamples[8192 * 10] = { 0 };

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

		float curtReverbDepth = 0;

		//过渡区域混音深度信息
		FadeEffectDepthInfo fadeReverbDepthInfo;

		//过渡区域和声深度信息
		FadeEffectDepthInfo fadeChorusDepthInfo;

		// 区域混音处理
		Reverb* regionReverb = nullptr;

		// 区域和声处理
		Chorus* regionChorus = nullptr;

		// 合成后的最终采样流
		uint8_t synthSampleStream[1000000] = { 0 };


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

		//所有乐器发音是否结束
		bool isVirInstSoundEnd = true;
		//所有发音是否结束
		bool isSoundEnd = true;

		//是否请求删除合成器
		bool isReqDelete = false;

		Semaphore waitDelSem;

		int instSoundCount[1000];

		//预设乐器替换
		unordered_map<uint32_t, uint32_t>* presetBankReplaceMap = nullptr;

		//
		Audio* audio = nullptr;

		//
		bool isMainSynther = false;
		atomic_int computedFrameBufSyntherCount;
		vector<Synther*> assistSynthers;

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
