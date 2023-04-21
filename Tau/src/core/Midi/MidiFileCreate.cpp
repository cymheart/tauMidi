#include"MidiFile.h"
#include <iostream>
#include <fstream>
#include"MidiTrack.h"
#include"MidiEvent.h"
#include <algorithm>
using namespace std;

namespace tau
{
	//保存midi格式内存数据到文件
	void MidiFile::SaveMidiFormatMemDataToDist(string saveFilePath)
	{
		try
		{
			ofstream outFile;
			outFile.open(saveFilePath, ios::out | ios::binary);
			outFile.write((const char*)midiWriter->begin(), midiWriter->getWriteCursor());
			outFile.close();
		}
		catch (exception)
		{
			cout << saveFilePath << "midi文件写入出错!" << endl;
		}
	}

	//生成midi格式内存数据
	void MidiFile::CreateMidiFormatMemData()
	{
		midiWriter->clear();
		midiWriter->write(nullptr, 1024 * 50);

		CreateHeaderChunk();

		for (int i = 0; i < midiTracks.size(); i++)
		{
			curtParseTickCount = 0;
			CreateTrackChuck(i);
		}
	}

	//生成头块
	bool MidiFile::CreateHeaderChunk()
	{
		//
		byte headerType[5] = { 'M','T','h','d' };
		midiWriter->write(headerType, 4);

		uint32_t size = 6;
		if (isLittleEndianSystem)
			size = to_big_endian(size);
		midiWriter->write(size);

		//
		short formatVal = (short)format;
		if (isLittleEndianSystem)
			formatVal = to_big_endian(formatVal);
		midiWriter->write(formatVal);

		//
		short trackCountVal = trackCount;
		if (isLittleEndianSystem)
			trackCountVal = to_big_endian(trackCountVal);
		midiWriter->write(trackCountVal);

		//
		short tickForQuarterNoteVal = tickForQuarterNote;
		if (isLittleEndianSystem)
			tickForQuarterNoteVal = to_big_endian(tickForQuarterNoteVal);
		midiWriter->write(tickForQuarterNoteVal);

		return true;
	}


	//生成轨道块
	int MidiFile::CreateTrackChuck(int trackIdx)
	{
		MidiTrack& track = *midiTracks[trackIdx];

		//
		byte headerType[4] = { 'M','T','r','k' };
		midiWriter->write(headerType, 4);

		//len
		size_t writeLenPos = midiWriter->getWriteCursor();
		midiWriter->write((int)0);

		LinkedList<MidiEvent*>* midiEvents = track.GetEventList();
		LinkedList<MidiEvent*>* newMidiEvents = nullptr;
		if (trackIdx == 0)
		{
			LinkedList<MidiEvent*>* midiGolbalEvents = track.GetGolbalEventList();
			if (midiEvents == nullptr || midiEvents->Empty()) {
				midiEvents = midiGolbalEvents;
			}
			else
			{
				newMidiEvents = new LinkedList<MidiEvent*>;

				LinkedListNode<MidiEvent*>* node = midiEvents->GetHeadNode();
				for (; node; node = node->next)
					newMidiEvents->AddLast(node->elem);

				node = midiGolbalEvents->GetHeadNode();
				for (; node; node = node->next)
					newMidiEvents->AddLast(node->elem);

				newMidiEvents->Sort(MidiEventTickCompare);
				midiEvents = newMidiEvents;
			}
		}


		LinkedListNode<MidiEvent*>* node = midiEvents->GetHeadNode();
		for (; node; node = node->next)
		{
			CreateEventData(*(node->elem));
		}

		//结尾: 00FF2F00
		WriteDynamicValue(*midiWriter, 0); //tick
		midiWriter->write((byte)0xFF);
		midiWriter->write((byte)0x2F);
		midiWriter->write((byte)0x00);

		size_t endpos = midiWriter->getWriteCursor();
		int32_t len = (int32_t)(endpos - writeLenPos - 4);
		midiWriter->setWriteCursor(writeLenPos);

		if (isLittleEndianSystem)
			len = to_big_endian(len);

		midiWriter->write((int32_t)len);

		midiWriter->setWriteCursor(endpos);

		DEL(newMidiEvents);

		return 0;
	}

