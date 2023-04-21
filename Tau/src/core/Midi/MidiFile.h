#ifndef _MidiFile_h_
#define _MidiFile_h_

#include "MidiTypes.h"
#include"MidiTrack.h"
#include <chrono>

namespace tau
{
	/// <summary>
	/// Midi文件解析类
	/// by cymheart, 2020--2021.
	/// </summary>
	class MidiFile
	{
	public:
		MidiFile();
		~MidiFile();

		//设置是否开启MidiEvent数量优化
		inline void SetEnableMidiEventCountOptimize(bool enable)
		{
			enableMidiEventCountOptimize = enable;
		}

		//保持相同StartTick按键事件的数量 (默认值:-1 无限制)
		inline void SetKeepSameStartTickNoteOnEventsCount(int count)
		{
			keepSameStartTickNoteOnEventsCount = count;
		}

		//设置是否开启拷贝相同通道控制事件(默认:开启)
		inline void SetEnableCopySameChannelControlEvents(bool enable)
		{
			enableCopySameChannelControlEvents = enable;
		}

		//设置是否启用解析极限时间(默认值:2s)
		inline void SetEnableParseLimitTime(bool enable, float limitSec = 2.0)
		{
			isEnableParseLimitTime = enable;
			limitParseSec = limitSec;
		}

		//判断是否全部解析
		inline bool IsFullParsed()
		{
			return isFullParsed;
		}


		// 获取文件格式
		inline MidiFileFormat GetFormat()
		{
			return format;
		}

		//设置格式
		inline void SetFormat(MidiFileFormat format)
		{
			this->format = format;
		}

		// 获取一个四分音符的tick数
		inline float GetTickForQuarterNote()
		{
			return tickForQuarterNote;
		}

		// 设置一个四分音符的tick数
		inline void SetTickForQuarterNote(float tickForQuarterNote)
		{
			this->tickForQuarterNote = (short)tickForQuarterNote;
		}

		inline vector<MidiTrack*>* GetTrackList()
		{
			return &midiTracks;
		}

		//清空midiTracks列表,但并不真正删除事件
		void ClearMidiTrackList();


		//增加一个Midi轨道
		void AddMidiTrack(MidiTrack* midiTrack);

		// 解析文件到可识别数据结构
		bool Parse(string filePath);

		//生成midi格式内存数据
		void CreateMidiFormatMemData();

		//保存midi格式内存数据到文件
		void SaveMidiFormatMemDataToDist(string saveFilePath);

		//获取全局事件列表
		LinkedList<MidiEvent*>* GetGolbalEventList();


		//打开解析
		void OpenParse()
		{
			isStopParse = false;
		}

		//停止解析
		void StopParse()
		{
			isStopParse = true;
		}

		//是否停止解析
		inline bool IsStopParse()
		{
			return isStopParse;
		}

	private:

		//每个通道midi事件分配到每一个轨道
		void PerChannelMidiEventToPerTrack();

		//复制轨道全局事件到新的轨道
		void CopyTrackGlobalEventsForNewTrack(int oldTrackIdx, int newTrackIdx);

		//拷贝相同通道控制事件
		void CopySameChannelControlEvents();

		//寻找轨道默认乐器改变事件
		void FindTracksDefaultProgramChangeEvent();

		//增加默认全局TimeSignatureEvent， KeySignatureEvent, TempoEvent事件
		void AddDefaultGlobalEvents(LinkedList<MidiEvent*>* globalMidiEvents);

		//去除前后值相同,以及后值覆盖前值的全局事件
		void RemoveSameAndOverrideGlobalEvents(LinkedList<MidiEvent*>* globalMidiEvents);

		//解析内核
		bool ParseCore();
		//解析头块
		bool ParseHeaderChunk();
		//解析轨道块
		int ParseTrackChuck(int trackIdx);
		int ParseEvent(MidiTrack& track, int trackIdx);

		//跳过解析的事件
		int PassParseEvent(MidiTrack& track);

		//生成头块
		bool CreateHeaderChunk();
		//生成轨道块
		int CreateTrackChuck(int trackIdx);
		//生成事件数据
		int CreateEventData(MidiEvent& midiEvent);

		//读取变长值
		uint32_t ReadDynamicValue(ByteStream& reader, int maxByteCount = 4);
		//写入变长值
		void WriteDynamicValue(ByteStream& writer, int32_t value);

		short ReadInt16(ByteStream& reader) const;
		uint32_t ReadInt32(ByteStream& reader) const;
		uint32_t Read3BtyesToInt32(ByteStream& reader) const;
		void WriteInt32To3Btyes(ByteStream& writer, int32_t value) const;

		static bool MidiEventTickCompare(MidiEvent* a, MidiEvent* b);

	private:
		ByteStream* midiReader = nullptr;
		ByteStream* midiWriter = nullptr;

		//是否停止解析
		bool isStopParse = false;

		//是否开启MidiEvent数量优化
		bool enableMidiEventCountOptimize = false;

		//保持相同StartTick按键事件的数量 (默认值:15 )
		int keepSameStartTickNoteOnEventsCount = 15;

		//是否开启拷贝相同通道控制事件
		bool enableCopySameChannelControlEvents = true;

		bool isLittleEndianSystem = true;

		// 最后解析的事件号
		byte lastParseEventNum = 0;

		// 最后解析的事件作用通道
		int lastParseEventChannel = 0;

		// 当前解析到的所有detlaTime相加的tick数量
		uint32_t curtParseTickCount = 0;

		// 格式
		MidiFileFormat format = MidiFileFormat::SyncTracks;

		// 音轨数量
		short trackCount = 0;

		// 一个四分音符的tick数
		short tickForQuarterNote = 480;

		//
		//是否启用解析极限时间
		bool isEnableParseLimitTime = false;
		//极限解析时间(默认值:2s)
		float limitParseSec = 2.0f;

		//是否全部解析
		bool isFullParsed = true;


		chrono::high_resolution_clock::time_point startParseTime;
		float perTrackParseSec = 0;
		float curtTrackParseSec = 0;

		vector<MidiTrack*> midiTracks;
		list<MidiEvent*> golbalEvents;

		int programChangeA = 0;
		int programChangeB = 0;


	};
}

#endif
