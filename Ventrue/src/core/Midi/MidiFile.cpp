#include"MidiFile.h"
#include <iostream>
#include <fstream>
#include"MidiTrack.h"
#include"MidiEvent.h"

namespace ventrue
{
    MidiFile::MidiFile()
    {
        midiReader = new ByteStream();
        midiWriter = new ByteStream();

        isLittleEndianSystem = JudgeLittleOrBigEndianSystem();
        midiTrackList = new MidiTrackList;
    }

    MidiFile::~MidiFile()
    {
        DEL(midiReader);
        DEL(midiWriter);
        DEL_OBJS_VECTOR(midiTrackList);
    }

    // 解析文件到可识别数据结构
    void MidiFile::Parse(string filePath)
    {
        std::ifstream t;
        try
        {
            size_t length;
            t.open(filePath, ios::in | ios::binary);
            t.seekg(0, std::ios::end);
            length = t.tellg();
            t.seekg(0, std::ios::beg);
            char* buffer = new char[length];
            t.read(buffer, length);
            t.close();
            midiReader->write(buffer, length);
            delete[] buffer;
            ParseCore();
        }
        catch (exception)
        {
            t.close();
            cout<< filePath << "文件打开出错!"<<endl;
        }

        midiReader->clear();
    }

    //增加一个Midi轨道
    void MidiFile::AddMidiTrack(MidiTrack* midiTrack)
    {
        if (midiTrackList == nullptr)
            midiTrackList = new MidiTrackList();

        midiTrackList->push_back(midiTrack);
        trackCount++;
    }

    //解析内核
    bool MidiFile::ParseCore()
    {
        bool isSuccess = ParseHeaderChunk();
        if (!isSuccess)
            return false;

        for (int i = 0; i < trackCount; i++)
        {
            lastParseEventNum = 0;
            lastParseEventChannel = 0;
            curtParseTickCount = 0;

            int ret = ParseTrackChuck();
            if (ret == -1)
                return false;
        }

        return true;
    }

    // 解析头块
    bool MidiFile::ParseHeaderChunk()
    {
        //
        byte headerType[5] = { 0 };
        midiReader->read(headerType, 0, 4);
        if (strcmp((const char*)headerType, "MThd") != 0)
            return false;

        //
        midiReader->read<int32_t>();

        //
        short formatVal = ReadInt16(*midiReader);
        if (formatVal > 2 || formatVal < 0)
            return false;
        format = (MidiFileFormat)formatVal;

        //
        trackCount = ReadInt16(*midiReader);
        if (trackCount <= 0)
            return false;

        //
        tickForQuarterNote = ReadInt16(*midiReader);
        return tickForQuarterNote > 0;

    }


    //解析轨道块
    int MidiFile::ParseTrackChuck()
    {
        //
        byte headerType[5] = { 0 };
        midiReader->read(headerType, 0, 4);
        if (strcmp((const char*)headerType, "MTrk") != 0)
            return -1;

        //
        uint32_t trackLen = ReadInt32(*midiReader);
        if (trackLen == 0)
            return 0;

        MidiTrack* track = new MidiTrack();
        track->SetTickForQuarterNote(tickForQuarterNote);
        int parseRet = 0;

        while (midiReader->getReadCursor() < midiReader->size())
        {
            uint32_t deltaTime = ReadDynamicValue(*midiReader);
            curtParseTickCount += deltaTime;

            //解析事件
            byte evnum = midiReader->peek<byte>();
            if (!(evnum >= 0x00 && evnum <= 0x7f))
            {
                evnum = midiReader->read<byte>();

                if (evnum == 0xFF)
                {
                    lastParseEventNum = 0xFF;
                }
                else if (evnum == 0xF0)
                {
                    lastParseEventNum = 0xF0;
                }
                else
                {
                    lastParseEventNum = (byte)(evnum >> 4);
                    lastParseEventChannel = (byte)(evnum & 0xf);
                }
            }

            parseRet = ParseEvent(*track);
            if (parseRet == -1)   //解析出错
                break;
            if (parseRet == 2)  //当前音轨数据解析结束
            {
                parseRet = 0;
                break;
            }
        }

        if (track->GetEventCount() != 0)
            midiTrackList->push_back(track);

        return parseRet;
    }

