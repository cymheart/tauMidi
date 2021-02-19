#ifndef _MidiFile_h_
#define _MidiFile_h_

#include "MidiTypes.h"

namespace ventrue
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

		// 获取文件格式
		MidiFileFormat GetFormat()
		{
			return format;
		}

		//设置格式
		void SetFormat(MidiFileFormat format)
		{
			this->format = format;
		}

		// 获取一个四分音符的tick数
		float GetTickForQuarterNote()
		{
			return tickForQuarterNote;
		}

		// 设置一个四分音符的tick数
		void SetTickForQuarterNote(float tickForQuarterNote)
		{
			this->tickForQuarterNote = (short)tickForQuarterNote;
		}

		MidiTrackList* GetTrackList()
		{
			return midiTrackList;
		}

		//增加一个Midi轨道
		void AddMidiTrack(MidiTrack* midiTrack);

		// 解析文件到可识别数据结构
		void Parse(string filePath);

		//生成midi格式内存数据
		void CreateMidiFormatMemData();

		//保存midi格式内存数据到文件
		void SaveMidiFormatMemDataToDist(string saveFilePath);

	private:
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

	private:
		ByteStream* midiReader = nullptr;
		ByteStream* midiWriter = nullptr;

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


		MidiTrackList* midiTrackList = nullptr;

	};
}

#endif
