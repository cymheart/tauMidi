#ifndef _Ventrue_h_
#define _Ventrue_h_

#include "VentrueTypes.h"
#include "Audio/Audio.h"
#include "Midi/MidiTypes.h"
#include"VentruePool.h"

namespace ventrue
{

	/*
	* Ventrue是一个soundfont合成器
	* by cymheart, 2020--2021.
	*/
	class DLL_CLASS Ventrue
	{
	public:
		Ventrue();
		~Ventrue();

		void Close();

		// 开启声音播放引擎
		void OpenAudio();

		//设置是否使用多线程
		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		void SetUseMulThread(bool use);

		//设置声道模式(立体声，单声道设置)
		void SetChannelOutputMode(ChannelOutputMode outputMode);

		//获取声道模式(立体声，单声道)
		inline ChannelOutputMode GetChannelOutputMode()
		{
			return channelOutputMode;
		}

		//设置渲染品质
		inline void SetRenderQuality(RenderQuality quality)
		{
			renderQuality = quality;
		}

		//获取渲染品质
		inline RenderQuality GetRenderQuality()
		{
			return renderQuality;
		}

		//增加一个解析格式类型
		void AddSoundFontParser(string formatName, SoundFontParser* sfParser);

		//根据格式类型,解析soundfont文件
		void ParseSoundFont(string formatName, string path);

		//启用乐器混响处理
		inline void EnableInstReverb()
		{
			isEnableInstReverb = true;
		}

		//禁用乐器混响处理
		inline void DisableInstReverb()
		{
			isEnableInstReverb = false;
		}

		//是否启用了乐器混响
		inline bool IsEnableInstReverb()
		{
			return isEnableInstReverb;
		}


		//启用乐器合唱处理
		inline void EnableInstChorus()
		{
			isEnableInstChorus = true;
		}

		//禁用乐器合唱处理
		inline void DisableInstChorus()
		{
			isEnableInstChorus = false;
		}

		//是否启用了乐器合唱
		inline bool IsEnableInstChorus()
		{
			return isEnableInstChorus;
		}


		//设置样本处理采样率
		void SetSampleProcessRate(int rate);

		//获取样本处理采样率
		inline float GetSampleProcessRate()
		{
			return sampleProcessRate;
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
			limitRegionSounderCount = count;
		}

		// 增加一个样本到样本列表
		Sample* AddSample(string name, short* samples, size_t size, byte* sm24 = nullptr);

		inline SampleList* GetSampleList()
		{
			return sampleList;
		}

		// 增加一个乐器到乐器列表
		Instrument* AddInstrument(string name);

		InstrumentList* GetInstrumentList()
		{
			return instList;
		}