    int MidiFile::ParseEvent(MidiTrack& track)
    {
        switch (lastParseEventNum)
        {
        case 0x9:
        {
            int note = midiReader->read<byte>();
            int velocity = midiReader->read<byte>();

            NoteOnEvent* noteOnEvent;
            if (velocity == 0)
            {
                noteOnEvent = track.FindNoteOnEvent(note, lastParseEventChannel);
               
                //
                NoteOffEvent* noteOffEvent = new NoteOffEvent();
                noteOffEvent->startTick = curtParseTickCount;
                noteOffEvent->note = note;
                noteOffEvent->velocity = velocity;
                noteOffEvent->channel = lastParseEventChannel;
                noteOffEvent->noteOnEvent = noteOnEvent;


                if (noteOnEvent != nullptr) {
                    noteOnEvent->endTick = curtParseTickCount;
                    noteOnEvent->noteOffEvent = noteOffEvent;
                }

                track.AddEvent(noteOffEvent);
            }
            else
            {
                noteOnEvent = new NoteOnEvent();
                noteOnEvent->startTick = curtParseTickCount;
                noteOnEvent->note = note;
                noteOnEvent->velocity = velocity;
                noteOnEvent->channel = lastParseEventChannel;
                track.AddEvent(noteOnEvent);
            }
        }
        break;

        case 0x8:
        {
            int note = midiReader->read<byte>();
            int velocity = midiReader->read<byte>();
            NoteOnEvent* noteOnEvent = track.FindNoteOnEvent(note, lastParseEventChannel);
            NoteOffEvent* noteOffEvent = new NoteOffEvent();
            noteOffEvent->startTick = curtParseTickCount;
            noteOffEvent->note = note;
            noteOffEvent->velocity = velocity;
            noteOffEvent->channel = lastParseEventChannel;
            noteOffEvent->noteOnEvent = noteOnEvent;

            if (noteOnEvent != nullptr) {
                noteOnEvent->endTick = curtParseTickCount;
                noteOnEvent->noteOffEvent = noteOffEvent;
            }
  
            track.AddEvent(noteOffEvent);
        }
        break;

        case 0xA:
        {
            KeyPressureEvent* keyPressureEvent = new KeyPressureEvent();
            keyPressureEvent->startTick = curtParseTickCount;
            keyPressureEvent->note = midiReader->read<byte>();
            keyPressureEvent->value = midiReader->read<byte>();
            keyPressureEvent->channel = lastParseEventChannel;
            track.AddEvent(keyPressureEvent);
        }
        break;

        case 0xB:
        {
            ControllerEvent* ctrlEvent = new ControllerEvent();
            ctrlEvent->startTick = curtParseTickCount;
            ctrlEvent->ctrlType = (MidiControllerType)midiReader->read<byte>();
            ctrlEvent->value = midiReader->read<byte>();
            ctrlEvent->channel = lastParseEventChannel;
            track.AddEvent(ctrlEvent);
        }
        break;

        case 0xC:
        {
            ProgramChangeEvent* programEvent = new ProgramChangeEvent();
            programEvent->startTick = curtParseTickCount;
            programEvent->channel = lastParseEventChannel;
            programEvent->value = midiReader->read<byte>();
            track.AddEvent(programEvent);
        }
        break;

        case 0xD:
        {
            ChannelPressureEvent* channelPressureEvent = new ChannelPressureEvent();
            channelPressureEvent->startTick = curtParseTickCount;
            channelPressureEvent->value = midiReader->read<byte>();
            channelPressureEvent->channel = lastParseEventChannel;
            track.AddEvent(channelPressureEvent);
        }
        break;

        case 0xE:
        {
            PitchBendEvent* pitchBendEvent = new PitchBendEvent();
            pitchBendEvent->startTick = curtParseTickCount;

            int ff = midiReader->read<byte>() & 0x7F;
            int nn = midiReader->read<byte>() & 0x7F;

            if (isLittleEndianSystem)
                pitchBendEvent->value = nn << 7 | ff;
            else
                pitchBendEvent->value = ff << 7 | nn;

            pitchBendEvent->channel = lastParseEventChannel;
            track.AddEvent(pitchBendEvent);
        }
        break;

        case 0xF0:
        {
            SysexEvent* sysexEvent = new SysexEvent();
            sysexEvent->startTick = curtParseTickCount;
            byte b;
            vector<byte> byteCodes;

            do
            {
                b = midiReader->read<byte>();
                byteCodes.push_back(b);
            } while (b != 0xF7);

            sysexEvent->CreateData(byteCodes.data(), byteCodes.size());
            track.AddEvent(sysexEvent);
        }
        break;

        case 0xFF:
        {
            byte type = midiReader->read<byte>();
            switch (type)
            {
            case 0x51:
            {
                midiReader->read<byte>();
                TempoEvent* tempoEvent = new TempoEvent();
                tempoEvent->startTick = curtParseTickCount;
                tempoEvent->microTempo = (float)Read3BtyesToInt32(*midiReader);
                track.AddEvent(tempoEvent);
            }
            break;

            case 0x58:
            {
                midiReader->read<byte>();
                TimeSignatureEvent* timeSignatureEvent = new TimeSignatureEvent();
                timeSignatureEvent->startTick = curtParseTickCount;
                timeSignatureEvent->numerator = midiReader->read<byte>();
                timeSignatureEvent->denominator = midiReader->read<byte>();
                timeSignatureEvent->metronomeCount = midiReader->read<byte>();
                timeSignatureEvent->nCount32ndNotesPerQuarterNote = midiReader->read<byte>();
                track.AddEvent(timeSignatureEvent);
            }
            break;

            case 0x59:
            {
                midiReader->read<byte>();
                KeySignatureEvent* keySignatureEvent = new KeySignatureEvent();
                keySignatureEvent->startTick = curtParseTickCount;
                keySignatureEvent->sf = midiReader->read<byte>();
                keySignatureEvent->mi = midiReader->read<byte>();
                track.AddEvent(keySignatureEvent);
            }
            break;


            case 0x2F:
                midiReader->read<byte>();  //FF2F00
                return 2;  //当前音轨数据解析结束

            default:
            {
                UnknownEvent* unknownEvent = new UnknownEvent();
                unknownEvent->codeType = type;
                uint32_t len = ReadDynamicValue(*midiReader);

                byte* byteCodes = (byte*)malloc(len);
                midiReader->read(byteCodes, 0, len);

                unknownEvent->CreateData(byteCodes, len);
                free(byteCodes);

                track.AddEvent(unknownEvent);
            }
            break;
            }
        }
        break;

        }

        return 0; //继续解析当前音轨数据
    }

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

