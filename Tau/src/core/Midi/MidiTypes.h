#ifndef _MidiTypes_h_
#define _MidiTypes_h_

#include"scutils/ByteStream.h"

using namespace scutils;

namespace tau
{
	enum class MidiControllerType;

	class MidiEvent;
	class NoteOnEvent;
	class NoteOffEvent;
	class MidiTrack;

	using MidiEventList = vector<MidiEvent*>;
	using MidiTrackList = vector<MidiTrack*>;
	using MidiControllerTypeList = vector<MidiControllerType>;



	/// <summary>
	/// Midi控制器类型
	/// </summary>
	enum class MidiControllerType
	{
		CC_None = -1,
		BankSelectMSB,
		ModulationWheelMSB,
		BreathControlMSB,
		CC_003,
		FootControllerMSB,
		PortamentoTimeMSB,
		DataEntryMSB,
		ChannelVolumeMSB,
		BalanceMSB,
		CC_009,
		PanMSB,
		ExpressionControllerMSB,
		EffectControl1MSB,
		EffectControl2MSB,
		CC_014,
		CC_015,
		GeneralPurposeController1MSB,
		GeneralPurposeController2MSB,
		GeneralPurposeController3MSB,
		GeneralPurposeController4MSB,
		CC_020,
		CC_021,
		CC_022,
		CC_023,
		CC_024,
		CC_025,
		CC_026,
		CC_027,
		CC_028,
		CC_029,
		CC_030,
		CC_031,
		BankSelectLSB,
		ModulationWheelLSB,
		BreathControlLSB,
		CC_035,
		FootControllerLSB,
		PortamentoTimeLSB,
		DataEntryLSB,
		ChannelVolumeLSB,
		BalanceLSB,
		CC_041,
		PanLSB,
		ExpressionControllerLSB,
		EffectControl1LSB,
		EffectControl2LSB,
		CC_046,
		CC_047,
		GeneralPurposeController1LSB,
		GeneralPurposeController2LSB,
		GeneralPurposeController3LSB,
		GeneralPurposeController4LSB,
		CC_052,
		CC_053,
		CC_054,
		CC_055,
		CC_056,
		CC_057,
		CC_058,
		CC_059,
		CC_060,
		CC_061,
		CC_062,
		CC_063,
		SustainPedalOnOff,
		PortamentoOnOff,
		SustenutoOnOff,
		SoftPedalOnOff,
		LegatoFootSwitch,
		Hold2,
		SoundController1Variation,
		SoundController2Timbre,
		SoundController3ReleaseTime,
		SoundController4AttackTime,
		SoundController5Brightness,
		SoundController6,
		SoundController7,
		SoundController8,
		SoundController9,
		SoundController10,
		GeneralPurposeController5LSB,
		GeneralPurposeController6LSB,
		GeneralPurposeController7LSB,
		GeneralPurposeController8LSB,
		PortamentoControl,
		CC_085,
		CC_086,
		CC_087,
		CC_088,
		CC_089,
		CC_090,
		Effects1DepthReverbSend,
		Effects2DepthTremoloDepth,
		Effects3DepthChorusSend,
		Effects4DepthCelesteDepth,
		Effects5DepthPhaserDepth,
		DataEntryInc,
		DataEntryDec,
		NRPNLSB,
		NRPNMSB,
		RPNLSB,
		RPNMSB,
		CC_102,
		CC_103,
		CC_104,
		CC_105,
		CC_106,
		CC_107,
		CC_108,
		CC_109,
		CC_110,
		CC_111,
		CC_112,
		CC_113,
		CC_114,
		CC_115,
		CC_116,
		CC_117,
		CC_118,
		CC_119,
		AllSoundOff,
		ResetAllControllers,
		LocalControlOnOff,
		AllNotesOff,
		OmniModeOff,
		OmniModeOn,
		PolyModeOff,
		PolyModeOn
	};


	/// <summary>
	/// Midi事件类型
	/// </summary>
	enum class MidiEventType
	{
		/// <summary>
		/// 未定义
		/// </summary>
		Unknown = -1,

		/// <summary>
		/// 按下音符
		/// </summary>
		NoteOn,

		/// <summary>
		/// 松开音符
		/// </summary>
		NoteOff,

		/// <summary>
		/// 速度设置
		/// </summary>
		Tempo,

		/// <summary>
		/// 节拍设置
		/// </summary>
		TimeSignature,

		/// <summary>
		/// 音调符号
		/// </summary>
		KeySignature,

		/// <summary>
		/// 控制器
		/// </summary>
		Controller,

		/// <summary>
		/// 乐器更换
		/// </summary>
		ProgramChange,

		/// <summary>
		/// 音符触后力度
		/// </summary>
		KeyPressure,

		/// <summary>
		/// 通道力度
		/// </summary>
		ChannelPressure,

		/// <summary>
		/// 滑音
		/// </summary>
		PitchBend,

		/// <summary>
		/// 文本
		/// </summary>
		Text,

		/// <summary>
		/// 系统码
		/// </summary>
		Sysex,

		/// <summary>
		/// 元事件
		/// </summary>
		Meta,
	};


	/// <summary>
	/// midi文本类型 
	/// </summary>
	enum class MidiTextType
	{
		GeneralText,
		Copyright,
		TrackName,
		InstrumentName,
		Lyric,
		Marker,
		Comment
	};

	/// <summary>
   /// midi文件格式
   /// </summary>
	enum class MidiFileFormat
	{
		/// <summary>
		/// 单轨
		/// </summary>
		SingleTrack,

		/// <summary>
		/// 同步多轨道
		/// </summary>
		SyncTracks,

		/// <summary>
		/// 异步多轨道
		/// </summary>
		AsyncTracks,
	};

	/// <summary>
	/// 轨道通道合并模式
	/// </summary>
	enum TrackChannelMergeMode
	{
		//自动判断合并
		AutoMerge,
		//总是合并
		AlwaysMerge,
		//从不合并
		NoMerge
	};


	/// <summary>
	/// 弹奏方式
	/// </summary>
	enum MidiEventPlayType
	{
		//左手
		LeftHand,
		//右手
		RightHand,
		//背景
		Background,
		//自定
		Custom
	};
}

#endif
