#include"MidiFile.h"
#include <iostream>
#include <fstream>
#include"MidiTrack.h"
#include"MidiEvent.h"
#include <algorithm>
using namespace std;

namespace tau
{
	MidiFile::MidiFile()
	{
		midiReader = new ByteStream();
		midiWriter = new ByteStream();

		isLittleEndianSystem = JudgeLittleOrBigEndianSystem();
	}

	MidiFile::~MidiFile()
	{
		DEL(midiReader);
		DEL(midiWriter);

		if (midiTrackList.size() <= 0)
			return;

		//
		for (int i = 0; i < midiTrackList.size(); i++)
		{
			DEL(midiTrackList[i]);
		}
		midiTrackList.clear();
	}

	// 解析文件到可识别数据结构
	void MidiFile::Parse(string filePath)
	{
		std::ifstream t;
		try
		{
			size_t length = 0;
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
		catch (exception e)
		{
			t.close();
			cout << filePath << "文件打开出错!" << endl;
			string error(e.what());
			cout << "错误原因:" << error << endl;
		}

		midiReader->clear();
	}

	//是否两个通道可以具有相同的乐器改变事件
	//只能其中一个有乐器改变事件，两者都有或都没有都不符合自动合并的要求
	bool MidiFile::IsSameProgramChannel(vector<MidiEvent*>* eventListA, vector<MidiEvent*>* eventListB)
	{
		programChangeA = -1;
		programChangeB = -1;

		vector<MidiEvent*>::iterator itA = eventListA->begin();
		MidiEvent* evA;
		bool isHavProgramChangeA = false;
		for (; itA != eventListA->end(); itA++)
		{
			evA = *itA;
			if (evA->type == MidiEventType::NoteOn)
				break;

			if (evA->type == MidiEventType::ProgramChange) {
				isHavProgramChangeA = true;
				programChangeA = ((ProgramChangeEvent*)evA)->value;
				break;
			}

		}

		vector<MidiEvent*>::iterator itB = eventListB->begin();
		MidiEvent* evB;
		bool isHavProgramChangeB = false;
		for (; itB != eventListB->end(); itB++)
		{
			evB = *itB;
			if (evB->type == MidiEventType::NoteOn)
				break;

			if (evB->type == MidiEventType::ProgramChange) {
				isHavProgramChangeB = true;
				programChangeB = ((ProgramChangeEvent*)evB)->value;
				break;
			}
		}

		//只能其中一个有乐器改变事件，两者都有或都没有都不符合自动合并的要求
		if ((isHavProgramChangeA && isHavProgramChangeB) ||
			(!isHavProgramChangeA && !isHavProgramChangeB))
			return false;

		return true;
	}

	//是否两个通道具有相同的乐器改变事件
	//即合并任何多个轨道上相同的通道事件到一个轨道，删除被合并轨道上对应通道的事件
	//如果一个轨道上所有通道事件都被合并完了，那这个轨道也会被相应删除
	void MidiFile::SetChannelSameProgram()
	{
		MidiEventList* eventListAtChannelA;
		MidiEventList* eventListAtChannelB;

		for (int i = 0; i < midiTrackList.size() - 1; i++)
		{
			eventListAtChannelA = midiTrackList[i]->GetEventListAtChannel();
			bool isEmptyChannelA = true;
			for (int n = 0; n < 16; n++)
			{
				if (!eventListAtChannelA[n].empty()) {
					isEmptyChannelA = false;
					break;
				}
			}

			if (isEmptyChannelA)
				continue;

			for (int j = i + 1; j < midiTrackList.size(); j++)
			{
				eventListAtChannelB = midiTrackList[j]->GetEventListAtChannel();

				for (int n = 0; n < 16; n++)
				{
					if (eventListAtChannelA[n].empty() ||
						eventListAtChannelB[n].empty())
						continue;

					//是否可以合并轨道通道事件的标准是:两个轨道通道事件，
					//只能其中一个有乐器改变事件，两者都有或都没有都不符合自动合并的要求
					if (mergeMode == AutoMerge &&
						!IsSameProgramChannel(&(midiTrackList[i]->GetEventListAtChannel())[n], &(midiTrackList[j]->GetEventListAtChannel())[n]))
						continue;

					if (programChangeA >= 0)
					{
						ProgramChangeEvent* pcev = new ProgramChangeEvent();
						pcev->value = programChangeA;
						eventListAtChannelB[n].insert(eventListAtChannelB[n].begin(), pcev);
					}
					else
					{
						ProgramChangeEvent* pcev = new ProgramChangeEvent();
						pcev->value = programChangeB;
						eventListAtChannelA[n].insert(eventListAtChannelA[n].begin(), pcev);
					}
				}
			}
		}
	}


	//每个通道midi事件分配到每一个轨道
	void MidiFile::PerChannelMidiEventToPerTrack()
	{
		//由于要每通道事件都分轨，当为单轨多通道时时，其实也变成了多轨
		//所以要把格式改为同步多轨格式
		if (format == MidiFileFormat::SingleTrack)
			format = MidiFileFormat::SyncTracks;

		bool isHavChannelMidiEvents = false;
		int len = midiTrackList.size();
		for (int j = 0; j < len; j++)
		{
			MidiEventList* eventListAtChannel = midiTrackList[j]->GetEventListAtChannel();
			isHavChannelMidiEvents = false;
			for (int n = 0; n < 16; n++)
			{
				if (eventListAtChannel[n].size() == 0)
					continue;

				if (!isHavChannelMidiEvents)
				{
					int count = midiTrackList[j]->GetEventCount();
					midiTrackList[j]->AppendMidiEvents(eventListAtChannel[n]);
					midiTrackList[j]->SetChannelNum(n);

					if (count != 0)
						midiTrackList[j]->GetEventList()->sort(MidiEventTickCompare);
				}
				else
				{
					bool isHavKey = false;
					for (int i = 0; i < eventListAtChannel[n].size(); i++)
					{
						if (eventListAtChannel[n][i]->type == MidiEventType::NoteOn) {
							isHavKey = true;
							break;
						}
					}

					if (isHavKey)
					{
						MidiTrack* channelTrack = new MidiTrack();
						channelTrack->AppendMidiEvents(eventListAtChannel[n]);
						channelTrack->SetChannelNum(n);
						midiTrackList.push_back(channelTrack);
					}
				}

				eventListAtChannel[n].clear();
				isHavChannelMidiEvents = true;
			}
		}
	}

	//寻找轨道默认乐器改变事件
	void MidiFile::FindTracksDefaultProgramChangeEvent()
	{
		int len = midiTrackList.size();
		for (int j = 0; j < len; j++)
		{
			midiTrackList[j]->FindDefaultProgramChangeEvent();
		}
	}


	//清空midiTrack列表,但并不真正删除track列表中的事件
	void MidiFile::ClearMidiTrackList()
	{
		for (int i = 0; i < midiTrackList.size(); i++)
		{
			midiTrackList[i]->Clear();
		}

	}


	//获取全局事件列表
	list<MidiEvent*>* MidiFile::GetGolbalEventList()
	{
		if (midiTrackList.size() <= 0)
			return nullptr;

		return midiTrackList[0]->GetGolbalEventList();
	}

	//增加一个Midi轨道
	void MidiFile::AddMidiTrack(MidiTrack* midiTrack)
	{
		midiTrackList.push_back(midiTrack);
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

		//有些midi文件的格式明明是SyncTracks，但速度设置却没有放在全局0轨道中，而把速度设置放在了其他轨道，
		//此时通过把速度设置事件迁移到0轨道来适配标准格式
		//当所以事件解析完成后，重新排序0轨道所有事件
		if (format == MidiFileFormat::SyncTracks && golbalEvents.size() != 0)
		{
			list<MidiEvent*>* globalMidiEvents = midiTrackList[0]->GetGolbalEventList();
			auto it = golbalEvents.begin();
			for (; it != golbalEvents.end(); it++)
			{
				globalMidiEvents->push_back(*it);
			}

			globalMidiEvents->sort(MidiEventTickCompare);

			//去除前后值相同,以及后值覆盖前值的全局事件
			RemoveSameAndOverrideGlobalEvents(globalMidiEvents);
		}

		//合并任何多个轨道上相同的通道事件到一个轨道，删除被合并轨道上对应通道的事件
		//如果一个轨道上所有通道事件都被合并完了，那这个轨道也会被相应删除
		if (mergeMode == AutoMerge || mergeMode == AlwaysMerge)
			SetChannelSameProgram();


		//每个通道midi事件分配到每一个轨道
		PerChannelMidiEventToPerTrack();

		FindTracksDefaultProgramChangeEvent();

		return true;
	}

	//去除前后值相同,以及后值覆盖前值的全局事件
	void MidiFile::RemoveSameAndOverrideGlobalEvents(list<MidiEvent*>* globalMidiEvents)
	{
		TempoEvent* prevTempo = nullptr;
		KeySignatureEvent* prevKeySig = nullptr;
		TimeSignatureEvent* prevTimeSig = nullptr;

		auto iter = globalMidiEvents->begin();
		for (; iter != globalMidiEvents->end(); )
		{
			switch ((*iter)->type)
			{
			case MidiEventType::Tempo:
			{
				TempoEvent* tempo = (TempoEvent*)*iter;
				if (prevTempo != nullptr)
				{
					if (prevTempo->microTempo == tempo->microTempo) {
						iter = globalMidiEvents->erase(iter);
						continue;
					}
					else if (prevTempo->startTick == tempo->startTick)
					{
						globalMidiEvents->remove(prevTempo);
					}
				}

				prevTempo = tempo;
				iter++;

			}
			break;

			case MidiEventType::KeySignature:
			{
				KeySignatureEvent* keySig = (KeySignatureEvent*)*iter;
				if (prevKeySig != nullptr)
				{
					if (prevKeySig->mi == keySig->mi &&
						prevKeySig->sf == keySig->sf) {
						iter = globalMidiEvents->erase(iter);
						continue;
					}
					else if (prevKeySig->startTick == keySig->startTick)
					{
						globalMidiEvents->remove(prevKeySig);
					}
				}

				prevKeySig = keySig;
				iter++;
			}

			break;

			case MidiEventType::TimeSignature:
			{
				TimeSignatureEvent* timeSig = (TimeSignatureEvent*)*iter;
				if (prevTimeSig != nullptr)
				{
					if (prevTimeSig->denominator == timeSig->denominator &&
						prevTimeSig->numerator == timeSig->numerator &&
						prevTimeSig->metronomeCount == timeSig->metronomeCount &&
						prevTimeSig->nCount32ndNotesPerQuarterNote == timeSig->nCount32ndNotesPerQuarterNote) {
						iter = globalMidiEvents->erase(iter);
						continue;
					}
					else if (prevTimeSig->startTick == timeSig->startTick)
					{
						globalMidiEvents->remove(prevTimeSig);
					}
				}

				prevTimeSig = timeSig;
				iter++;

			}
			break;

			default:
				iter++;
				break;

			}

		}
	}

	bool MidiFile::MidiEventTickCompare(MidiEvent* a, MidiEvent* b)
	{
		return a->startTick < b->startTick;//升序
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
		//只支持同步轨
		if (format == MidiFileFormat::AsyncTracks)
			format = MidiFileFormat::SyncTracks;


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
			return -3;

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

		midiTrackList.push_back(track);

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
				if (noteOnEvent == nullptr)
					break;

				//
				NoteOffEvent* noteOffEvent = new NoteOffEvent();
				noteOffEvent->startTick = curtParseTickCount;
				noteOffEvent->note = note;
				noteOffEvent->velocity = velocity;
				noteOffEvent->channel = lastParseEventChannel;
				noteOffEvent->noteOnEvent = noteOnEvent;

				//
				noteOnEvent->endTick = curtParseTickCount;
				noteOnEvent->noteOffEvent = noteOffEvent;

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
			if (noteOnEvent == nullptr)
				break;

			NoteOffEvent* noteOffEvent = new NoteOffEvent();
			noteOffEvent->startTick = curtParseTickCount;
			noteOffEvent->note = note;
			noteOffEvent->velocity = velocity;
			noteOffEvent->channel = lastParseEventChannel;
			noteOffEvent->noteOnEvent = noteOnEvent;

			//
			noteOnEvent->endTick = curtParseTickCount;
			noteOnEvent->noteOffEvent = noteOffEvent;

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
				if (b != 0xF7)
					byteCodes.push_back(b);
				else
					break;
			} while (true);

			sysexEvent->CreateData(byteCodes.data(), byteCodes.size());
			track.AddEvent(sysexEvent);
		}
		break;