		// 增加一个预设到预设列表
		Preset* AddPreset(string name, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		inline PresetList* GetPresetList()
		{
			return presetList;
		}

		// 样本绑定到乐器上
		Region* SampleBindToInstrument(Sample* sample, Instrument* inst);

		// 乐器绑定到预设上 
		Region* InstrumentBindToPreset(Instrument* inst, Preset* preset);

		// 帧渲染
		void FrameRender(uint8_t* stream, int len);

		// 设置是否总是使用滑音    
		void SetAlwaysUsePortamento(bool isAlwaysUse);

		// 设置是否使用滑音    
		void SetUsePortamento(bool isUse);

		//设置是否使用单音模式  
		void SetUseMonoMode(bool isUse);

		//是否使用单音模式  
		inline bool UseMonoMode()
		{
			return useMonoMode;
		}

		// 获取是否总是使用滑音   
		inline bool AlwaysUsePortamento()
		{
			return alwaysUsePortamento;
		}

		// 获取是使用滑音   
		inline bool UsePortamento()
		{
			return usePortamento;
		}

		//设置滑音过渡时间(单位:秒)
		void SetPortaTime(float tm);

		//获取滑音过渡时间
		inline float GetPortaTime()
		{
			return portaTime;
		}


		//设置是否使用连音
		void SetUseLegato(bool isUse);

		//是否使用连音
		inline bool UseLegato()
		{
			return useLegato;
		}



		//投递任务
		void PostTask(TaskCallBack taskCallBack, void* data, int delay = 0);

		//投递任务
		void PostTask(Task* task, int delay = 0);

		//投递实时按键操作任务
		void PostRealtimeKeyOpTask(Task* task, int delay = 0);

	private:

		// 获取乐器预设
		Preset* GetInstrumentPreset(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

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
		VirInstrument* EnableVirInstrument(uint32_t deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在，将在虚拟乐器列表中自动创建它
		// 注意如果channel已经被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
		// 而不会同时在一个通道上创建超过1个的虚拟乐器
		VirInstrument* EnableVirInstrument(Preset* preset, Channel* channel);

		//根据设备通道号获取设备通道
		Channel* GetDeviceChannel(uint32_t deviceChannelNum);

		//增加所有soundfont解析器
		void AddSoundFontParsers();

		//增加效果器
		void AddEffect(VentrueEffect* effect);

		// 按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst);

		// 释放按键
		void OffKey(int key, float velocity, VirInstrument* virInst);

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

		/// 停止所有乐器midi的录制
		void StopRecordMidi();

		/// <summary>
		/// 停止指定乐器midi的录制
		/// </summary>
		/// <param name="virInst">如果为null,将停止录制所有乐器</param>
		void StopRecordMidi(VirInstrument* virInst);

		/// <summary>
		/// 生成所有乐器已录制的midi到midiflie object中
		/// </summary>
		/// <returns>midiflie object</returns>
		MidiFile* CreateRecordMidiFileObject();

		/// <summary>
		/// 根据给定的乐器组，生成它们已录制的midi到midiflie object中
		/// </summary>
		/// <param name="virInsts">乐器组</param>
		/// <returns>midiflie object</returns>
		MidiFile* CreateRecordMidiFileObject(VirInstrument** virInsts, int size);

		//保存midiFile到文件
		void SaveMidiFileToDisk(MidiFile* midiFile, string diskPath);

		// 请求帧渲染事件     
		void ReqFrameRender();

		// 渲染每帧音频
		void Render();

		// 处理实时onkey或者offkey事件
		void ProcessRealtimeKeyEvents();

		// 处理播放midi文件事件
		void ProcessMidiEvents();

		//清除通道buffer
		void ClearChannelBuffer();

		//混合所有乐器中的样本到声道buffer中
		void MixVirInstsSamplesToChannelBuffer();

		// 渲染虚拟乐器区域发声     
		void RenderVirInstRegionSound();

		//快速释音超过限制的区域发声
		void FastReleaseRegionSounders();


		//应用效果器到乐器的声道buffer
		void ApplyEffectsToChannelBuffer();

		//合并声道buffer到数据流
		void CombineChannelBufferToStream();

		//  移除已完成所有区域发声处理(采样处理)的KeySounder               
		void RemoveProcessEndedKeySounder();

		//设置所有虚拟乐器关联值
		void SetVirInstsRelationValues();

		//设置虚拟乐器关联值
		void SetVirInstRelationValues(VirInstrument* virInst);

		//调制虚拟乐器参数
		void ModulationVirInstParams(Channel* channel);

		//获取当前距离音频开启的时间
		float GetCurtAudioTime();

		static void FillAudioSample(void* udata, uint8_t* stream, int len);

		//
		static void _FrameRender(Task* ev);

		static bool SounderCountCompare(VirInstrument* a, VirInstrument* b);

	public:
		//渲染发音的时间点回调
		RenderTimeCallBack renderTimeCallBack = nullptr;
		//渲染发音的时间点回调附带数据
		void* renderTimeCallBackData = nullptr;

	private:

		//开始音频处理的起始时间
		clock::time_point* openedAudioTime;

		//此处不能修改为普通bool类型，如果改成普通bool类型
		//由于多线程同时会检测和修改这个量，会导致检测不准确，声音渲染会有几率触发不正常停止
		atomic_bool isFrameRenderCompleted;

		//立体声，单声道选择
		ChannelOutputMode channelOutputMode = ChannelOutputMode::Stereo;

		//渲染品质
		RenderQuality renderQuality = RenderQuality::Fast;

		//效果器
		EffectList* effects;

		// 使用单音模式  
		bool useMonoMode = false;

		// 使用连音  
		//如果前一个按键没有松开，即触发第二个按键，将触发连音
		//此时第二个按键发音将没有attact的过程
		bool useLegato = false;

		// 是否总是使用滑音
		bool alwaysUsePortamento = false;
		// 是否使用滑音
		bool usePortamento = false;
		//滑音过渡时间(单位:秒)
		float portaTime = 0.015f;


		//使用多线程渲染处理声音
		//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
		//当childFrameSampleCount >= 256时，多线程效率会比较高
		//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
		//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
		bool useMulThreads = false;


		//
		SampleList* sampleList = nullptr;
		InstrumentList* instList = nullptr;
		PresetList* presetList = nullptr;
		PresetMap* presetBankDict = nullptr;

		//// 左通道已处理采样点
		float leftChannelSamples[8192 * 10] = { 0 };

		//// 右通道已处理采样点
		float rightChannelSamples[8192 * 10] = { 0 };

		float leftChannelFrameBuf[4096 * 10] = { 0 };
		float rightChannelFrameBuf[4096 * 10] = { 0 };

		//目前渲染子帧位置
		uint32_t childFramePos = 0;


		// 合成后的最终采样流
		uint8_t synthSampleStream[1000000] = { 0 };

		// 合成后的最终采样流
		RingBuffer* synthSampleRingBuffer;

		//所有正在发声的区域
		RegionSounder* totalRegionSounders[100000] = { nullptr };
		//所有正在发声的区域数量
		int totalRegionSounderCount = 0;
		//发声区域最大限制数量
		int limitRegionSounderCount = 600;


		TaskProcesser* taskProcesser = nullptr;
		TaskProcesser* realtimeKeyOpTaskProcesser = nullptr;
		RegionSounderThread* regionSounderThreadPool = nullptr;


		//使用中的虚拟乐器列表
		VirInstList* virInstList = nullptr;
		//使用中的虚拟乐器列表
		vector<VirInstrument*>* virInsts = nullptr;

		//是否启用乐器混响处理
		bool isEnableInstReverb = false;

		//是否启用乐器合唱处理
		bool isEnableInstChorus = false;

		//所有乐器发音是否结束
		bool isVirInstSoundEnd = false;

		//所有发音是否结束
		bool isSoundEnd = false;


		//声音处理时的采样周期，不同于样本采样率
		//样本采样率和声音处理采样率的频率矫正倍率计算为: rateAdjustMul = sampleRate / sampleProcessRate
		//如每秒钟处理采样点44100个采样点
		float sampleProcessRate = 44100;

		// 帧采样数量
		int frameSampleCount = 1024;

		//子帧细分采样数量,会影响midi发音的流畅度和每个按键播放时间的精度
		int childFrameSampleCount = 64;

		//采样率的倒数，表示1个采样点所花费的时间
		float invSampleProcessRate = 1.0f / 44100.0f;

		//当前处理采样的总时间点(单位:秒),这个时间点只是以实际处理采样点数来计算的，
		//不受真实时间流逝影响，比如已处理了2000个点采样点，每个采样点的采样时间为1.0f/44100.0f
		//则当前sec = 2000 * 1.0f/44100.0f = 0.045s = 45ms,
		//即表示目前播放时间已流逝0.045s
		float sec = 0;

		//当前采样点个数
		int curtSampleCount = 0;


		//音源解析格式
		SoundFontParserMap* sfParserMap;

		//
		Audio* audio = nullptr;

		RealtimeKeyEventList* realtimeKeyEventList = nullptr;
		MidiPlayList* midiPlayList = nullptr;
		MidiFileList* midiFileList = nullptr;

		//设备通道列表
		ChannelMap* deviceChannelMap = nullptr;

		mutex* cmdLock;

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

		//
		friend class VentrueCmd;
		friend class VirInstrument;
		friend class MidiPlay;
		friend class RegionSounderThread;
	};
}

#endif
