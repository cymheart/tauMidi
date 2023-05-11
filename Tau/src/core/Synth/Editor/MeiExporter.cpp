#include"MeiExporter.h"
#include"Track.h"
#include"Midi/MidiEvent.h"
#include"Editor.h"
#include"Midi/MidiEvent.h"
#include <algorithm>


namespace tau
{
	void AloneNote::SetNote(NoteOnEvent* note) {
		this->note = note;
		startTick = note->startTick;
		endTick = note->endTick;
	}

	Chord::Chord(const Chord& org)
	{
		startTick = org.startTick;
		endTick = org.endTick;
		//
		AloneNote* orgAloneNote;
		for (int i = 0; i < org.aloneNotes.size(); i++) {
			orgAloneNote = org.aloneNotes[i];
			AloneNote* newAloneNote = new AloneNote(*orgAloneNote);
			aloneNotes.push_back(newAloneNote);
		}
	}

	Chord::~Chord()
	{
		for (int i = 0; i < aloneNotes.size(); i++) 
			delete aloneNotes[i];
		aloneNotes.clear();
	}

	Tuplet::~Tuplet()
	{
		for (int i = 0; i < elems.size(); i++)
			delete elems[i];
		elems.clear();
	}


	TupletFrag::~TupletFrag()
	{
		if (elem != nullptr) {
			delete elem;
			elem = nullptr;
		}
	}


	TupletBucket::~TupletBucket()
	{
		for (int i = 0; i < elems.size(); i++)
			delete elems[i];
		elems.clear();
	}


	void Tie::GetAllAloneNotes(vector<AloneNote*>& aloneNotes)
	{
		GetChildAloneNotes(aloneNotes, this);
	}

	void Tie::GetChildAloneNotes(vector<AloneNote*>& aloneNotes, Tie* cTie)
	{
		vector<VirElem*>& cElems = cTie->elems;
		VirElem* elem;
		for (int i = 0; i < cElems.size(); i++) {
			elem = cElems[i];
			if (elem->Type() == ALONE_NOTE) {
				aloneNotes.push_back((AloneNote*)elem);
			}
			else if (elem->Type() == TIE) {
				GetChildAloneNotes(aloneNotes, (Tie*)elem);
			}
			else if (elem->Type() == CHORD) {
				Chord* chord = (Chord*)elem;
				for (int j = 0; j < chord->aloneNotes.size(); j++)
					aloneNotes.push_back(chord->aloneNotes[j]);
			}
		}
	}



	MeiExporter::MeiExporter(Editor* editor)
	{
		this->editor = editor;

		//音符时值
		double _noteTimes[] = {
			1, 1 / 2.0, 1 / 4.0, 1 / 8.0, 1 / 16.0,  1 / 32.0, 1 / 64.0, 1 / 128.0, 1 / 256.0,
			1 + 1 / 2.0, 1 / 2.0 + 1 / 4.0, 1 / 4.0 + 1 / 8.0, 1 / 8.0 + 1 / 16.0, 1 / 16.0 + 1 / 32.0, 1 / 32.0 + 1 / 64.0, 1 / 64.0 + 1 / 128.0, 1 / 128.0 + 1 / 256.0,
			1 + 1 / 2.0 + 1 / 4.0, 1 / 2.0 + 1 / 4.0 + 1 / 8.0, 1 / 4.0 + 1 / 8.0 + 1 / 16.0, 1 / 8.0 + 1 / 16.0 + 1 / 32.0, 1 / 16.0 + 1 / 32.0 + 1 / 64.0,
			1 / 32.0 + 1 / 64.0 + 1 / 128.0, 1 / 64.0 + 1 / 128.0 + 1 / 256.0, 1 / 128.0 + 1 / 256.0 + 1 / 512.0,
		};

		noteTimeLen = sizeof(_noteTimes) / sizeof(double);
		noteTimes = new double[noteTimeLen];
		memcpy(noteTimes, _noteTimes, sizeof(_noteTimes));


		//音符时值对应音符名称
		int32_t _noteTypes[] = {
			NOTE_1, NOTE_2, NOTE_4, NOTE_8, NOTE_16, NOTE_32, NOTE_64, NOTE_128, NOTE_256,
			NOTE_1_D, NOTE_2_D, NOTE_4_D, NOTE_8_D, NOTE_16_D,  NOTE_32_D, NOTE_64_D, NOTE_128_D,
			NOTE_1_DD, NOTE_2_DD, NOTE_4_DD, NOTE_8_DD, NOTE_16_DD,  NOTE_32_DD, NOTE_64_DD, NOTE_128_DD,
		};

		int32_t len = sizeof(_noteTypes) / sizeof(int32_t);
		noteTypes = new int32_t[len];
		memcpy(noteTypes, _noteTypes, sizeof(_noteTypes));

		//音符名称排序
		int32_t _noteTypeOrders[] = {
			NOTE_1, NOTE_2, NOTE_4, NOTE_8, NOTE_16, NOTE_32, NOTE_1_D, NOTE_2_D, NOTE_4_D, NOTE_8_D, NOTE_16_D,
			NOTE_32_D, NOTE_1_DD, NOTE_2_DD, NOTE_4_DD, NOTE_8_DD, NOTE_16_DD, NOTE_64, NOTE_64_D,
			NOTE_128, NOTE_128_D, NOTE_256,NOTE_32_DD, NOTE_64_DD, NOTE_128_DD
		};
		noteTypeOrderLen = sizeof(_noteTypeOrders) / sizeof(int32_t);
		noteTypeOrders = new int32_t[noteTypeOrderLen];
		memcpy(noteTypeOrders, _noteTypeOrders, sizeof(_noteTypeOrders));

	}

	MeiExporter::~MeiExporter()
	{
		if (noteTimes != nullptr) {
			delete noteTimes;
			noteTimes = nullptr;
		}

		if (noteTypes != nullptr) {
			delete noteTypes;
			noteTypes = nullptr;
		}

	}

	//当前时间curtPlaySec往前处理一个sec的时间长度的所有midi事件
	void MeiExporter::Execute()
	{
		MidiMarkerList* midiMarkerList = editor->GetMidiMarkerList();
		qNoteTicks = midiMarkerList->GetTickForQuarterNote();
		vector<Track*>& tracks = editor->GetTracks();
		MeasureInfo* measureInfo = editor->GetMeasureInfo();
		int measureCount = measureInfo->GetMeasureCount();

		float measureStartSec, measureEndSec;
		for (int i = 1; i <= measureCount; i++) 
		{
			measureStartSec = measureInfo->GetMeasureStartSec(i);
			measureEndSec = measureInfo->GetMeasureEndSec(i);

			int startTick = measureInfo->GetMeasureStartTick(i);
			int endTick = measureInfo->GetMeasureEndTick(i);
			int beatCount = measureInfo->GetMeasureBeatCount(i);

			vector<NoteOnEvent*> notes;

			for (int j = 0; j < tracks.size(); j++)
			{
				notes.clear();
				CollectMeasureNotes(tracks[j], measureStartSec, measureEndSec, notes);
				Parse(i, notes);
			}
		}


		//
		//ProcessSimpleModeTrack(simpleModeTrack, isDirectGoto);
	}