        for (int i = 0; i < midiTrackList->size(); i++)
        {
            curtParseTickCount = 0;
            CreateTrackChuck(i);
        }
    }

    //生成头块
    bool MidiFile::CreateHeaderChunk()
    {
        //
        byte headerType[5] = { 'M','T','h','d'};
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
        MidiTrack& track = *(*midiTrackList)[trackIdx];

        //
        byte headerType[4] = { 'M','T','r','k'};
        midiWriter->write(headerType, 4);

        //len
        size_t writeLenPos = midiWriter->getWriteCursor();
        midiWriter->write((int)0); 
        MidiEventList* midiEvents = track.GetEventList();

        for (int i = 0; i < midiEvents->size(); i++)
        {
            CreateEventData(*(*midiEvents)[i]);
        }

        //结尾: 00FF2F00
        WriteDynamicValue(*midiWriter, 0); //tick
        midiWriter->write((byte)0xFF);
        midiWriter->write((byte)0x2F);
        midiWriter->write((byte)0x00);

        size_t endpos = midiWriter->getWriteCursor();
        int32_t len = (int32_t)(endpos - writeLenPos - 4);
        midiWriter->setWriteCursor(writeLenPos);

        if(isLittleEndianSystem)
            len = to_big_endian(len);

        midiWriter->write((int32_t)len);

        midiWriter->setWriteCursor(endpos);

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


    short MidiFile::ReadInt16(ByteStream& reader)
    {
        byte dataBtyes[2] = { 0 };
        reader.read(dataBtyes, 0, 2);
        if (isLittleEndianSystem)
            swap(dataBtyes[0], dataBtyes[1]);
        return *((short*)dataBtyes);
    }

    uint32_t MidiFile::ReadInt32(ByteStream& reader)
    {
        byte dataBtyes[4] = { 0 };
        reader.read(dataBtyes, 0, 4);

        if (isLittleEndianSystem) {
            swap(dataBtyes[0], dataBtyes[3]);
            swap(dataBtyes[1], dataBtyes[2]);
        }

        return *((uint32_t*)dataBtyes);
    }

    uint32_t MidiFile::Read3BtyesToInt32(ByteStream& reader)
    {
        byte dataBtyes[4] = { 0 };
        reader.read(dataBtyes, 1, 3);

        if (isLittleEndianSystem) {
            swap(dataBtyes[0], dataBtyes[3]);
            swap(dataBtyes[1], dataBtyes[2]);
        }

        return *((uint32_t*)dataBtyes);
    }


    void MidiFile::WriteInt32To3Btyes(ByteStream& writer, int32_t value)
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