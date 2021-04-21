#ifndef _MidiEvent_h_
#define _MidiEvent_h_

#include"MidiTypes.h"

namespace ventrue
{
	/// <summary>
	/// midi事件基类
	/// by cymheart, 2020--2021.
	/// </summary>   
	class MidiEvent
	{
	public:
		MidiEventType type = MidiEventType::Unknown;

		/// <summary>
		/// 起始tick
		/// </summary>
		uint32_t startTick = 0;

		/// <summary>
		/// 事件相关通道
		/// </summary>
		int channel = -1;

	};


	/// <summary>
	/// 按下音符事件
	/// </summary>
	class NoteOnEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 结束tick
		/// </summary>
		uint32_t endTick = 0;

		/// <summary>
		/// 音符
		/// </summary>
		int note = 0;

		/// <summary>
		/// 力度
		/// </summary>
		int velocity = 0;

		/// <summary>
		/// 对应的NoteOffEvent
		/// </summary>
		NoteOffEvent* noteOffEvent = nullptr;

		NoteOnEvent()
		{
			type = MidiEventType::NoteOn;
		}
	};


	/// <summary>
	/// 松开音符事件
	/// </summary>
	class NoteOffEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 音符
		/// </summary>
		int note = 0;

		/// <summary>
		/// 力度
		/// </summary>
		int velocity = 0;

		/// <summary>
		/// 对应的NoteOnEvent
		/// </summary>
		NoteOnEvent* noteOnEvent = nullptr;

		NoteOffEvent()
		{
			type = MidiEventType::NoteOff;
		}
	};

	/// <summary>
	/// 控制器事件
	/// </summary>
	class ControllerEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 控制器类型
		/// </summary>
		MidiControllerType ctrlType = MidiControllerType::BalanceMSB;

		/// <summary>
		/// 控制值
		/// </summary>
		int value = 0;

		ControllerEvent()
		{
			type = MidiEventType::Controller;
		}
	};

	/// <summary>
	/// 乐器设置事件
	/// </summary>
	class ProgramChangeEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 乐器值
		/// </summary>
		int value = 0;

		ProgramChangeEvent()
		{
			type = MidiEventType::ProgramChange;
		}
	};

	/// <summary>
	/// 按键力度设置事件
	/// </summary>
	class KeyPressureEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 音符
		/// </summary>
		int note = 0;

		/// <summary>
		/// 力度值
		/// </summary>
		int value = 0;

		KeyPressureEvent()
		{
			type = MidiEventType::KeyPressure;
		}
	};


	/// <summary>
	/// 通道力度设置事件
	/// </summary>
	class ChannelPressureEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 力度值
		/// </summary>
		int value = 0;

		ChannelPressureEvent()
		{
			type = MidiEventType::ChannelPressure;
		}
	};

	/// <summary>
	/// 滑音设置事件
	/// </summary>
	class PitchBendEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 滑音值
		/// </summary>
		int value = 0;

		PitchBendEvent()
		{
			type = MidiEventType::PitchBend;
		}
	};

	/// <summary>
	/// 系统码事件
	/// </summary>
	class SysexEvent : public MidiEvent
	{
	public:
		byte* data = nullptr;
		size_t size = 0;

		SysexEvent()
		{
			type = MidiEventType::Sysex;
		}

		SysexEvent(const SysexEvent& obj)
		{
			CreateData(obj.data, obj.size);
		}

		~SysexEvent();
		void CreateData(byte* d, size_t len);

	};

	/// <summary>
	/// 速度设置事件
	/// 一个四分音符的微秒数
	/// </summary>
	class TempoEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 一个四分音符的微秒数
		/// </summary>
		float microTempo = 0;

		TempoEvent()
		{
			type = MidiEventType::Tempo;
		}
	};

	/// <summary>
	/// 节拍设置事件
	/// </summary>
	class TimeSignatureEvent :public MidiEvent
	{
	public:
		/// <summary>
		/// 分子
		/// </summary>
		int numerator = 4;

		/// <summary>
		/// 分母
		/// denominatorResult = pow(2, denominator);
		/// </summary>
		int denominator = 2;

		/// <summary>
		/// 节拍器一次click的时钟数量
		/// 1个四分音符为24个时钟数量
		/// </summary>
		int metronomeCount = 24;

		/// <summary>
		/// 每个四分音符有几个32分音符
		/// </summary>
		int nCount32ndNotesPerQuarterNote = 8;

		TimeSignatureEvent()
		{
			type = MidiEventType::TimeSignature;
		}
	};

	/// <summary>
	/// 调号设置事件
	/// </summary>
	class KeySignatureEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// sf指明乐曲曲调中升号、降号的数目。
		/// 也就是说，升号数目写成0x，降号数目写成8x
		/// sf = -7: 7 flats 7个降号
		/// sf = -1: 1 flat  1个降号
		/// sf = 0: key of C C调
		/// sf = 1: 1 sharp  1个升调
		/// sf = 7: 7 sharps 7个升调
		/// </summary>
		int sf = 0;

		/// <summary>
		/// 大调小调
		/// 0:major (大调)
		/// 1:minor(小调)
		/// </summary>
		int mi = 0;

		KeySignatureEvent()
		{
			type = MidiEventType::KeySignature;
		}
	};

	/// <summary>
	/// 文本设置事件
	/// </summary>
	class TextEvent : public MidiEvent
	{
	public:
		MidiTextType textType = MidiTextType::GeneralText;
		string text;

		TextEvent()
		{
			type = MidiEventType::Text;
		}
	};


	/// <summary>
	/// 未定义事件
	/// </summary>
	class UnknownEvent : public MidiEvent
	{
	public:
		byte codeType = 0;
		byte* data = nullptr;
		size_t size = 0;

		UnknownEvent()
		{
			type = MidiEventType::Unknown;
		}

		UnknownEvent(const UnknownEvent& obj)
		{
			CreateData(obj.data, obj.size);
		}

		~UnknownEvent();
		void CreateData(byte* d, size_t len);

	};
}

#endif