	int MidiFile::CreateEventData(MidiEvent& midiEvent)
	{
		WriteDynamicValue(*midiWriter, midiEvent.startTick - curtParseTickCount);
		curtParseTickCount = midiEvent.startTick;

		switch (midiEvent.type)
		{
		case MidiEventType::NoteOn:
		{
			NoteOnEvent& noteOnEvent = (NoteOnEvent&)midiEvent;
			byte type = 9 << 4 | midiEvent.channel;
			midiWriter->write(type);
			midiWriter->write((byte)noteOnEvent.note);
			midiWriter->write((byte)noteOnEvent.velocity);
		}
		break;

		case MidiEventType::NoteOff:
		{
			NoteOffEvent& noteOffEvent = (NoteOffEvent&)midiEvent;
			byte type = 8 << 4 | midiEvent.channel;
			midiWriter->write(type);
			midiWriter->write((byte)noteOffEvent.note);
			midiWriter->write((byte)noteOffEvent.velocity);
		}
		break;

		case MidiEventType::KeyPressure:
		{
			KeyPressureEvent& keyPressureEvent = (KeyPressureEvent&)midiEvent;
			byte type = 0xA << 4 | midiEvent.channel;
			midiWriter->write(type);
			midiWriter->write((byte)keyPressureEvent.note);
			midiWriter->write((byte)keyPressureEvent.value);
		}
		break;

		case MidiEventType::Controller:
		{
			ControllerEvent& ctrlEvent = (ControllerEvent&)midiEvent;
			byte type = 0xB << 4 | midiEvent.channel;
			midiWriter->write(type);
			midiWriter->write((byte)ctrlEvent.ctrlType);
			midiWriter->write((byte)ctrlEvent.value);
		}
		break;

		case MidiEventType::ProgramChange:
		{
			ProgramChangeEvent& programEvent = (ProgramChangeEvent&)midiEvent;
			byte type = 0xC << 4 | midiEvent.channel;
			midiWriter->write(type);
			midiWriter->write((byte)programEvent.value);
		}
		break;

		case MidiEventType::ChannelPressure:
		{
			ChannelPressureEvent& channelPressureEvent = (ChannelPressureEvent&)midiEvent;
			byte type = 0xD << 4 | midiEvent.channel;
			midiWriter->write(type);
			midiWriter->write((byte)channelPressureEvent.value);
		}
		break;

		case MidiEventType::PitchBend:
		{
			PitchBendEvent& pitchBendEvent = (PitchBendEvent&)midiEvent;
			byte type = 0xE << 4 | midiEvent.channel;
			midiWriter->write(type);

			byte ff, nn;
			if (isLittleEndianSystem)
			{
				ff = pitchBendEvent.value & 0x7f;
				nn = (pitchBendEvent.value >> 7) & 0x7f;
			}
			else
			{
				ff = (pitchBendEvent.value >> 7) & 0x7f;
				nn = pitchBendEvent.value & 0x7f;
			}

			midiWriter->write(ff);
			midiWriter->write(nn);
		}
		break;

		case MidiEventType::Sysex:
		{
			SysexEvent& sysexEvent = (SysexEvent&)midiEvent;
			midiWriter->write((byte)0xF0);
			midiWriter->write(sysexEvent.data, sysexEvent.size);
		}
		break;

		case MidiEventType::Tempo:
		{
			TempoEvent& tempoEvent = (TempoEvent&)midiEvent;
			midiWriter->write((byte)0xff);  //事件类型
			midiWriter->write((byte)0x51);  //种类
			midiWriter->write((byte)0x3);  //len
			WriteInt32To3Btyes(*midiWriter, (int32_t)(tempoEvent.microTempo));
		}

		break;

		case MidiEventType::TimeSignature:
		{
			TimeSignatureEvent& timeSignatureEvent = (TimeSignatureEvent&)midiEvent;
			midiWriter->write((byte)0xff);  //事件类型
			midiWriter->write((byte)0x58);  //种类
			midiWriter->write((byte)0x4);  //len
			midiWriter->write((byte)timeSignatureEvent.numerator);
			midiWriter->write((byte)timeSignatureEvent.numerator);
			midiWriter->write((byte)timeSignatureEvent.metronomeCount);
			midiWriter->write((byte)timeSignatureEvent.nCount32ndNotesPerQuarterNote);
		}

		break;

		case MidiEventType::KeySignature:
		{
			KeySignatureEvent& keySignatureEvent = (KeySignatureEvent&)midiEvent;
			midiWriter->write((byte)0xff);  //事件类型
			midiWriter->write((byte)0x59);  //种类
			midiWriter->write((byte)0x2);  //len
			midiWriter->write((byte)keySignatureEvent.sf);
			midiWriter->write((byte)keySignatureEvent.mi);
		}

		case MidiEventType::Smpte:
		{
			SmpteEvent& smpteEvent = (SmpteEvent&)midiEvent;
			midiWriter->write((byte)0xff);  //事件类型
			midiWriter->write((byte)0x54);  //种类
			midiWriter->write((byte)0x5);  //len
			midiWriter->write((byte)smpteEvent.hr);
			midiWriter->write((byte)smpteEvent.mn);
			midiWriter->write((byte)smpteEvent.sec);
			midiWriter->write((byte)smpteEvent.fr);
			midiWriter->write((byte)smpteEvent.ff);
		}
		break;

		case MidiEventType::Unknown:
		{
			UnknownEvent& unknownEvent = (UnknownEvent&)midiEvent;
			midiWriter->write((byte)0xff);  //事件类型
			midiWriter->write((byte)0x59);  //种类
			WriteDynamicValue(*midiReader, (int32_t)unknownEvent.size); //len
			midiWriter->write(unknownEvent.data, unknownEvent.size);
		}

		break;
		}

		return 0; //继续解析当前音轨数据
	}