		case 0xFF:
		{
			byte type = midiReader->read<byte>();
			switch (type)
			{
			case 0x03:
			case 0x04:
			{
				TextEvent* textEvent = new TextEvent();

				switch (type)
				{
				case 0x03: textEvent->textType = MidiTextType::TrackName; break;
				case 0x04: textEvent->textType = MidiTextType::InstrumentName; break;
				}

				uint32_t len = ReadDynamicValue(*midiReader);
				if (len != 0)
				{
					byte* byteCodes = (byte*)malloc(len);
					midiReader->read(byteCodes, 0, len);
					byteCodes[len - 1] = 0;
					if (byteCodes != nullptr) {
						textEvent->text.assign((const char*)byteCodes);
						free(byteCodes);
					}
				}

				track.AddEvent(textEvent);
			}
			break;

			case 0x51:
			{
				midiReader->read<byte>();
				TempoEvent* tempoEvent = new TempoEvent();
				tempoEvent->startTick = curtParseTickCount;
				tempoEvent->microTempo = (float)Read3BtyesToInt32(*midiReader);

				//有些midi文件的格式明明是SyncTracks，但速度设置却没有放在全局0轨道中，而把速度设置放在了其他轨道，
				//此时通过把速度设置事件迁移到0轨道来适配标准格式
				if (midiTrackList.size() != 0 &&
					midiTrackList[0] != nullptr &&
					format == MidiFileFormat::SyncTracks)
				{
					golbalEvents.push_back(tempoEvent);
				}
				else
				{
					track.AddEvent(tempoEvent);
				}
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

				//有些midi文件的格式明明是SyncTracks，但全局设置却没有放在全局0轨道中，而把全局设置放在了其他轨道，
				//此时通过把全局设置事件迁移到0轨道来适配标准格式
				if (midiTrackList.size() != 0 &&
					midiTrackList[0] != nullptr &&
					format == MidiFileFormat::SyncTracks)
				{
					golbalEvents.push_back(timeSignatureEvent);
				}
				else
				{
					track.AddEvent(timeSignatureEvent);
				}
			}
			break;

			case 0x59:
			{
				midiReader->read<byte>();
				KeySignatureEvent* keySignatureEvent = new KeySignatureEvent();
				keySignatureEvent->startTick = curtParseTickCount;
				keySignatureEvent->sf = midiReader->read<byte>();
				keySignatureEvent->mi = midiReader->read<byte>();

				//有些midi文件的格式明明是SyncTracks，但全局设置却没有放在全局0轨道中，而把全局设置放在了其他轨道，
				//此时通过把全局设置事件迁移到0轨道来适配标准格式
				if (midiTrackList.size() != 0 &&
					midiTrackList[0] != nullptr &&
					format == MidiFileFormat::SyncTracks)
				{
					golbalEvents.push_back(keySignatureEvent);
				}
				else
				{
					track.AddEvent(keySignatureEvent);
				}
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
}