	/// <summary>
	/// 搜集小节notes
	/// </summary>
	void MeiExporter::CollectMeasureNotes(
		Track* track, float measureStartSec, float measureEndSec, vector<NoteOnEvent*>& notes)
	{
		MidiEvent* ev;

		//重新搜集跨小节notes
		vector<MidiEvent*>& evs = track->reProcessMidiEvents;
		if (!evs.empty()) {
			for (int i = evs.size() - 1; i >= 0; i--) {
				ev = evs[i];
				notes.push_back((NoteOnEvent*)evs[i]);
				if (!(ev->startSec <= measureEndSec && ev->endSec > measureEndSec))
					evs.erase(evs.begin() + i, evs.begin() + i + 1);
			}
		}

		//搜集小节notes
		LinkedList<MidiEvent*>* eventList;
		InstFragment* instFrag;
		auto& instFragmentBranchs = track->instFragmentBranchs;
		for (int i = 0; i < instFragmentBranchs.size(); i++)
		{
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[i]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[i]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				instFrag = *frag_it;
				eventList = &(instFrag->midiEvents);

				LinkedListNode<MidiEvent*>* node = instFrag->eventOffsetNode;
				for (; node; node = node->next)
				{
					ev = node->elem;
					if (ev->type != MidiEventType::NoteOn)
						continue;

					if (ev->startSec < measureEndSec && ev->endSec > measureEndSec)
						track->reProcessMidiEvents.push_back(ev);

					if (ev->startSec >= measureStartSec && ev->startSec < measureEndSec)
						notes.push_back((NoteOnEvent*)ev);
					else
						break;

				}

				instFrag->eventOffsetNode = node;
			}
		}
	}

	//解析小节中的所有音符
	void MeiExporter::Parse(int measureIdx, vector<NoteOnEvent*>& notes) 
	{
		if (notes.empty())
			return;

		MeasureInfo* measureInfo = editor->GetMeasureInfo();
		int32_t measureStartTick = measureInfo->GetMeasureStartTick(measureIdx);
		int32_t measureEndTick = measureInfo->GetMeasureEndTick(measureIdx);
		int beatCount = measureInfo->GetMeasureBeatCount(measureIdx);
		//一拍对应的tick数量
		int perBeatTicks = (measureEndTick - measureStartTick + 1) / beatCount;

		//4分音符 perBeatBaseNote = 4
		//8分音符 perBeatBaseNote = 8
		perBeatBaseNote = qNoteTicks / perBeatTicks * 4; 
		perBeatBaseNoteOrder = GetNoteTypeOrderIdx(perBeatBaseNote);
		//
		vector<AloneNote*> aloneNotes;
		NoteOnEvent* note;
		AloneNote* aloneNote, * aloneNote2, * minDiffAloneNote = nullptr;
		bool havModifyNote = true;
		bool flag = true;
		float err = 0.1f; //误差值
		float err2 = 0.07f; //误差值

		//为所有真实音符生成虚拟音符,以方便修改tick
		for (int i = 0; i < notes.size(); i++) {
			note = notes[i];
			//当一个真实音符长度超过当前小节的长度时，
			//调整对应虚拟音符的长度，最长到小节结束位置
			if (note->endTick >= measureEndTick) {
				aloneNote = new AloneNote();
				aloneNote->SetNote(note);
				aloneNote->endTick = measureEndTick - 1;
			}
			else {
				aloneNote = new AloneNote();
				aloneNote->SetNote(note);
			}
			aloneNotes.push_back(aloneNote);
		}

		//1.消除音符之间的距离的小误差造成的间隙,使得音符一定程度上可以互相对齐
		AlginNotes(aloneNotes, measureStartTick, measureEndTick, perBeatTicks);

	
		//2.分割时间上互相重叠交错的音符，使得所有音符形成简单的位置对齐
		//所谓简单位置对齐，就是比如，如果是互相重叠的两个音符，
		//则通过分割后，生成了两个首尾对齐的子音符组1，以及其它两个单独子音符2，3,
		//音符组1作为整体和2，3这几个子音符之间首尾相连
		AloneNote* clipAloneNote;
		for (int i = 0; i < aloneNotes.size(); i++)
		{
			aloneNote = aloneNotes[i];
			for (int j = 0; j < aloneNotes.size(); j++) {
				if (i == j)continue;
				aloneNote2 = aloneNotes[j];

				//两个note的tick范围相同，将不作处理
				if (aloneNote2->startTick == aloneNote->startTick &&
					aloneNote2->endTick == aloneNote->endTick)
					continue;

				//aloneNote1:   |---| 
				//aloneNote2: |--------| 
				if (aloneNote2->startTick < aloneNote->startTick && 
					aloneNote2->endTick > aloneNote->endTick)
				{
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote2->note);
					clipAloneNote->startTick = aloneNote2->startTick;
					clipAloneNote->endTick = aloneNote->startTick-1;
					aloneNotes.push_back(clipAloneNote);
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote->note);
					clipAloneNote->startTick = aloneNote->startTick;
					clipAloneNote->endTick = aloneNote->endTick;
					aloneNotes.push_back(clipAloneNote);
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote2->note);
					clipAloneNote->startTick = aloneNote->startTick;
					clipAloneNote->endTick = aloneNote->endTick;
					aloneNotes.push_back(clipAloneNote);
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote2->note);
					clipAloneNote->startTick = aloneNote->endTick + 1;
					clipAloneNote->endTick = aloneNote2->endTick;
					aloneNotes.push_back(clipAloneNote);

				}
				//aloneNote1: |------|  ,  |------|   ,  |------| ,  |------|    
				//aloneNote2:   |--|    ,  |--|       ,  |------| ,      |--|  
				else if (aloneNote2->startTick >= aloneNote->startTick &&
					aloneNote2->startTick < aloneNote->endTick &&
					aloneNote2->endTick >= aloneNote->startTick &&
					aloneNote2->endTick <= aloneNote->endTick)
				{
					if (aloneNote->startTick != aloneNote2->startTick) {
						clipAloneNote = new AloneNote();
						clipAloneNote->SetNote(aloneNote->note);
						clipAloneNote->startTick = aloneNote->startTick;
						clipAloneNote->endTick = aloneNote2->startTick - 1;
						aloneNotes.push_back(clipAloneNote);
					}
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote->note);
					clipAloneNote->startTick = aloneNote2->startTick;
					clipAloneNote->endTick = aloneNote2->endTick;
					aloneNotes.push_back(clipAloneNote);
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote2->note);
					clipAloneNote->startTick = aloneNote2->startTick;
					clipAloneNote->endTick = aloneNote2->endTick;
					aloneNotes.push_back(clipAloneNote);

					if (aloneNote->endTick != aloneNote2->endTick) {
						clipAloneNote = new AloneNote();
						clipAloneNote->SetNote(aloneNote->note);
						clipAloneNote->startTick = aloneNote2->endTick + 1;
						clipAloneNote->endTick = aloneNote->endTick;
						aloneNotes.push_back(clipAloneNote);
					}

				
					i = -1;
					auto it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote);
					aloneNotes.erase(it);
					delete aloneNote;
					//
					it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote2);
					aloneNotes.erase(it);
					delete aloneNote2;
					break;

				}
				//aloneNote1:  |------|     ,  |------|    
				//aloneNote2:  |---------|   ,     |-------|
				else if (aloneNote2->startTick >= aloneNote->startTick &&
					aloneNote2->startTick < aloneNote->endTick)
				{
					if (aloneNote->startTick != aloneNote2->startTick) {
						clipAloneNote = new AloneNote();
						clipAloneNote->SetNote(aloneNote->note);
						clipAloneNote->startTick = aloneNote->startTick;
						clipAloneNote->endTick = aloneNote2->startTick - 1;
						aloneNotes.push_back(clipAloneNote);
					}
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote->note);
					clipAloneNote->startTick = aloneNote2->startTick;
					clipAloneNote->endTick = aloneNote->endTick;
					aloneNotes.push_back(clipAloneNote);
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote2->note);
					clipAloneNote->startTick = aloneNote2->startTick;
					clipAloneNote->endTick = aloneNote->endTick;
					aloneNotes.push_back(clipAloneNote);

					if (aloneNote2->endTick > aloneNote->endTick) {
						clipAloneNote = new AloneNote();
						clipAloneNote->SetNote(aloneNote2->note);
						clipAloneNote->startTick = aloneNote->endTick + 1;
						clipAloneNote->endTick = aloneNote2->endTick;
						aloneNotes.push_back(clipAloneNote);
					}
					
					i = -1;
					auto it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote);
					aloneNotes.erase(it);
					delete aloneNote;

					it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote2);
					aloneNotes.erase(it);
					delete aloneNote2;
					break;
				}
				//aloneNote1:     |------|   ,         |------|    
				//aloneNote2:  |---------|   ,     |-------|
				else if (aloneNote2->endTick > aloneNote->startTick &&
					aloneNote2->endTick <= aloneNote->endTick)
				{
					if (aloneNote->endTick != aloneNote2->endTick) {
						clipAloneNote = new AloneNote();
						clipAloneNote->SetNote(aloneNote->note);
						clipAloneNote->startTick = aloneNote2->endTick + 1;
						clipAloneNote->endTick = aloneNote->endTick;
						aloneNotes.push_back(clipAloneNote);
					}

					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote->note);
					clipAloneNote->startTick = aloneNote->startTick;
					clipAloneNote->endTick = aloneNote2->endTick;
					aloneNotes.push_back(clipAloneNote);
					//
					clipAloneNote = new AloneNote();
					clipAloneNote->SetNote(aloneNote2->note);
					clipAloneNote->startTick = aloneNote->startTick;
					clipAloneNote->endTick = aloneNote2->endTick;
					aloneNotes.push_back(clipAloneNote);

					if (aloneNote2->startTick < aloneNote->startTick) {
						clipAloneNote = new AloneNote();
						clipAloneNote->SetNote(aloneNote2->note);
						clipAloneNote->startTick = aloneNote2->startTick;
						clipAloneNote->endTick = aloneNote->startTick - 1;
						aloneNotes.push_back(clipAloneNote);
					}
					

					i = -1;
					auto it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote);
					aloneNotes.erase(it);
					delete aloneNote;
					//
					it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote2);
					aloneNotes.erase(it);
					delete aloneNote2;
					break;
				}
			}
		}


		//3.组合单个note为和声
		vector<VirElem*> virElems;
		for (int i = 0; i < aloneNotes.size(); i++)
		{
			Chord* chord = nullptr;
			aloneNote = aloneNotes[i];

			for (int j = 0; j < aloneNotes.size(); j++)
			{
				if (i == j)continue;
				aloneNote2 = aloneNotes[j];
				//两个note的tick范围相同，组和为和声
				if (aloneNote2->startTick == aloneNote->startTick &&
					aloneNote2->endTick == aloneNote->endTick)
				{
					if (chord == nullptr) {
						chord = new Chord();
						chord->startTick = aloneNote->startTick;
						chord->endTick = aloneNote->endTick;
						chord->aloneNotes.push_back(aloneNote);
					}

					chord->aloneNotes.push_back(aloneNote2);
				}
			}

			//
			if (chord != nullptr) {
				virElems.push_back(chord);
				for (int j = 0; j < chord->aloneNotes.size(); j++) {
					auto it = std::find(aloneNotes.begin(), aloneNotes.end(), chord->aloneNotes[j]);
					aloneNotes.erase(it);
				}
			}
			else {
				virElems.push_back(aloneNote);
				auto it = std::find(aloneNotes.begin(), aloneNotes.end(), aloneNote);
				aloneNotes.erase(it);
			}

			i = -1;
		}


		//4.在音符元素间隙之间添加休止符元素,因为有的连音会用到休止符
		//先按tick先后排序小节中所有元素
		sort(virElems.begin(), virElems.end(), [](VirElem* a, VirElem* b) {
			return a->startTick < b->startTick;
			});
		vector<VirElem*> resultElems;
		Rest* rest;
		int prevTick = measureStartTick;
		for (int i = 0; i < virElems.size(); i++)
		{
			if (virElems[i]->startTick > prevTick)
			{
				rest = new Rest();
				rest->startTick = prevTick;
				rest->endTick = virElems[i]->startTick - 1;
				resultElems.push_back(rest);
			}

			resultElems.push_back(virElems[i]);
			prevTick = virElems[i]->endTick + 1;
		}

		//如果小节尾部有空隙，将在小节所有元素尾部添加休止符
		if (!virElems.empty() &&
			virElems[virElems.size() - 1]->endTick < measureEndTick)
		{
			rest = new Rest();
			rest->startTick = virElems[virElems.size() - 1]->endTick + 1;
			rest->endTick = measureEndTick;
			resultElems.push_back(rest);
		}


		//5.找出小节中的连音组
		virElems = CreateTuplets(resultElems);


		//6.先移除休止符，这样为后面分割音符算法好处理，后面将会再加回来
		resultElems.clear();
		for (int i = 0; i < virElems.size(); i++)
		{
			if (virElems[i]->Type() != REST) {
				resultElems.push_back(virElems[i]);
				virElems[i] = nullptr;
			}
		}	
		for (auto it = virElems.begin(); it != virElems.end(); ++it) {
			if (*it != nullptr) {
				delete *it;
			}
		}
		virElems.clear();
	


		//7.比较元素和小节的每拍分割线间的位置关系,如果音符头部或尾部位置稍微出一点拍线位置，
		//那么将忽略多出的部分，使得元素头部或尾部和拍线平齐	
		if (!resultElems.empty()) {
			int32_t beatTick = measureStartTick;
			VirElem* elem;
			float error = 0.05;
			while (beatTick <= measureEndTick) {
				for (int i = 0; i < resultElems.size(); i++)
				{
					elem = resultElems[i];
					float t = (float)abs(elem->startTick - beatTick) / perBeatTicks;
					if (t < error)elem->SetStartTick(beatTick);
					//
					t = (float)abs(elem->endTick - beatTick) / perBeatTicks;
					if (t < error)elem->SetEndTick(beatTick - 1);

					//
					int n = elem->endTick - elem->startTick + 1;
					float diff = (float)abs(n) / perBeatTicks;
					if (n < 0 || diff < error) {
						resultElems.erase(
							remove(resultElems.begin(), resultElems.end(), elem), 
							resultElems.end());
						delete elem;
						i = -1;
						continue;
					}

					if (elem->endTick <= beatTick)
						continue;
					break;
				}

				beatTick += perBeatTicks;
			}
		}


		//8.重新添加休止符
		vector<VirElem*> resultElems2;
		prevTick = measureStartTick;
		for (int i = 0; i < resultElems.size(); i++)
		{
			if (resultElems[i]->startTick > prevTick)
			{
				rest = new Rest();
				rest->startTick = prevTick;
				rest->endTick = resultElems[i]->startTick - 1;
				resultElems2.push_back(rest);
			}

			resultElems2.push_back(resultElems[i]);
			prevTick = resultElems[i]->endTick + 1;
		}

		//如果小节尾部有空隙，将在小节所有元素尾部添加休止符
		if (!resultElems.empty() &&
			resultElems[resultElems.size() - 1]->endTick < measureEndTick)
		{
			rest = new Rest();
			rest->startTick = resultElems[resultElems.size() - 1]->endTick + 1;
			rest->endTick = measureEndTick ;
			resultElems2.push_back(rest);
		}

		//9.用小节的四分音符为一拍的分割线分割当前所有note,形成以四分音符为一拍为单位的notes
		resultElems.clear();
		int32_t beatTick = measureStartTick + qNoteTicks;
		VirElem* virElem;
		for (auto it = resultElems2.begin(); it != resultElems2.end();)
		{
			virElem = *it;
			while (beatTick <= virElem->startTick)
				beatTick += perBeatTicks;

			if (virElem->endTick < beatTick) {
				resultElems.push_back(virElem);
				++it;
				continue;
			}

			if (beatTick > virElem->startTick && beatTick < virElem->endTick)
			{
				if (virElem->Type() == TUPLET) {
					resultElems.push_back(virElem);
					++it;
					continue;
				}
				//
				if (virElem->Type() == CHORD) {
					Chord* rightChord = (Chord*)virElem;
					Chord* leftChord = (Chord*)CopyElem(rightChord);
					leftChord->SetEndTick(beatTick - 1);
					resultElems.push_back(leftChord);
					rightChord->SetStartTick(beatTick);
	
				}
				else {
					AloneNote* rightAloneNote = (AloneNote*)virElem;
					AloneNote* leftAloneNote = (AloneNote*)CopyElem(rightAloneNote);
					leftAloneNote->SetEndTick(beatTick - 1);
					resultElems.push_back(leftAloneNote);
					rightAloneNote->SetStartTick(beatTick);
				}
			}
		}


		//10.对当前小节中每个音符时值格式化到标准几分音符时值
		//将按小节中以四分音符为一拍的范围进行格式化
		FormatMeasureNoteToStdType(resultElems, measureStartTick);


		//
	
	}

	//消除音符之间的距离的小误差造成的间隙,使得音符一定程度上可以互相对齐
	void MeiExporter::AlginNotes(
		vector<AloneNote*>& aloneNotes, 
		int32_t measureStartTick, int32_t measureEndTick, int perBeatTicks)
	{
		AloneNote* aloneNote, * aloneNote2;
		int noteTicks, diffTicks;
		float diff;
		float err = 0.1f; //误差值
		vector<AloneNote*> diffNotes;
		vector<int> diffNoteTypes;


		//当前音符头部位置和小节拍线位置比较
		int32_t beatTick = measureStartTick;
		for (int i = 0; i < aloneNotes.size(); i++)
		{
			aloneNote = aloneNotes[i];
			noteTicks = aloneNote->endTick - aloneNote->startTick + 1;
			int beatTick = measureStartTick;
			for (; beatTick <= measureEndTick; beatTick += perBeatTicks)
			{
				if (aloneNote->endTick+1 < beatTick) {
					diffTicks = beatTick - aloneNote->endTick;	
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						aloneNote->endTick = beatTick - 1;
						break;
					}
				}

				if (aloneNote->startTick > beatTick) {
					diffTicks = aloneNote->startTick - beatTick;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						aloneNote->startTick = beatTick;
						break;
					}
				}

				if (aloneNote->startTick < beatTick && aloneNote->endTick > beatTick) {
					diffTicks = beatTick - aloneNote->startTick;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						aloneNote->startTick = beatTick;
						break;
					}

					diffTicks = aloneNote->endTick - beatTick;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						aloneNote->endTick = beatTick - 1;
						break;
					}
				}
			}
		}
	
		//消除音符之间的距离的小误差造成的间隙,使得音符一定程度上可以互相对齐
		for (int i = 0; i < aloneNotes.size(); i++) 
		{
			aloneNote = aloneNotes[i];
			diffNotes.clear();
			diffNoteTypes.clear();

			for (int j = 0; j < aloneNotes.size(); j++) 
			{
				if (i == j)continue;
				aloneNote2 = aloneNotes[j];

				//1.当前音符的尾部位置和其它不相交音符的头部位置距离比较
				//aloneNote: |------|      
				//aloneNote2:           |------|
				if (aloneNote2->startTick > aloneNote->endTick+1)
				{
					diffTicks = aloneNote2->startTick - aloneNote->endTick + 1;
					noteTicks = aloneNote->endTick - aloneNote->startTick + 1;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						diffNotes.push_back(aloneNote2);
						diffNoteTypes.push_back(1);
					}
				}
				//2.当前音符的尾部位置和其它音符的头部位置距离比较
				//aloneNote: |------|      
				//aloneNote2:    |------|
				else if (aloneNote2->startTick != aloneNote->endTick+1 &&
					aloneNote2->startTick > aloneNote->startTick && 
					aloneNote2->endTick > aloneNote->endTick)
				{
					diffTicks = abs(aloneNote->endTick - aloneNote2->startTick + 1);
					noteTicks = aloneNote2->endTick - aloneNote2->startTick + 1;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						diffNotes.push_back(aloneNote2);
						diffNoteTypes.push_back(2);
					}
				}

				//3.当前音符的尾部位置和其它音符的尾部位置距离比较
				//aloneNote:       |------| ,  |------|     ,   |------|     ,   |----------|        
				//aloneNote2:  |------|     ,     |------|  , |-----------|  ,     |------|        
				if (aloneNote2->endTick != aloneNote->endTick &&
					aloneNote2->endTick > aloneNote->startTick && 
					aloneNote2->startTick < aloneNote->endTick)
				{
					diffTicks = abs(aloneNote2->endTick - aloneNote->endTick);
					noteTicks = aloneNote->endTick - aloneNote->startTick + 1;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						diffNotes.push_back(aloneNote2);
						diffNoteTypes.push_back(3);
					}
				}
		
				//4.当前音符的头部位置和其它音符的头部位置距离比较
				//aloneNote:       |------| ,  |------|     ,   |------|      ,   |----------|        
				//aloneNote2:  |------|     ,     |------|  , |-----------|   ,     |------|        
				else if (aloneNote2->startTick != aloneNote->startTick &&
					aloneNote2->endTick > aloneNote->startTick &&
					aloneNote2->startTick < aloneNote->endTick)
				{
					diffTicks = abs(aloneNote2->startTick - aloneNote->startTick);
					noteTicks = aloneNote->endTick - aloneNote->startTick + 1;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						diffNotes.push_back(aloneNote2);
						diffNoteTypes.push_back(4);
					}
				}

				//5.当前音符的头部位置和其它不相交音符的尾部位置距离比较
				//aloneNote:            |------|      
				//aloneNote2: |------|
				if (aloneNote2->endTick + 1 < aloneNote->startTick)
				{
					diffTicks = aloneNote->startTick - aloneNote2->endTick + 1;
					noteTicks = aloneNote2->endTick - aloneNote2->startTick + 1;
					diff = (float)diffTicks / (noteTicks + diffTicks);
					if (diff <= err || diffTicks < 15) {
						diffNotes.push_back(aloneNote2);
						diffNoteTypes.push_back(5);
					}
				}
			}


			//
			if (diffNotes.empty())
				continue;

			int stdStartTick = aloneNote->startTick;
			int stdEndTick = aloneNote->endTick;
			for (int i = 0; i < diffNoteTypes.size(); i++) 
			{
				if (diffNoteTypes[i] == 1) 
				{
					if (diffNotes[i]->startTick > aloneNote->endTick &&
						diffNotes[i]->startTick > stdEndTick) {
						stdEndTick = diffNotes[i]->startTick;
					}
				}
				
				if (diffNoteTypes[i] == 4)
				{
					if (diffNotes[i]->startTick < aloneNote->startTick &&
						diffNotes[i]->startTick < stdStartTick) {
						stdStartTick = diffNotes[i]->startTick;
					}
				}
			}

			if (stdEndTick == aloneNote->endTick) {
				for (int i = 0; i < diffNoteTypes.size(); i++)
				{
					if (diffNoteTypes[i] == 2) {
						stdEndTick = aloneNote->endTick;
					}
	
					if ( diffNoteTypes[i] == 3){
						if (diffNotes[i]->endTick > aloneNote->endTick &&
							diffNotes[i]->endTick > stdEndTick) {
							stdEndTick = diffNotes[i]->endTick;
						}	
					}
				}
				aloneNote->endTick = stdEndTick;
			}
			else {
				aloneNote->endTick = stdEndTick - 1;
			}

			aloneNote->startTick = stdStartTick;

			for (int i = 0; i < diffNoteTypes.size(); i++)
			{
				if (diffNoteTypes[i] == 1)
					diffNotes[i]->startTick = stdEndTick;
				else if (diffNoteTypes[i] == 2)
					diffNotes[i]->startTick = stdEndTick + 1;
				else if ( diffNoteTypes[i] == 3)
					diffNotes[i]->endTick = stdEndTick;
				else if (diffNoteTypes[i] == 4)
					diffNotes[i]->startTick = stdStartTick;
				else
					diffNotes[i]->endTick = stdStartTick - 1;
			}
		}
	}

	//生成连音
	vector<VirElem*> MeiExporter::CreateTuplets(vector<VirElem*>& elems)
	{
		vector<VirElem*> resultElems;
		if (elems.empty())
			return resultElems;

		vector<ElemFrag> frags = CreateFrags(elems);
		vector<Tuplet*> tuplets = CreateTuplets(frags);
		if (tuplets.empty())
			return elems;

		//
		for (int i = 0; i < tuplets.size(); i++)
			ModifyTupletElemTick(tuplets[i], elems);

		//把找到的连音组替换原元素队列中的所有元素
		int n = 0;
		Tuplet* tuplet = tuplets[n];
		for (int i = 0; i < elems.size(); i++)
		{
			if (tuplet != nullptr && 
				elems[i]->startTick == tuplet->endTick + 1) {
				++n;
				if (n < tuplets.size()) tuplet = tuplets[n];
				else tuplet = nullptr;
			}
			//
			if (tuplet == nullptr)
			{
				resultElems.push_back(elems[i]);
			}else {
				if (elems[i]->startTick < tuplet->startTick) {
					resultElems.push_back(elems[i]);
				}
				else if (elems[i]->startTick == tuplet->startTick) {
					resultElems.push_back(tuplet);
				}
			}
		}

		return resultElems;
	}


	void MeiExporter::ModifyTupletElemTick(VirElem* tupletElem, vector<VirElem*>& elems)
	{
		switch (tupletElem->Type())
		{
		case TUPLET_FRAG:
		{
			TupletFrag* frag = (TupletFrag*)tupletElem;
			if (frag->elem->startTick < frag->startTick)
			{
				VirElem* newElem = CopyElem(frag->elem);
				newElem->SetTickRange(frag->elem->startTick, frag->startTick - 1);
				auto it = std::find(elems.begin(), elems.end(), frag->elem);
				elems.insert(it, newElem);
				//
				frag->elem->startTick = frag->startTick;
			}

			if (frag->elem->endTick > frag->endTick)
			{
				VirElem* oldElem = frag->elem;
				VirElem* newElem = CopyElem(frag->elem);
				newElem->SetTickRange(frag->startTick, frag->endTick);
				frag->elem = newElem;
				oldElem->SetTickRange(frag->endTick + 1, oldElem->endTick);
			}
		}
		break;

		case TUPLET_BUCKET:
		{
			TupletBucket* bucket = (TupletBucket*)tupletElem;
			for (int i = 0; i < bucket->elems.size(); i++)
				ModifyTupletElemTick(bucket->elems[i], elems);
		}
		break;

		case TUPLET:
			Tuplet* tuplet = (Tuplet*)tupletElem;
			for (int i = 0; i < tuplet->elems.size(); i++)
			{
				tupletElem = tuplet->elems[i];
				ModifyTupletElemTick((Tuplet*)tupletElem, elems);
			}
			break;
		}
	}


	//生成连音组
	vector<Tuplet*> MeiExporter::CreateTuplets(vector<ElemFrag>& frags)
	{
		vector<vector<ElemBucket>> tupletBucketsGroup = GetTupletBucketGroup(frags);
		vector<Tuplet*> tuplets;
		//
		for (int i = 0; i < tupletBucketsGroup.size(); i++)
		{
			vector<ElemBucket>& buckets = tupletBucketsGroup[i];
			Tuplet* tuplet = new Tuplet();
			tuplet->startTick = buckets.front().frags.front().startTick;
			tuplet->endTick = buckets.back().frags.back().endTick;
			for (int j = 0; j < buckets.size(); j++)
			{
				TupletBucket* tupletBucket = CreateTupletBucket(buckets[j].frags);
				if (tupletBucket->elems.size() == 1) {
					tuplet->elems.push_back(tupletBucket->elems[0]);
					tupletBucket->elems.clear();
					delete tupletBucket;
				}else {
					tuplet->elems.push_back(tupletBucket);
				}
			}

			tuplets.push_back(tuplet);
		}

		return tuplets;
	}


	//生成连音分桶
	TupletBucket* MeiExporter::CreateTupletBucket(vector<ElemFrag>& frags)
	{
		vector<vector<ElemBucket>> tupletBucketsGroup = GetTupletBucketGroup(frags, false);
		vector<Tuplet*> tuplets;
		//
		for (int i = 0; i < tupletBucketsGroup.size(); i++) 
		{
			vector<ElemBucket>& buckets = tupletBucketsGroup[i];
			Tuplet* tuplet = new Tuplet();
			tuplet->startTick = buckets.front().frags.front().startTick;
			tuplet->endTick = buckets.back().frags.back().endTick;
			for (int j = 0; j < buckets.size(); j++) 
			{
				TupletBucket* tupletBucket = CreateTupletBucket(buckets[j].frags);
				if (tupletBucket->elems.size() == 1) {
					tuplet->elems.push_back(tupletBucket->elems[0]);
					tupletBucket->elems.clear();
					delete tupletBucket;
				}else {
					tuplet->elems.push_back(tupletBucket);
				}
			}

			tuplets.push_back(tuplet);
		}

		//
		TupletBucket* tupletBucket = new TupletBucket();
		int j = 0;
		ElemFrag* tfrag = nullptr;
		if(!tupletBucketsGroup.empty())
			tfrag = &(tupletBucketsGroup[j].front().frags.front());

		for (int i = 0; i < frags.size(); i++) {

			if (tfrag == nullptr || 
				frags[i].startTick != tfrag->startTick)
			{
				TupletFrag* tupletFrag = new TupletFrag();
				tupletFrag->startTick = frags[i].startTick;
				tupletFrag->endTick = frags[i].endTick;
				tupletFrag->elem = frags[i].elem;
				tupletBucket->elems.push_back(tupletFrag);
			}
			else {
				tupletBucket->elems.push_back(tuplets[j]);
				int nCount = 0;
				vector<ElemBucket>& buckets = tupletBucketsGroup[j];
				for (int n = 0; n < buckets.size(); n++)
					nCount += buckets[n].frags.size();
				i += nCount - 1;
				j++;
				if(j < tupletBucketsGroup.size())	
					tfrag = &(tupletBucketsGroup[j].front().frags.front());
			}

		}

		return tupletBucket;
	}


	//获取连音分桶组
	vector<vector<ElemBucket>> MeiExporter::GetTupletBucketGroup(vector<ElemFrag>& frags, bool isStrictMode)
	{
		int order;
		uint32_t startTick, endTick;
		uint32_t needBucketTicks;
		vector<vector<ElemBucket>> tupletBucketsGroup;
		//
		for (int offElemIdx = 0; offElemIdx < frags.size();)
		{
			int nTotalCount = frags.size() - offElemIdx;
			for (int nCount = 1; nCount < nTotalCount; nCount++)
			{
				vector<ElemBucket> leftBuckets;
				vector<ElemBucket> rightBuckets;
				ElemBucket bucket;
				needBucketTicks = 0;
				for (int i = offElemIdx; i < offElemIdx + nCount; i++)
				{
					ElemFrag& frag = frags[i];
					needBucketTicks += frag.endTick - frag.startTick + 1;
					ElemFrag efrag = { i, frag.startTick, frag.endTick, frag.elem};
					bucket.frags.push_back(efrag);
				}


				NoteValueInfo info = GetNoteValueInfo(needBucketTicks, false);
				order = GetNoteTypeOrderIdx(noteTypes[info.noteTimeIdx]);
				//排除掉标准时值音符元素,因为连音中的音符时值很少是标准音符时值
				if (isStrictMode && info.t >= 0.95f && order < 18) 
					break;
				
				bucket.ticks = needBucketTicks;
				leftBuckets.push_back(bucket);
				rightBuckets.push_back(bucket);

				ElemBucket leftBucket;
				leftBucket.startFrag = bucket.frags.front().idx - 1;
				leftBucket.bucketStartTick = bucket.frags.front().startTick - 1;
				leftBuckets.insert(leftBuckets.begin(),leftBucket);
				//
				ElemBucket rightBucket;
				rightBucket.startFrag = bucket.frags.back().idx + 1;
				rightBucket.bucketStartTick = bucket.frags.back().endTick + 1;
				rightBuckets.push_back(rightBucket);


				//
				while (true) 
				{
					ElemBucket& lastBucket = rightBuckets.back();
					if (!lastBucket.isEnable)
					{
						lastBucket.isEnable = true;
					}
					else {
						while (true)
						{
							int i;
							ElemBucket& lastBucket = rightBuckets.back();
							if (lastBucket.ticks < needBucketTicks)
							{
								if (lastBucket.frags.empty()) {
									i = lastBucket.startFrag;
									startTick = lastBucket.bucketStartTick;
								}
								else {
									ElemFrag& lastFrag = lastBucket.frags.back();
									ElemFrag& orgFrag = frags[lastFrag.idx];
									if (lastFrag.endTick == orgFrag.endTick) {
										i = lastFrag.idx + 1;
										startTick = orgFrag.endTick + 1;
									}
									else {
										i = lastFrag.idx;
										startTick = lastFrag.endTick + 1;
									}
								}

		
								//不足以生成needBucketTicks长度
								if (i >= frags.size())
									break;

								int32_t n = needBucketTicks - lastBucket.ticks;
								ElemFrag& orgFrag = frags[i];
								int32_t richTicks = orgFrag.endTick - startTick + 1;
								int32_t addTicks = 0;
								float m = (float)n / needBucketTicks;
								float t = abs((float)(n - richTicks) / richTicks);

								if (t > 0.05 && richTicks >= n) {
									//非严格模式下，不允许分割使用已有的音符，只能完整包含一个音符ticks
									if (!isStrictMode || m < 0.2)
										break;

									ElemFrag frag = { i, startTick, startTick + n, orgFrag.elem };
									lastBucket.frags.push_back(frag);
									lastBucket.ticks += n;
								}
								else {
									ElemFrag frag = { i, startTick, orgFrag.endTick, orgFrag.elem };
									lastBucket.frags.push_back(frag);
									lastBucket.ticks += richTicks;
								}

								t = (float)lastBucket.ticks / needBucketTicks;
								if (t >= 0.98)
									lastBucket.ticks = needBucketTicks;

							}
							else {
								break;
							}
						}
					}

					//
					ElemBucket& firstBucket = leftBuckets.front();
					if (!firstBucket.isEnable)
					{
						firstBucket.isEnable = true;
					}
					else {
						while (true)
						{
							int i;
							ElemBucket& firstBucket = leftBuckets.front();
							if (firstBucket.ticks < needBucketTicks)
							{
								if (firstBucket.frags.empty()) {
									i = firstBucket.startFrag;
									endTick = firstBucket.bucketStartTick;
								}
								else {
									ElemFrag& firstFrag = firstBucket.frags.front();
									ElemFrag& orgFrag = frags[firstFrag.idx];
									if (firstFrag.startTick == orgFrag.startTick) {
										i = firstFrag.idx - 1;
										endTick = orgFrag.startTick - 1;
									}
									else {
										i = firstFrag.idx;
										endTick = firstFrag.startTick - 1;
									}
								}

								//不足以生成needBucketTicks长度
								if (i < 0)
									break;

								int32_t n = needBucketTicks - firstBucket.ticks;
								ElemFrag& orgFrag = frags[i];
								int32_t richTicks = endTick - orgFrag.startTick + 1;
								float m = (float)n / needBucketTicks;
								float t = abs((float)(n - richTicks) / richTicks);
								if (t > 0.05 && richTicks >= n) 
								{
									//非严格模式下，不允许分割使用已有的音符，只能完整包含一个音符ticks
									if (!isStrictMode || m < 0.2)
										break;

									ElemFrag frag = { i, endTick - n, endTick, orgFrag.elem };
									firstBucket.frags.insert(firstBucket.frags.begin(), frag);
									firstBucket.ticks += n;
								}
								else {
									ElemFrag frag = { i, orgFrag.startTick, endTick, orgFrag.elem };
									firstBucket.frags.insert(firstBucket.frags.begin(), frag);
									firstBucket.ticks += richTicks;
								}

								t = (float)firstBucket.ticks / needBucketTicks;
								if (t >= 0.98)
									firstBucket.ticks = needBucketTicks;
							}
							else {
								break;
							}
						}
					}


					//比较这两个Bucket，然后选择最优的一个使用,另一个隐藏
					firstBucket = leftBuckets.front();
					lastBucket = rightBuckets.back();

					//
					float rightVal = 1;
					if (lastBucket.ticks < needBucketTicks) {
						rightVal = 0;
					}
					else {
						ElemFrag& headFrag = lastBucket.frags[0];
						ElemFrag& tailFrag = lastBucket.frags[lastBucket.frags.size() - 1];
						ElemFrag& headElem = frags[headFrag.idx];
						ElemFrag& tailElem = frags[tailFrag.idx];

						//所有音符都被包裹在frag内
						if (headElem.startTick >= headFrag.startTick && tailElem.endTick <= tailFrag.endTick)
						{
							rightVal = 1;
						}
						//只有一个音符完全包裹frag
						else if (lastBucket.frags.size() == 1) {
							rightVal = 0.5;
						}
						//两头音符都不在frag范围内
						else if ((headElem.startTick < headFrag.startTick && headElem.endTick < tailFrag.endTick) &&
							(tailElem.endTick > tailFrag.endTick && tailElem.startTick > headFrag.startTick)) {
							rightVal = 0.1;
						}
						//两头音符有一个不在frag范围内
						else if ((headElem.startTick < headFrag.startTick && headElem.endTick < tailFrag.endTick) ||
							(tailElem.endTick > tailFrag.endTick && tailElem.startTick > headFrag.startTick)) {
							rightVal = 0.2;
						}
						
					}

					lastBucket.val = rightVal;


					//
					float leftVal = 1;	
					if (firstBucket.ticks < needBucketTicks)
					{
						leftVal = 0;
					}
					else {
						ElemFrag& headFrag = firstBucket.frags[0];
						ElemFrag& tailFrag = firstBucket.frags[firstBucket.frags.size() - 1];
						ElemFrag& headElem = frags[headFrag.idx];
						ElemFrag& tailElem = frags[tailFrag.idx];

						//所有音符都被包裹在frag内
						if (headElem.startTick >= headFrag.startTick && tailElem.endTick <= tailFrag.endTick)
						{
							leftVal = 1;
						}
						//只有一个音符完全包裹frag
						else if (firstBucket.frags.size() == 1) {
							leftVal = 0.5;
						}
						//两头音符都不在frag范围内
						else if ((headElem.startTick < headFrag.startTick && headElem.endTick < tailFrag.endTick) &&
							(tailElem.endTick > tailFrag.endTick && tailElem.startTick > headFrag.startTick)) {
							leftVal = 0.1;
						}
						//两头音符有一个不在frag范围内
						else if ((headElem.startTick < headFrag.startTick && headElem.endTick < tailFrag.endTick) ||
							(tailElem.endTick > tailFrag.endTick && tailElem.startTick > headFrag.startTick)) {
							leftVal = 0.2;
						}		
					}

					firstBucket.val = leftVal;

					//
					bool isTestTuplet = false;
					if (lastBucket.val == 0 &&
						firstBucket.val == 0)
					{
						if (!isStrictMode)
							isTestTuplet = true;
						else
							break;
					}

					//
					if (leftVal <= rightVal)
						firstBucket.isEnable = false;
					else
						lastBucket.isEnable = false;

					//
					float decVal = 0;
					float totalVal = 0;
					int nTuplet = 0;
					uint32_t totalTicks = 0;
					for (int i = 0; i < leftBuckets.size(); i++) {
						ElemBucket& eBucket = leftBuckets[i];
						if (eBucket.isEnable && eBucket.ticks == needBucketTicks) {

							for (int j = 0; j < eBucket.frags.size(); j++) {
								if (eBucket.frags[j].elem->Type() == REST && eBucket.val < 1)
									decVal += 0.1f;
							}

							totalTicks += eBucket.ticks;
							totalVal += eBucket.val;
							nTuplet++;
						}
					}

					for (int i = 1; i < rightBuckets.size(); i++) {
						ElemBucket& eBucket = rightBuckets[i];
						if (eBucket.isEnable && eBucket.ticks == needBucketTicks) {

							for (int j = 0; j < eBucket.frags.size(); j++) {
								if (eBucket.frags[j].elem->Type() == REST && eBucket.val < 1)
									decVal += 0.1f;
							}

							totalTicks += eBucket.ticks;
							totalVal += eBucket.val;
							nTuplet++;
						}
					}


	


					totalVal /= nTuplet;
					totalVal -= decVal;

					if (nTuplet > 1 && totalVal >= 0.8f)
					{
						info = GetNoteValueInfo(totalTicks, false);
						order = GetNoteTypeOrderIdx(noteTypes[info.noteTimeIdx]);
						 //1.只有在主动测试连音或者
						 //2.在严格模式下连音总时值必须非常吻合标准音符时值，才算是真正的连音
						if (isTestTuplet ||
							(isStrictMode && info.t >= 0.99f && order < 16))
						{
							vector<ElemBucket> buckets;
							for (int i = 0; i < leftBuckets.size(); i++) {
								bucket = leftBuckets[i];
								if (bucket.isEnable && bucket.ticks == needBucketTicks)
									buckets.push_back(bucket);
							}
							for (int i = 1; i < rightBuckets.size(); i++) {
								bucket = rightBuckets[i];
								if (bucket.isEnable && bucket.ticks == needBucketTicks)
									buckets.push_back(bucket);
							}

							//
							tupletBucketsGroup.push_back(buckets);
							nCount = nTotalCount;
							ElemFrag& frag = buckets.back().frags.back();
							if (frag.endTick == frags[frag.idx].endTick) {
								offElemIdx = frag.idx;
							}
							else {
								frags[frag.idx].startTick = frag.endTick + 1;
								offElemIdx = frag.idx - 1;
							}

							break;
						}
					}

					//
					if (lastBucket.val == 0 &&
						firstBucket.val == 0)
							break;
			

					//往右增加一个新的bucket
					if (lastBucket.isEnable)
					{
						int i;
						ElemFrag& lastFrag = lastBucket.frags.back();
						ElemFrag & orgFrag = frags[lastFrag.idx];
						if (lastFrag.endTick == orgFrag.endTick) {
							i = lastFrag.idx + 1;
							startTick = orgFrag.endTick + 1;
						}
						else {
							i = lastFrag.idx;
							startTick = lastFrag.endTick + 1;
						}

						ElemBucket bucket;
						bucket.startFrag = i;
						bucket.bucketStartTick = startTick;
						rightBuckets.push_back(bucket);
					}
					//往左增加一个新的bucket
					else {
						int i;
						ElemFrag& firstFrag = firstBucket.frags.front();
						ElemFrag& orgFrag = frags[firstFrag.idx];
						if (firstFrag.startTick == orgFrag.startTick) {
							i = firstFrag.idx - 1;
							endTick = orgFrag.startTick - 1;
						}
						else {
							i = firstFrag.idx;
							endTick = firstFrag.startTick - 1;
						}

						ElemBucket bucket;
						bucket.startFrag = i;
						bucket.bucketStartTick = endTick;
						leftBuckets.insert(leftBuckets.begin(), bucket);
					}
				}
			}

			offElemIdx++;
		}

		return tupletBucketsGroup;

	}

	//对当前小节中每个音符时值格式化到标准几分音符时值
	//将按小节中以四分音符为一拍的范围进行格式化
	void MeiExporter::FormatMeasureNoteToStdType(vector<VirElem*>& elems, int32_t measureStartTick)
	{
		VirElem* elem = nullptr;
		int noteTimeIdx;
		int nextIdx = 0;
		NoteValueInfo info;
		int beatStartTick = measureStartTick; //当前拍的起始tick
		int beatEndTick = beatStartTick + qNoteTicks - 1;  //当前拍的结束tick
		//当前小节中当前拍的起始和结束位置
		int beatStartElemIdx = 0;
		int beatEndElemIdx = 0;
		int noteTicks;

		//按照拍线获取每拍音符分组编号范围
		vector<int32_t> beatRanges;
		beatRanges.push_back(0);
		int32_t tickCount = 0;
		for (int i = 0; i < elems.size(); i++) {
			elem = elems[i];
			tickCount += elem->endTick - elem->startTick + 1;
			if (tickCount % qNoteTicks == 0) {
				beatRanges.push_back(i);
				tickCount = 0;
				if(i+1 < elems.size())
					beatRanges.push_back(i+1);
			}
		}

		int startTick = measureStartTick;
		int oldDiffTicks = 0;
		bool havDiffTicks = false;
		int testCount = 0;
		for (int i = 0; i < beatRanges.size(); i += 2)
		{
			beatStartElemIdx = beatRanges[i];
			beatEndElemIdx = beatRanges[i + 1];
			beatStartTick = elems[beatStartElemIdx]->startTick;
			beatEndTick = elems[beatEndElemIdx]->endTick;

			testCount = 0;
			//获取到当前拍的起始结束元素idx后，在这个拍的范围内格式化音符时值到标准音符时值
			//格式化一拍范围中的音符时值到标准几分音符时值
			while (true)
			{
				testCount++;
				//
				startTick = beatStartTick;
				for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
				{
					elem = elems[i];
					noteTicks = elem->endTick - elem->startTick + 1;
					if (noteTicks <= 0)
						continue;

					noteTimeIdx = GetNoteTimeIdx(noteTicks);

					//
					elem->SetStartTick(startTick);
					noteTicks = noteTimes[noteTimeIdx] * 4 * qNoteTicks;
					elem->SetEndTick(elem->startTick + noteTicks - 1);
					startTick = elem->endTick + 1;
				}

				//在当前拍范围中会处理所有音符时长格式化后的总值不足一拍时长，或者超出一拍的情况
				elem = elems[beatEndElemIdx];
				int diffTicks = elem->endTick - beatEndTick; //结尾音符的endtick与当前节拍结束tick的差值

				//
				if (oldDiffTicks > 0 && diffTicks < 0 || oldDiffTicks < 0 && diffTicks > 0)
					havDiffTicks = false;

				if (diffTicks == 0)
				{

					break;
				}
				//在当前拍范围中所有音符时长格式化后的总值超出一拍
				else if (diffTicks > 0)
				{
					if (!havDiffTicks || diffTicks < oldDiffTicks)
					{
						float curtStartTick = elems[beatStartElemIdx]->startTick;
						float curtEndTick = elems[beatEndElemIdx]->endTick;
						float t = (beatEndTick - beatStartTick + 1) / (curtEndTick - curtStartTick + 1);
						int startTick = beatStartTick;
						for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
						{
							elem = elems[i];
							float ticks = (elem->endTick - elem->startTick + 1) * t;
							int endTick = startTick + ticks;
							elem->SetTickRange(startTick, endTick);
							startTick = endTick + 1;
						}

						oldDiffTicks = diffTicks;
						havDiffTicks = true;
					}
					else {

						while (diffTicks > 0)
						{
							int orgDiffTicks = diffTicks;
							int maxTicksElemIdx = 0;
							int maxTicks = -1;

							if (testCount > 3) {
								for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
								{
									elem = elems[i];
									int ticks = elem->endTick - elem->startTick + 1;
									if (maxTicks < 0 || ticks < maxTicks) {
										maxTicks = ticks;
										maxTicksElemIdx = i;
									}
								}
								testCount = 0;
							}
							else {
								for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
								{
									elem = elems[i];
									int ticks = elem->endTick - elem->startTick + 1;
									if (maxTicks < 0 || ticks >= maxTicks) {
										maxTicks = ticks;
										maxTicksElemIdx = i;
									}
								}
							}

							//在添加元素少的ticks后，还要向后平移后面的元素
							elem = elems[maxTicksElemIdx];
							if (elem->endTick - diffTicks < elem->startTick) {
								diffTicks = elem->endTick - elem->startTick + 1;
								elem->endTick = elem->startTick - 1;
							}
							else {
								elem->endTick -= diffTicks;
							}

							for (int i = maxTicksElemIdx + 1; i <= beatEndElemIdx; i++)
							{
								elem = elems[i];
								elem->startTick -= diffTicks;
								elem->endTick -= diffTicks;
							}

							diffTicks = orgDiffTicks - diffTicks;
							havDiffTicks = false;
						}

					}
				}
				else if (diffTicks < 0)
				{
					if (!havDiffTicks || diffTicks > oldDiffTicks)
					{
						float curtStartTick = elems[beatStartElemIdx]->startTick;
						float curtEndTick = elems[beatEndElemIdx]->endTick;
						float t = (beatEndTick - beatStartTick + 1) / (curtEndTick - curtStartTick + 1);
						int startTick = beatStartTick;
						for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
						{
							elem = elems[i];
							float ticks = (elem->endTick - elem->startTick + 1) * t;
							int endTick = startTick + ticks;
							elem->SetTickRange(startTick, endTick);
							startTick = endTick + 1;
						}

						oldDiffTicks = diffTicks;
						havDiffTicks = true;
					}
					else {

						while (diffTicks < 0)
						{
							int orgDiffTicks = diffTicks;
							int maxTicksElemIdx = 0;
							int maxTicks = -1;

							if (testCount > 3) {
								for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
								{
									elem = elems[i];
									int ticks = elem->endTick - elem->startTick + 1;
									if (maxTicks < 0 || ticks >= maxTicks) {
										maxTicks = ticks;
										maxTicksElemIdx = i;
									}
								}
								testCount = 0;
							}
							else {
								for (int i = beatStartElemIdx; i <= beatEndElemIdx; i++)
								{
									elem = elems[i];
									int ticks = elem->endTick - elem->startTick + 1;
									if (maxTicks < 0 || ticks < maxTicks) {
										maxTicks = ticks;
										maxTicksElemIdx = i;
									}
								}
							}

							//在添加元素少的ticks后，还要向后平移后面的元素
							elem = elems[maxTicksElemIdx];
							elem->endTick -= diffTicks;

							for (int i = maxTicksElemIdx + 1; i <= beatEndElemIdx; i++)
							{
								elem = elems[i];
								elem->startTick -= diffTicks;
								elem->endTick -= diffTicks;
							}

							diffTicks = orgDiffTicks - diffTicks;
							havDiffTicks = false;
						}
					}
				}
			}
		}
	}


	//为元素生成片段
	vector<ElemFrag> MeiExporter::CreateFrags(vector<VirElem*>& elems) 
	{
		vector<ElemFrag> frags;
		VirElem* elem;
		for (int i = 0; i < elems.size(); i++) {
			elem = elems[i];
			ElemFrag frag = {i, elem->startTick, elem->endTick, elem};
			frags.push_back(frag);
		}

		return frags;
	}

	//获取音符类型的顺序编号
	int32_t MeiExporter::GetNoteTypeOrderIdx(int32_t noteType)
	{
		for (int i = 0; i < noteTypeOrderLen; i++) {
			if (noteTypeOrders[i] == noteType)
				return i;
		}
		return 2;
	}

	//获取音符时值对应的编号
	int MeiExporter::GetNoteTimeIdx(int noteTicks)
	{
		NoteValueInfo info = GetNoteValueInfo(noteTicks, true);
		return info.noteTimeIdx;
	}

	//获取音符价值信息
	NoteValueInfo MeiExporter::GetNoteValueInfo(int noteTicks, bool isEnableMinNote)
	{
		//1分音符为一拍音符价值
		static double _1BeatVal[] = {
			1, 0.9, 0.8, 0.6, 0.3, 0.1, 0.05, 0.02,0.01,
			0.5, 0.4, 0.3, 0.2, 0.1, 0.05, 0.02, 0.01,
			0.1, 0.08, 0.06, 0.05, 0.05, 0.01, 0.01, 0.001,
		};


		//2分音符为一拍音符价值
		static double _2BeatVal[] = {
			0.9, 1, 0.9, 0.8, 0.6, 0.4, 0.2, 0.02,0.01,
			0.4, 0.5, 0.4, 0.3, 0.2, 0.1, 0.02, 0.01,
			0.1, 0.1, 0.1, 0.05, 0.05, 0.01, 0.01, 0.001,
		};


		//4分音符为一拍音符价值
		static double _4BeatVal[] = {
			0.8, 0.9, 1, 0.9, 0.8, 0.7, 0.2, 0.1, 0.01,
			0.3, 0.5, 0.5, 0.3, 0.2, 0.1, 0.02, 0.01,
			0.1, 0.1, 0.1, 0.05, 0.05, 0.01, 0.01, 0.001,
		};

		//8分音符为一拍音符价值
		static double _8BeatVal[] = {
			0.4, 0.8, 0.9, 1, 0.9, 0.8, 0.7, 0.6, 0.02,
			0.2, 0.3, 0.4, 0.5, 0.4, 0.3, 0.03, 0.02,
			0.01, 0.02, 0.05, 0.1, 0.05, 0.03, 0.02, 0.001,
		};

		//16分音符为一拍音符价值
		static double _16BeatVal[] = {
			0.3, 0.4, 0.8, 0.9, 1, 0.9, 0.8, 0.3, 0.1,
			0.1, 0.2, 0.3, 0.4, 0.5, 0.4, 0.2, 0.1,
			0.01, 0.02, 0.04, 0.05, 0.1, 0.05, 0.04, 0.005,
		};


		//32分音符为一拍音符价值
		static double _32BeatVal[] = {
			0.2, 0.3, 0.4, 0.8, 0.9, 1, 0.9, 0.8, 0.1,
			0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.3, 0.1,
			0.001, 0.01, 0.02, 0.05, 0.05, 0.1, 0.05, 0.01,
		};

		//64分音符为一拍音符价值
		static double _64BeatVal[] = {
			0.05, 0.1, 0.3, 0.4, 0.8, 0.9, 1, 0.9, 0.8,
			0.01, 0.05, 0.1, 0.1, 0.2, 0.4, 0.5, 0.4,
			0.001, 0.01, 0.02, 0.02, 0.03, 0.05, 0.1, 0.05,
		};

		//128分音符为一拍音符价值
		static double _128BeatVal[] = {
			0.01, 0.05, 0.1, 0.2, 0.4, 0.8, 0.9, 1, 0.9,
			0.01, 0.05, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5,
			0.001, 0.01, 0.01, 0.02, 0.03, 0.03, 0.05, 0.1,
		};



		double* beatVal = _4BeatVal;
		switch (perBeatBaseNote) {
		case 2: beatVal = _2BeatVal; break;
		case 4: beatVal = _4BeatVal; break;
		case 8: beatVal = _8BeatVal; break;
		case 16: beatVal = _16BeatVal; break;
		case 32: beatVal = _32BeatVal; break;
		case 64: beatVal = _64BeatVal; break;
		case 128: beatVal = _128BeatVal; break;
		}


		if (isEnableMinNote) {
			double baseNoteTicks = 4.0 / perBeatBaseNote * qNoteTicks;
			double t = (double)noteTicks / baseNoteTicks;
			if (t < 0.0625f) {
				noteTicks = baseNoteTicks * 0.0625f;
			}
		}

		//
		int maxIdx = 2;
		int maxOrder = -100;
		double curtVal, maxVal = -1;
		double alpha = 0.7f;
		for (int i = 0; i < noteTimeLen; i++) {
			int n = noteTimes[i] * 4 * qNoteTicks;
			double t = abs((double)(noteTicks - n)) / n;
			if (t > 1) continue;
			if(t<0.1) //y = 0.1(10𝑥)^3 
				t = 0.1 * pow(10 * t, 3); 
			else {  //y = ((x-0.1)/m)^0.5 + 0.1
				t = (t - 0.1) / (0.9 / pow(0.9, 2));    
				t = pow(t, 0.5) + 0.1;
			}

			int order = GetNoteTypeOrderIdx(noteTypes[i]);
			int aOrderDiff = abs(maxOrder - perBeatBaseNoteOrder);
			int bOrderDiff = abs(order - perBeatBaseNoteOrder);
			int orderDiff = abs(aOrderDiff - bOrderDiff);
			if (maxVal > 0.7f && aOrderDiff < bOrderDiff && orderDiff > 9) 
				continue;
			
			if (t <= 0.0001f) {
				NoteValueInfo info = {1-t, i};
				return info;
			}

			//时值占比70%，音符估值占30%
			curtVal = (1 - t) * alpha + (1 - alpha) * beatVal[i];
			if (curtVal > maxVal) {
				maxVal = curtVal;
				maxIdx = i;
				maxOrder = order;
			}
		}

		NoteValueInfo info = { maxVal, maxIdx };
		return info;
	}


	//复制元素
	VirElem* MeiExporter::CopyElem(VirElem* orgElem) 
	{
		switch (orgElem->Type())
		{
		case ALONE_NOTE:
			return new AloneNote(*(AloneNote*)orgElem);
		case REST:
			return new Rest(*(Rest*)orgElem);
		case CHORD:
			return new Chord(*(Chord*)orgElem);
			break;

		default:
			return nullptr;
		}

	}
}