	//读取变长值
	uint32_t MidiFile::ReadDynamicValue(ByteStream& reader, int maxByteCount)
	{
		uint32_t val = 0;
		for (int i = 0; i < maxByteCount; i++)
		{
			uint32_t b = reader.read<byte>();
			val = (val << 7) | (b & 0x7f);
			if ((b & 0x80) == 0)
				break;
		}

		if (!isLittleEndianSystem)
		{
			byte* v = (byte*)(&val);
			swap(v[0], v[3]);
			swap(v[1], v[2]);
		}

		return val;
	}

	//写入变长值
	void MidiFile::WriteDynamicValue(ByteStream& writer, int32_t value)
	{
		if (!isLittleEndianSystem)
			value = to_little_endian(value);

		uint8_t byteValue[4] = { 0 };
		int i = 0;
		for (; i < 4; i++)
		{
			byteValue[i] = value & 0x7f;
			value >>= 7;
			if (value == 0)
				break;
		}

		for (; i > 0; i--)
			writer.write((uint8_t)(byteValue[i] | 0x80));
		writer.write(byteValue[0]);
	}


	short MidiFile::ReadInt16(ByteStream& reader) const
	{
		byte dataBtyes[2] = { 0 };
		reader.read(dataBtyes, 0, 2);
		if (isLittleEndianSystem)
			swap(dataBtyes[0], dataBtyes[1]);
		return *((short*)dataBtyes);
	}

	uint32_t MidiFile::ReadInt32(ByteStream& reader) const
	{
		byte dataBtyes[4] = { 0 };
		reader.read(dataBtyes, 0, 4);

		if (isLittleEndianSystem) {
			swap(dataBtyes[0], dataBtyes[3]);
			swap(dataBtyes[1], dataBtyes[2]);
		}

		return *((uint32_t*)dataBtyes);
	}

	uint32_t MidiFile::Read3BtyesToInt32(ByteStream& reader) const
	{
		byte dataBtyes[4] = { 0 };
		reader.read(dataBtyes, 1, 3);

		if (isLittleEndianSystem) {
			swap(dataBtyes[0], dataBtyes[3]);
			swap(dataBtyes[1], dataBtyes[2]);
		}

		return *((uint32_t*)dataBtyes);
	}


	void MidiFile::WriteInt32To3Btyes(ByteStream& writer, int32_t value) const
	{
		byte* v = (byte*)&value;

		if (isLittleEndianSystem)
		{
			writer.write(v[2]);
			writer.write(v[1]);
			writer.write(v[0]);
		}
		else
		{
			writer.write(v[1]);
			writer.write(v[2]);
			writer.write(v[3]);
		}
	}
}
