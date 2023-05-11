#include"MidiFile.h"
#include <iostream>
#include <fstream>
#include"MidiTrack.h"
#include"MidiEvent.h"
#include <algorithm>
#include <chrono>
#include <codecvt>
#include<scutils/CodeConvert.h>

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

		if (midiTracks.empty())
			return;

		//
		for (int i = 0; i < midiTracks.size(); i++)
		{
			DEL(midiTracks[i]);
		}
		midiTracks.clear();
	}

	// 解析文件到可识别数据结构
	bool MidiFile::Parse(string filePath)
	{
		bool ret = true;
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
			ret = ParseCore();
		}
		catch (exception e)
		{
			t.close();
			cout << filePath << "文件打开出错!" << endl;
			string error(e.what());
			cout << "错误原因:" << error << endl;
		}

		midiReader->clear();
		return ret;
	}


	//每个通道midi事件分配到每一个轨道
	//每轨通道中的midiEvent将被合并到轨道events中
	void MidiFile::PerChannelMidiEventToPerTrack()
	{
		//由于要每通道事件都分轨，当为单轨多通道时时，其实也变成了多轨
		//所以要把格式改为同步多轨格式
		if (format == MidiFileFormat::SingleTrack)
			format = MidiFileFormat::SyncTracks;

		bool isHavChannelMidiEvents = false; //是否在当前轨道上已经存在通道事件组了
		int len = midiTracks.size();
		for (int j = 0; j < len; j++)
		{
			LinkedList<MidiEvent*>* eventListAtChannel = midiTracks[j]->GetEventListAtChannel();
			isHavChannelMidiEvents = false;
			for (int n = 0; n < 16; n++)
			{
				if (eventListAtChannel[n].Size() == 0)
					continue;

				//如果在当前轨道上不存在通道事件组，将为当前轨道保存通道事件
				if (!isHavChannelMidiEvents)
				{
					int count = midiTracks[j]->GetEventCount();
					midiTracks[j]->AppendMidiEvents(eventListAtChannel[n]);
					midiTracks[j]->SetChannelNum(n);

					if (count != 0)
						midiTracks[j]->GetEventList()->Sort(MidiEventTickCompare);
				}
				else //如果在当前轨道上已存在通道事件组，将增加一个轨道到尾部，并把当前通道事件保存到这个尾部轨道中
				{
					bool isHavKey = false;
					LinkedListNode<MidiEvent*>* node = eventListAtChannel[n].GetHeadNode();
					for (; node; node = node->next)
					{
						if (node->elem->type == MidiEventType::NoteOn) {
							isHavKey = true;
							break;
						}
					}

					if (isHavKey)
					{
						MidiTrack* channelTrack = new MidiTrack();
						channelTrack->AppendMidiEvents(eventListAtChannel[n]);
						channelTrack->SetChannelNum(n);
						midiTracks.push_back(channelTrack);

						//复制轨道全局事件到新的轨道
						//0号轨道为全局轨道，其中的全局事件不需要复制，它对所有轨道有效
						if (j != 0)
							CopyTrackGlobalEventsForNewTrack(j, midiTracks.size() - 1);
					}
				}


				eventListAtChannel[n].Release();
				isHavChannelMidiEvents = true;
			}
		}
	}

	//复制轨道全局事件到新的轨道
	void MidiFile::CopyTrackGlobalEventsForNewTrack(int oldTrackIdx, int newTrackIdx)
	{
		MidiEvent* cpyMidiEvent;
		LinkedList<MidiEvent*>* globalMidiEvents = midiTracks[0]->GetGolbalEventList();
		LinkedListNode<MidiEvent*>* next;
		for (auto node = globalMidiEvents->GetHeadNode(); node; node = next)
		{
			if (node->elem->track != oldTrackIdx) {
				next = node->next;
				continue;
			}

			MidiEvent* midiEvent = node->elem;
			switch (midiEvent->type)
			{
			case MidiEventType::KeySignature:
				next = node->next;
				cpyMidiEvent = new KeySignatureEvent(*(KeySignatureEvent*)midiEvent);
				globalMidiEvents->AddBack(node, cpyMidiEvent);
				break;
			default:
				next = node->next;
				break;
			}
		}
	}

	//拷贝相同通道控制事件
	void MidiFile::CopySameChannelControlEvents()
	{
		int c1, c2;
		int len = midiTracks.size();
		LinkedList<MidiEvent*>* copyMidiEvents = new LinkedList<MidiEvent*>[len];

		for (int i = 0; i < len; i++)
		{
			c1 = midiTracks[i]->GetChannelNum();

			for (int j = 0; j < len; j++)
			{
				if (j == i)
					continue;

				c2 = midiTracks[j]->GetChannelNum();
				if (c1 != c2)
					continue;

				midiTracks[i]->CopyControlEvents(copyMidiEvents[j]);
			}
		}

		for (int i = 0; i < len; i++)
		{
			if (copyMidiEvents[i].Empty())
				continue;

			midiTracks[i]->AppendMidiEvents(copyMidiEvents[i]);
			midiTracks[i]->GetEventList()->Sort(MidiEventTickCompare);
		}
	}


	//寻找轨道默认乐器改变事件
	void MidiFile::FindTracksDefaultProgramChangeEvent()
	{
		int len = midiTracks.size();
		for (int j = 0; j < len; j++)
		{
			midiTracks[j]->FindDefaultProgramChangeEvent();
		}
	}


	//清空midiTrack列表,但并不真正删除track列表中的事件
	void MidiFile::ClearMidiTrackList()
	{
		for (int i = 0; i < midiTracks.size(); i++)
		{
			midiTracks[i]->Clear();
		}

	}


	//获取全局事件列表
	LinkedList<MidiEvent*>* MidiFile::GetGolbalEventList()
	{
		if (midiTracks.empty())
			return nullptr;

		return midiTracks[0]->GetGolbalEventList();
	}

	//增加一个Midi轨道
	void MidiFile::AddMidiTrack(MidiTrack* midiTrack)
	{
		midiTracks.push_back(midiTrack);
		trackCount++;
	}


	//解析内核
	bool MidiFile::ParseCore()
	{
		bool isSuccess = ParseHeaderChunk();
		if (!isSuccess)
			return false;

		//
		if (isEnableParseLimitTime)
		{
			perTrackParseSec = limitParseSec / trackCount;
			curtTrackParseSec = perTrackParseSec;
		}

		for (int i = 0; i < trackCount; i++)
		{
			lastParseEventNum = 0;
			lastParseEventChannel = 0;
			curtParseTickCount = 0;

			int ret = ParseTrackChuck(i);
			if (ret == -1 || isStopParse)
				return false;

		}

		//有些midi文件的格式明明是SyncTracks，但速度设置却没有放在全局0轨道中，而把速度设置放在了其他轨道，
		//此时通过把速度设置事件迁移到0轨道来适配标准格式
		//当所有事件解析完成后，重新排序0轨道所有事件
		if (format == MidiFileFormat::SyncTracks && !golbalEvents.empty())
		{
			LinkedList<MidiEvent*>* globalMidiEvents = midiTracks[0]->GetGolbalEventList();
			auto it = golbalEvents.begin();
			for (; it != golbalEvents.end(); it++)
			{
				globalMidiEvents->AddLast(*it);
			}

			//
			LinkedList<MidiEvent*>* midiEvents;
			for (int i = 1; i < midiTracks.size(); i++)
			{
				midiEvents = midiTracks[i]->GetGolbalEventList();
				for (auto node = midiEvents->GetHeadNode(); node; node = node->next)
				{
					if (node->elem->type == MidiEventType::Text)
					{
						TextEvent* textEv = (TextEvent*)node->elem;

						if (textEv->textType == MidiTextType::Marker ||
							textEv->textType == MidiTextType::ProgramName ||
							textEv->textType == MidiTextType::Title ||
							textEv->textType == MidiTextType::Copyright ||
							textEv->textType == MidiTextType::Comment)
						{
							globalMidiEvents->AddLast(node->elem);
						}
					}
					else
					{
						globalMidiEvents->AddLast(node->elem);
					}
				}
			}

			globalMidiEvents->Sort(MidiEventTickCompare);

			//去除前后值相同,以及后值覆盖前值的全局事件
			RemoveSameAndOverrideGlobalEvents(globalMidiEvents);
		}

		//增加默认全局事件
		AddDefaultGlobalEvents(midiTracks[0]->GetGolbalEventList());


		//每个通道midi事件分配到每一个轨道
		PerChannelMidiEventToPerTrack();

		//拷贝相同通道控制事件
		if (enableCopySameChannelControlEvents)
			CopySameChannelControlEvents();

		FindTracksDefaultProgramChangeEvent();

		//重新设置所有midi事件的轨道编号
		for (int i = 0; i < midiTracks.size(); i++)
			midiTracks[i]->SetAllMidiEventTrackIdx(i);

		//精简合并音符事件
		if (enableMergeNotesOptimize)
			MergeNoteEvents();

		return true;
	}

	//增加默认全局TimeSignatureEvent， KeySignatureEvent, TempoEvent事件
	void MidiFile::AddDefaultGlobalEvents(LinkedList<MidiEvent*>* globalMidiEvents)
	{
		LinkedListNode<MidiEvent*>* node = globalMidiEvents->GetHeadNode();
		bool isHaveTimeSignature = false;
		bool isHaveKeySignature = false;
		bool isHaveTempo = false;

		for (; node; node = node->next)
		{
			if (node->elem->startTick != 0)
				break;

			if (node->elem->type == MidiEventType::TimeSignature)
				isHaveTimeSignature = true;
			if (node->elem->type == MidiEventType::Tempo)
				isHaveTempo = true;
			if (node->elem->type == MidiEventType::KeySignature && node->elem->track == 0)
				isHaveKeySignature = true;

			if (isHaveTempo && isHaveTimeSignature && isHaveKeySignature)
				break;

		}

		if (!isHaveTempo) {
			TempoEvent* tempoEvent = new TempoEvent();
			tempoEvent->startTick = 0;
			tempoEvent->microTempo = 60000000 / 120;
			globalMidiEvents->AddFirst(tempoEvent);
		}

		if (!isHaveTimeSignature) {
			TimeSignatureEvent* timeSignatureEvent = new TimeSignatureEvent();
			timeSignatureEvent->startTick = 0;
			timeSignatureEvent->numerator = 4;
			timeSignatureEvent->denominator = 2;
			globalMidiEvents->AddFirst(timeSignatureEvent);
		}

		if (!isHaveKeySignature) {
			KeySignatureEvent* keySignatureEvent = new KeySignatureEvent();
			keySignatureEvent->startTick = 0;
			keySignatureEvent->sf = 0;
			keySignatureEvent->mi = 0;
			globalMidiEvents->AddFirst(keySignatureEvent);
		}


	}

	//去除前后值相同,以及后值覆盖前值的全局事件
	void MidiFile::RemoveSameAndOverrideGlobalEvents(LinkedList<MidiEvent*>* globalMidiEvents)
	{
		unordered_map<int, vector<LinkedListNode<MidiEvent*>*>> keySigNodeMap;

		LinkedListNode<MidiEvent*>* prevTempoNode = nullptr;
		LinkedListNode<MidiEvent*>* prevTimeSigNode = nullptr;

		LinkedListNode<MidiEvent*>* node = globalMidiEvents->GetHeadNode();
		LinkedListNode<MidiEvent*>* next;
		for (; node;)
		{
			switch (node->elem->type)
			{
			case MidiEventType::Tempo:
			{
				TempoEvent* tempo = (TempoEvent*)node->elem;
				if (prevTempoNode != nullptr)
				{
					TempoEvent* prevTempo = (TempoEvent*)prevTempoNode->elem;
					if (prevTempo->microTempo == tempo->microTempo) {

						next = globalMidiEvents->Remove(node);
						DEL(node->elem);
						DEL(node);
						node = next;
						continue;
					}
					else if (prevTempo->startTick == tempo->startTick)
					{
						globalMidiEvents->Remove(prevTempoNode);
						DEL(prevTempoNode->elem);
						DEL(prevTempoNode);
					}
				}

				prevTempoNode = node;
				node = node->next;

			}
			break;

			case MidiEventType::KeySignature:
			{
				KeySignatureEvent* keySig = (KeySignatureEvent*)node->elem;
				keySigNodeMap[keySig->track].push_back(node);
				node = node->next;
			}

			break;

			case MidiEventType::TimeSignature:
			{
				TimeSignatureEvent* timeSig = (TimeSignatureEvent*)node->elem;
				if (prevTimeSigNode != nullptr)
				{
					TimeSignatureEvent* prevTimeSig = (TimeSignatureEvent*)prevTimeSigNode->elem;

					if (prevTimeSig->denominator == timeSig->denominator &&
						prevTimeSig->numerator == timeSig->numerator &&
						prevTimeSig->metronomeCount == timeSig->metronomeCount &&
						prevTimeSig->nCount32ndNotesPerQuarterNote == timeSig->nCount32ndNotesPerQuarterNote) {
						next = globalMidiEvents->Remove(node);
						DEL(node->elem);
						DEL(node);
						node = next;
						continue;
					}
					else if (prevTimeSig->startTick == timeSig->startTick)
					{
						globalMidiEvents->Remove(prevTimeSigNode);
						DEL(prevTimeSigNode->elem);
						DEL(prevTimeSigNode);
					}
				}

				prevTimeSigNode = node;
				node = node->next;

			}
			break;

			default:
				node = node->next;
				break;
			}
		}

		//
		LinkedListNode<MidiEvent*>* prevKeySigNode = nullptr;
		for (auto& v : keySigNodeMap)
		{
			prevKeySigNode = nullptr;
			auto nodes = v.second;
			for (int i = 0; i < nodes.size(); i++)
			{
				node = nodes[i];
				KeySignatureEvent* keySig = (KeySignatureEvent*)node->elem;
				if (prevKeySigNode != nullptr)
				{
					KeySignatureEvent* prevKeySig = (KeySignatureEvent*)prevKeySigNode->elem;

					if (prevKeySig->mi == keySig->mi &&
						prevKeySig->sf == keySig->sf) {
						globalMidiEvents->Remove(node);
						DEL(node->elem);
						DEL(node);
						continue;
					}
					else if (prevKeySig->startTick == keySig->startTick)
					{
						globalMidiEvents->Remove(prevKeySigNode);
						DEL(prevKeySigNode->elem);
						DEL(prevKeySigNode);
					}
				}

				prevKeySigNode = node;
			}
		}
	}


	//简化合并音符事件
	void MidiFile::MergeNoteEvents()
	{
		LinkedList<MidiEvent*> eventsAtNotes[128];
		int perGroupTick = 200; //每组tick数 (默认:100)
		int perGroupNoteCount = 50; //每组note数量 (默认:128)
		int mergeNoteMaxSelfTick = 5; //能被合并的note的最大tick (默认:5)
		int mergeNoteMaxDistTick = 5; //能被合并的note的最大距离tick  (默认:5)

		//
		NoteOnEvent* noteOnEv;
		int noteCount = 0;
		bool isSetStartTick = true;
		bool isSetEndTick = true;
		uint32_t startTick = 0, endTick = perGroupTick;
		for (int i = 0; i < midiTracks.size(); i++)
		{
			LinkedList<MidiEvent*>* midiEventList = midiTracks[i]->GetEventList();
			LinkedListNode<MidiEvent*>* orgNode = midiEventList->GetHeadNode();
			LinkedListNode<MidiEvent*>* nextNode = nullptr, * orgNodeNext = nullptr;
			LinkedListNode<MidiEvent*>* startNode = nullptr, * prevNode;

			for (; orgNode; orgNode = orgNodeNext)
			{
				MidiEvent* midiEvent = orgNode->elem;


				if (orgNode->next || noteCount == 0)
				{
					if (midiEvent->type != MidiEventType::NoteOn) {
						orgNodeNext = orgNode->next;
						continue;
					}


					noteOnEv = (NoteOnEvent*)midiEvent;

					if (isSetEndTick) {
						endTick = noteOnEv->startTick + perGroupTick;
						isSetEndTick = false;
					}

					//获取note分段，按最小在perGroupTick为一组范围内note被插入到eventsAtNotes分类中，
					//同时算法需要保证note的on,off,必须在一个tick组分段中,而不能被拆开到下一个tick组分段中
					//当note的开始时间点小于指定结束时间点时，把note插入到eventsAtNotes分类中
					if (noteOnEv->startTick < endTick) {
						noteCount++;

						//如果要插入的note结束时间点大于指定结束时间点时，
						//将调整指定结束时间点到最新位置，目的是为了包含note的 on, off事件在一组eventsAtNotes分类中
						if (noteOnEv->endTick > endTick)
							endTick = noteOnEv->endTick;

						if (isSetStartTick) {
							startNode = orgNode;
							startTick = noteOnEv->startTick;
							isSetStartTick = false;
						}

						orgNodeNext = orgNode->next;
						continue;
					}
				}

				if (orgNode->next == nullptr)
					orgNode = nullptr;


				//当note的开始时间点大于指定结束时间点时，
				//将对已插入eventsAtNotes分类进行简化合并算法
				float tickMul = (endTick - startTick) / (float)perGroupTick;
				noteCount /= tickMul; //每perGroupTick有多少个note
				//当每perGroupTick没有有超过perGroupNoteCount个note在其中时，不启用note合并算法
				if (noteCount <= perGroupNoteCount) {
					orgNodeNext = orgNode;
					startNode = nullptr;
					prevNode = nullptr;
					noteCount = 0;
					isSetEndTick = true;
					isSetStartTick = true;
					continue;
				}

				prevNode = startNode->prev;
				//当每perGroupTick有超过perGroupNoteCount个note在其中时，将启用简化合并note算法
				//把符合要求的note插入到eventsAtNotes分类中
				LinkedListNode<MidiEvent*>* nd;
				for (nd = startNode; nd; nd = nextNode)
				{
					MidiEvent* midiEvent = nd->elem;
					if (midiEvent->type == MidiEventType::NoteOn)
					{
						NoteOnEvent* noteOnEv = (NoteOnEvent*)midiEvent;
						//当note的开始时间点小于指定结束时间点时，把note插入到eventsAtNotes分类中
						if (noteOnEv->startTick < endTick) {
							nextNode = midiEventList->Remove(nd);
							DEL(nd);
							eventsAtNotes[noteOnEv->note].AddLast(noteOnEv);
						}
						else {
							break;
						}
					}
					else if (midiEvent->type == MidiEventType::NoteOff)
					{
						nextNode = midiEventList->Remove(nd);
						DEL(nd);
						NoteOffEvent* noteOffEv = (NoteOffEvent*)midiEvent;
						eventsAtNotes[noteOffEv->note].AddLast(midiEvent);
					}
					else {
						nextNode = nd->next;
					}
				}


				//启用简化合并note算法
				int firstIdx = -1;
				LinkedList<MidiEvent*>* newNotes = nullptr;
				NoteOnEvent* noteOnEv2;
				for (int j = 0; j < 128; j++)
				{
					LinkedList<MidiEvent*>& notes = eventsAtNotes[j];
					LinkedListNode<MidiEvent*>* node = notes.GetHeadNode();
					LinkedListNode<MidiEvent*>* next = nullptr, * prevNext = nullptr;

					if (node != nullptr && firstIdx == -1)
					{
						newNotes = &notes;
						firstIdx = j;
					}

					//
					for (; node; node = next)
					{
						MidiEvent* ev = node->elem;
						MidiEvent* ev2 = node->next->elem;

						//1. ev和ev2形成note1, 并且 note2 跟随 note1 
						if (ev->type == MidiEventType::NoteOn &&
							ev2->type == MidiEventType::NoteOff)
						{
							//如果存在note2，连接note2到note1中
							if (node->next->next != nullptr)
							{
								noteOnEv = (NoteOnEvent*)ev;
								noteOnEv2 = (NoteOnEvent*)(node->next->next->elem);

								//当note1和note2距离在规定范围mergeNoteMaxDistTick内时 
								//且note2的tick范围小于mergeNoteMaxSelfTick，连接note2到note1中
								if (noteOnEv2->startTick - noteOnEv->endTick <= mergeNoteMaxDistTick &&
									noteOnEv2->endTick - noteOnEv2->startTick <= mergeNoteMaxSelfTick)
								{
									prevNext = node->next;
									next = notes.Remove(prevNext);
									DEL(prevNext);
									notes.Remove(next);
									DEL(next);
									DEL(noteOnEv->noteOffEvent);
									noteOnEv->noteOffEvent = noteOnEv2->noteOffEvent;
									DEL(noteOnEv2);
									noteOnEv->endTick = noteOnEv->noteOffEvent->startTick;

									next = node;
								}
								else {
									next = node->next->next;
								}
							}
							else {
								next = node->next->next;
							}
						}
						//2. note2和note1交错 或者note2被note1包含
						else if (ev->type == MidiEventType::NoteOn &&
							ev2->type == MidiEventType::NoteOn)
						{
							noteOnEv = (NoteOnEvent*)ev;
							noteOnEv2 = (NoteOnEvent*)(node->next->elem);

							//2.1 note2被note1包含，移除note2
							if (noteOnEv2->endTick <= noteOnEv->endTick)
							{
								next = node->next;
								notes.Remove(next);
								DEL(next);
								next = notes.ContainsAtNode(noteOnEv2->noteOffEvent, node);
								notes.Remove(next);
								DEL(next);
								DEL(noteOnEv2->noteOffEvent);
								DEL(noteOnEv2);
							}
							//2.2 note2和note1交错, 连接note2到note1中
							else
							{
								next = node->next;
								notes.Remove(next);
								DEL(next);
								next = notes.ContainsAtNode(noteOnEv->noteOffEvent, node);
								notes.Remove(next);
								DEL(next);
								DEL(noteOnEv->noteOffEvent);

								noteOnEv->noteOffEvent = noteOnEv2->noteOffEvent;
								noteOnEv->endTick = noteOnEv->noteOffEvent->startTick;
								DEL(noteOnEv2);
							}

							next = node;

						}
						else {
							next = node->next->next;
						}
					}
				}

				//把合并好的notes重新排序，并填入原始track中
				for (int j = firstIdx; j < 128; j++)
				{
					if (eventsAtNotes[j].Empty())
						continue;
					newNotes->Merge(eventsAtNotes[j]);
				}

				newNotes->Sort(MidiEventTickCompare);

				if (prevNode == nullptr) {
					midiEventList->AddBefore(prevNode,
						newNotes->GetHeadNode(), newNotes->GetLastNode(),
						newNotes->Size());
				}
				else {
					midiEventList->AddBack(prevNode,
						newNotes->GetHeadNode(), newNotes->GetLastNode(),
						newNotes->Size());
				}

				newNotes->Clear();


				//清空临时数据,为下一次计算作准备
				orgNodeNext = orgNode;
				startNode = nullptr;
				prevNode = nullptr;
				noteCount = 0;
				isSetEndTick = true;
				isSetStartTick = true;
			}
		}
	}

	//按从小到大顺序排列
	bool MidiFile::MidiEventTickCompare(MidiEvent* a, MidiEvent* b)
	{
		return a->startTick < b->startTick;
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
	int MidiFile::ParseTrackChuck(int trackIdx)
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
		int parseRet = -1;

		float sec = 0;
		int eventCount = 0;
		bool isPassEvents = false;  //跳过事件解析

		startParseTime = chrono::high_resolution_clock::now();

		while (midiReader->getReadCursor() < midiReader->getWriteCursor() && !isStopParse)
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

			if (!isPassEvents)
				parseRet = ParseEvent(*track, trackIdx);
			else {
				parseRet = PassParseEvent(*track);
				if (parseRet == 2)
				{
					parseRet = 0;
					break;
				}
				continue;
			}

			//
			if (isEnableParseLimitTime)
			{
				eventCount++;
				if (eventCount > 3000)
				{
					chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
					sec = (chrono::duration_cast<chrono::duration<double>>(t2 - startParseTime)).count();

					if (sec > curtTrackParseSec) {
						curtTrackParseSec = perTrackParseSec;
						isPassEvents = true;
					}

					eventCount = 0;
				}
			}

			//
			if (parseRet == -1)   //解析出错
				break;
			if (parseRet == 2)  //当前音轨数据解析结束
			{
				if (isEnableParseLimitTime)
				{
					sec = (chrono::duration_cast<chrono::microseconds>(
						chrono::high_resolution_clock::now() - startParseTime).count() * 0.001f);

					if (sec > curtTrackParseSec)
						curtTrackParseSec = perTrackParseSec;
					else
						curtTrackParseSec = perTrackParseSec + curtTrackParseSec - sec;

				}

				parseRet = 0;
				break;
			}
		}

		midiTracks.push_back(track);

		if (isPassEvents)
			isFullParsed = false;

		return parseRet;
	}

	int MidiFile::ParseEvent(MidiTrack& track, int trackIdx)
	{
		switch (lastParseEventNum)
		{
		case 0x9:
		{
			int note = midiReader->read<byte>();
			int velocity = midiReader->read<byte>();
			LinkedListNode<MidiEvent*>* noteOnEventNode;

			if (velocity == 0)
			{
				noteOnEventNode = track.FindNoteOnEvent(note, lastParseEventChannel);
				if (noteOnEventNode == nullptr || noteOnEventNode->elem == nullptr)
					break;

				NoteOnEvent* noteOnEvent = (NoteOnEvent*)noteOnEventNode->elem;

				//优化midi音符数量，对于tick数(时长)非常小的NoteOn事件采取忽略策略
				if (enableMidiEventCountOptimize && curtParseTickCount - noteOnEvent->startTick < 5 && noteOnEvent->channel != 9)
				{
					track.RemoveEvent(noteOnEventNode);
					break;
				}

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
				bool isNeed = true;

				//开启按键数量控制优化(根据按键事件的startTick相同情况下，维持一定的数量的midievent, 超过的将不计数)
				if (enableMidiEventCountOptimize)
					isNeed = track.IsNeedNoteOnEvents(lastParseEventChannel, curtParseTickCount, keepSameStartTickNoteOnEventsCount);
				if (!isNeed)
					break;

				NoteOnEvent* noteOnEvent = new NoteOnEvent();
				noteOnEvent->startTick = curtParseTickCount;
				noteOnEvent->note = note;
				noteOnEvent->velocity = velocity;
				noteOnEvent->channel = lastParseEventChannel;
				noteOnEventNode = track.AddEvent(noteOnEvent);
			}
		}
		break;

		case 0x8:
		{
			int note = midiReader->read<byte>();
			int velocity = midiReader->read<byte>();

			LinkedListNode<MidiEvent*>* noteOnEventNode = track.FindNoteOnEvent(note, lastParseEventChannel);
			if (noteOnEventNode == nullptr || noteOnEventNode->elem == nullptr)
				break;

			NoteOnEvent* noteOnEvent = (NoteOnEvent*)noteOnEventNode->elem;


			//优化midi音符数量，对于tick数(时长)非常小的NoteOn事件采取忽略策略
			if (enableMidiEventCountOptimize && curtParseTickCount - noteOnEvent->startTick < 5)
			{
				track.RemoveEvent(noteOnEventNode);
				break;
			}

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
			case 0x00:  //轨道音序
				midiReader->read<byte>();
				midiReader->read<uint16_t>();
				break;

			case 0x01:case 0x02:case 0x03:
			case 0x04:case 0x05:case 0x06:
			case 0x07:case 0x08:case 0x09:
			{
				TextEvent* textEvent = new TextEvent();
				textEvent->startTick = curtParseTickCount;

				switch (type)
				{
				case 0x01:
					if (midiTracks.empty())
						textEvent->textType = MidiTextType::Comment;
					else
						textEvent->textType = MidiTextType::GeneralText;
					break;

				case 0x02:
					textEvent->textType = MidiTextType::Copyright;
					break;

				case 0x03:
					if (midiTracks.empty())
						textEvent->textType = MidiTextType::Title;
					else
						textEvent->textType = MidiTextType::TrackName;
					break;

				case 0x04: textEvent->textType = MidiTextType::InstrumentName; break;
				case 0x05: textEvent->textType = MidiTextType::Lyric; break;
				case 0x06: textEvent->textType = MidiTextType::Marker; break;
				case 0x07: textEvent->textType = MidiTextType::StartPos; break;
				case 0x08: textEvent->textType = MidiTextType::ProgramName; break;
				case 0x09: textEvent->textType = MidiTextType::DeviceName; break;
				}

				uint32_t len = ReadDynamicValue(*midiReader);
				if (len != 0)
				{
					byte* byteCodes = (byte*)malloc(len + 1);
					if (byteCodes != nullptr)
					{
						midiReader->read(byteCodes, 0, len);
						byteCodes[len] = 0;
						textEvent->text.assign((const char*)byteCodes);
						free(byteCodes);
					}
				}

				track.AddEvent(textEvent);
			}
			break;


			case 0x20: //MIDI通道
				midiReader->read<byte>();  // 1字节长度
				midiReader->read<byte>();
				break;

			case 0x21: //接口号码
				midiReader->read<byte>();  // 1字节长度
				midiReader->read<byte>();
				break;

			case 0x51: //速度设置
			{
				midiReader->read<byte>();  // 长度
				TempoEvent* tempoEvent = new TempoEvent();
				tempoEvent->startTick = curtParseTickCount;
				tempoEvent->microTempo = (float)Read3BtyesToInt32(*midiReader);

				//有些midi文件的格式明明是SyncTracks，但速度设置却没有放在全局0轨道中，而把速度设置放在了其他轨道，
				//此时通过把速度设置事件迁移到0轨道来适配标准格式
				if (midiTracks.size() != 0 &&
					midiTracks[0] != nullptr &&
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

			case 0x54:
			{
				midiReader->read<byte>(); // 长度
				SmpteEvent* smpteEvent = new SmpteEvent();
				smpteEvent->startTick = curtParseTickCount;
				smpteEvent->hr = midiReader->read<byte>();
				smpteEvent->mn = midiReader->read<byte>();
				smpteEvent->sec = midiReader->read<byte>();
				smpteEvent->fr = midiReader->read<byte>();
				smpteEvent->ff = midiReader->read<byte>();
				track.AddEvent(smpteEvent);
			}
			break;

			case 0x58: //节拍设置
			{
				midiReader->read<byte>(); // 长度
				TimeSignatureEvent* timeSignatureEvent = new TimeSignatureEvent();
				timeSignatureEvent->startTick = curtParseTickCount;
				timeSignatureEvent->numerator = midiReader->read<byte>();
				timeSignatureEvent->denominator = midiReader->read<byte>();
				timeSignatureEvent->metronomeCount = midiReader->read<byte>();
				timeSignatureEvent->nCount32ndNotesPerQuarterNote = midiReader->read<byte>();

				//有些midi文件的格式明明是SyncTracks，但全局设置却没有放在全局0轨道中，而把全局设置放在了其他轨道，
				//此时通过把全局设置事件迁移到0轨道来适配标准格式
				if (midiTracks.size() != 0 &&
					midiTracks[0] != nullptr &&
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

			case 0x59: //调号设置
			{
				midiReader->read<byte>(); // 长度
				KeySignatureEvent* keySignatureEvent = new KeySignatureEvent();
				keySignatureEvent->startTick = curtParseTickCount;
				keySignatureEvent->sf = (int8_t)midiReader->read<byte>();
				keySignatureEvent->mi = midiReader->read<byte>();
				keySignatureEvent->track = trackIdx;

				//有些midi文件的格式明明是SyncTracks，但全局设置却没有放在全局0轨道中，而把全局设置放在了其他轨道，
				//此时通过把全局设置事件迁移到0轨道来适配标准格式
				if (midiTracks.size() != 0 &&
					midiTracks[0] != nullptr &&
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
				unknownEvent->startTick = curtParseTickCount;
				unknownEvent->codeType = type;
				uint32_t len = ReadDynamicValue(*midiReader);
				if (len <= 0)
					break;

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

	//跳过解析的事件
	int MidiFile::PassParseEvent(MidiTrack& track)
	{
		switch (lastParseEventNum)
		{
		case 0x8:
		case 0x9:
		case 0xA:
		case 0xB:
		case 0xE:
			midiReader->read<int16_t>();
			break;

		case 0xC:
		case 0xD:
			midiReader->read<byte>();
			break;

		case 0xF0:
			do
			{
				byte b = midiReader->read<byte>();
				if (b == 0xF7)
					break;
			} while (true);
			break;

		case 0xFF:
		{
			byte type = midiReader->read<byte>();

			switch (type)
			{
			case 0x00:
				midiReader->setReadCursor(midiReader->getReadCursor() + 3);
				break;
			case 0x03:
			case 0x04:
			{
				uint32_t len = ReadDynamicValue(*midiReader);
				if (len != 0)
					midiReader->setReadCursor(midiReader->getReadCursor() + len);
			}
			break;


			case 0x20: //MIDI通道
			case 0x21: //接口号码
				midiReader->read<int16_t>();
				break;

			case 0x51:
				midiReader->read<byte>();  // 长度
				Read3BtyesToInt32(*midiReader);
				break;

			case 0x54:
				midiReader->setReadCursor(midiReader->getReadCursor() + 6);
				break;

			case 0x58:
				midiReader->setReadCursor(midiReader->getReadCursor() + 5);
				break;

			case 0x59:
				midiReader->setReadCursor(midiReader->getReadCursor() + 3);
				break;

			case 0x2F:
				midiReader->read<byte>();  //FF2F00
				return 2;  //当前音轨数据解析结束

			default:
			{
				uint32_t len = ReadDynamicValue(*midiReader);
				if (len <= 0)
					break;
				midiReader->setReadCursor(midiReader->getReadCursor() + len);
				break;
			}
			}
			break;
		}
		break;

		}

		return 0; //继续解析当前音轨数据
	}
}
