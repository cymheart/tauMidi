#ifndef _MidiFile_h_
#define _MidiFile_h_

#include "MidiTypes.h"
#include"MidiTrack.h"

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

		//设置轨道通道合并模式
		inline void SetTrackChannelMergeMode(TrackChannelMergeMode mode)
		{
			mergeMode = mode;
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

		inline MidiTrackList* GetTrackList()
		{
			return &midiTrackList;
		}

		//清空midiTracks列表,但并不真正删除事件
		void ClearMidiTrackList();


		//增加一个Midi轨道
		void AddMidiTrack(MidiTrack* midiTrack);

		// 解析文件到可识别数据结构
		void Parse(string filePath);

		//生成midi格式内存数据
		void CreateMidiFormatMemData();

		//保存midi格式内存数据到文件
		void SaveMidiFormatMemDataToDist(string saveFilePath);

		//获取全局事件列表
		list<MidiEvent*>* GetGolbalEventList();



	private:

		// 是否两个通道具有相同的乐器改变事件
		//即合并任何多个轨道上相同的通道事件到一个轨道，删除被合并轨道上对应通道的事件
		//如果一个轨道上所有通道事件都被合并完了，那这个轨道也会被相应删除
		void SetChannelSameProgram();

		//是否两个通道可以具有相同的乐器改变事件
		//只能其中一个有乐器改变事件，两者都有或都没有都不符合自动合并的要求
		bool IsSameProgramChannel(vector<MidiEvent*>* eventListA, vector<MidiEvent*>* eventListB);

		//每个通道midi事件分配到每一个轨道
		void PerChannelMidiEventToPerTrack();

		//寻找轨道默认乐器改变事件
		void FindTracksDefaultProgramChangeEvent();

		//去除前后值相同,以及后值覆盖前值的全局事件
		void RemoveSameAndOverrideGlobalEvents(list<MidiEvent*>* globalMidiEvents);

		//解析内核
		bool ParseCore();
		//解析头块
		bool ParseHeaderChunk();
		//解析轨道块
		int ParseTrackChuck();
		int ParseEvent(MidiTrack& track);

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

		short ReadInt16(ByteStream& reader);
		uint32_t ReadInt32(ByteStream& reader);
		uint32_t Read3BtyesToInt32(ByteStream& reader);
		void WriteInt32To3Btyes(ByteStream& writer, int32_t value);

		static bool MidiEventTickCompare(MidiEvent* a, MidiEvent* b);

	private:
		ByteStream* midiReader = nullptr;
		ByteStream* midiWriter = nullptr;

		bool isLittleEndianSystem = true;

		//轨道通道合并模式
		TrackChannelMergeMode mergeMode = TrackChannelMergeMode::AutoMerge;

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


		MidiTrackList midiTrackList;
		list<MidiEvent*> golbalEvents;

		int programChangeA = 0;
		int programChangeB = 0;


	};
}

#endif
