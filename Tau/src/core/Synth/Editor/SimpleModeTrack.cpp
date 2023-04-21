#include"Editor.h"
#include"MidiEditor.h"
#include"Track.h"
#include"Midi/MidiEvent.h"

namespace tau
{
	//生成简单模式音符轨道
	void MidiEditor::CreateSimpleModeTrack()
	{
		simpleModeTrackNotes.Release();
		simpleModeTrackNotesOffset = 0;

		//
		int noteCounts[128] = { 0 };
		for (int i = 0; i < 128; i++) {
			classifyNoteOnEvents[i].clear();
		}

		//获取需要弹奏的noteEv并按note值分类到classifyNoteOnEvents中
		int minTrackIdx = -1;
		Track* track;
		for (int i = 0; i < tracks.size(); i++)
		{
			track = tracks[i];
			MidiEvent* ev;
			NoteOnEvent* noteOnEv;

			LinkedList<MidiEvent*>* eventList;
			InstFragment* instFrag;
			auto& instFragmentBranchs = track->instFragmentBranchs;
			for (int j = 0; j < instFragmentBranchs.size(); j++)
			{
				list<InstFragment*>::iterator frag_it = instFragmentBranchs[j]->begin();
				list<InstFragment*>::iterator frag_end = instFragmentBranchs[j]->end();
				for (; frag_it != frag_end; frag_it++)
				{
					instFrag = *frag_it;
					eventList = &(instFrag->midiEvents);
					LinkedListNode<MidiEvent*>* next = nullptr;
					LinkedListNode<MidiEvent*>* node = eventList->GetHeadNode();
					for (; node; node = next)
					{
						if (node->elem->type != MidiEventType::NoteOn &&
							node->elem->type != MidiEventType::NoteOff)
						{
							next = node->next;
							continue;
						}

						ev = node->elem;
						if (IsPointerPlayNote(ev, track))
						{
							if (ev->type == MidiEventType::NoteOff)
							{
								next = eventList->Remove(node);
								DEL(node);
							}
							else
							{
								noteOnEv = (NoteOnEvent*)ev;
								classifyNoteOnEvents[noteOnEv->note].push_back(noteOnEv);
								noteCounts[noteOnEv->note]++;

								if (minTrackIdx == -1 || i < minTrackIdx)
									minTrackIdx = i;

								next = eventList->Remove(node);
								DEL(node);
							}
						}
						else {
							next = node->next;
						}
					}
				}
			}
		}

		//
		simpleModeTrack = tracks[minTrackIdx];


		//计算出需要弹奏的key范围startKeyIdx 到 endKeyIdx
		//按范围中note的数量最多，来决定key的范围
		int whiteKeyCount;
		int noteCount = 0;
		int maxNoteCount = 0;
		int startKeyIdx = 0;
		int endKeyIdx = 0;
		int a = A0, b = 1;

		//在仅有7个键的时候按键值的八度进行范围匹配
		if (simpleModePlayWhiteKeyCount == 7) {
			a = C1; b = 12;
		}

		int end;
		for (int i = a; i <= C8 - simpleModePlayWhiteKeyCount; i += b)
		{
			end = i;
			whiteKeyCount = 0;
			noteCount = 0;
			for (int j = i; j <= C8; j++)
			{
				if (GetNoteType(j) == KeyWhite)
					whiteKeyCount++;

				noteCount += noteCounts[j];
				if (whiteKeyCount >= simpleModePlayWhiteKeyCount) {
					end = j;
					break;
				}
			}

			if (noteCount > maxNoteCount) {
				maxNoteCount = noteCount;
				startKeyIdx = i;
				endKeyIdx = end;
			}
		}

		//把startKeyIdx 到endKeyIdx 按键范围外的noteEv 以到到范围中
		int blackNoteIdx = startKeyIdx;
		int whiteNoteIdx = startKeyIdx - 1;
		int gotoClassifyNoteIdx;
		for (int i = A0; i <= C8; i++)
		{
			if (i >= startKeyIdx && i <= endKeyIdx)
				continue;

			if (simpleModePlayWhiteKeyCount <= 2)
			{
				whiteNoteIdx = GetNextWhiteNote(whiteNoteIdx);
				if (whiteNoteIdx == -1 || whiteNoteIdx > endKeyIdx) {
					whiteNoteIdx = startKeyIdx - 1;
					whiteNoteIdx = GetNextWhiteNote(whiteNoteIdx);
				}
				gotoClassifyNoteIdx = whiteNoteIdx;
			}
			else if (simpleModePlayWhiteKeyCount != 7)
			{
				if (GetNoteType(i) == KeyBlack) {
					blackNoteIdx = GetNextBlackNote(blackNoteIdx);
					if (blackNoteIdx == -1 || blackNoteIdx > endKeyIdx) {
						blackNoteIdx = startKeyIdx;
						blackNoteIdx = GetNextBlackNote(blackNoteIdx);
					}
					gotoClassifyNoteIdx = blackNoteIdx;
				}
				else {
					whiteNoteIdx = GetNextWhiteNote(whiteNoteIdx);
					if (whiteNoteIdx == -1 || whiteNoteIdx > endKeyIdx) {
						whiteNoteIdx = startKeyIdx - 1;
						whiteNoteIdx = GetNextWhiteNote(whiteNoteIdx);
					}
					gotoClassifyNoteIdx = whiteNoteIdx;
				}
			}
			else {
				gotoClassifyNoteIdx = startKeyIdx + i % 12;
			}

			vector<NoteOnEvent*>& gotoClassNoteOnEvents = classifyNoteOnEvents[gotoClassifyNoteIdx];
			vector<NoteOnEvent*>& noteOnEvs = classifyNoteOnEvents[i];
			for (int j = 0; j < noteOnEvs.size(); j++)
				gotoClassNoteOnEvents.push_back(noteOnEvs[j]);
			noteOnEvs.clear();
		}


		//组合时间点连在一起的noteEv为一个noteEv,并把其放入子项中
		vector<NoteOnEvent*> childNoteOnEvents;
		NoteOnEvent* newNoteEv;
		NoteOffEvent* newNoteOffEv;
		NoteOnEvent* ev;
		for (int i = A0; i <= C8; i++)
		{
			if (i < startKeyIdx || i > endKeyIdx)
				continue;

			childNoteOnEvents.clear();
			newNoteEv = nullptr;
			newNoteOffEv = nullptr;
			vector<NoteOnEvent*>& noteOnEvents = classifyNoteOnEvents[i];
			if (!noteOnEvents.empty()) {
				//按时间排序需要弹奏的Notes
				sort(noteOnEvents.begin(), noteOnEvents.end(), NoteCmp);
				newNoteEv = new NoteOnEvent();
				newNoteOffEv = new NoteOffEvent();
				newNoteEv->noteOffEvent = newNoteOffEv;
				newNoteOffEv->noteOnEvent = newNoteEv;

				newNoteEv->track = minTrackIdx;
				newNoteOffEv->track = minTrackIdx;
				CopyChildNoteOnEv(newNoteEv, newNoteOffEv, noteOnEvents[0], i);
				childNoteOnEvents.push_back(noteOnEvents[0]);
			}

			for (int j = 1; j < noteOnEvents.size(); j++)
			{
				ev = noteOnEvents[j];

				if (ev->startSec <= newNoteEv->endSec)
				{
					if (ev->endTick > newNoteEv->endTick) {
						newNoteOffEv->startTick = newNoteEv->endTick = ev->endTick;
						newNoteOffEv->endSec = newNoteOffEv->startSec = newNoteEv->endSec = ev->endSec;
					}

					childNoteOnEvents.push_back(ev);
				}
				else {
					newNoteEv->AppendChildNoteOnEvents(childNoteOnEvents);
					childNoteOnEvents.clear();
					simpleModeTrackNotes.AddLast(newNoteEv);
					simpleModeTrackNotes.AddLast(newNoteOffEv);

					//
					newNoteEv = new NoteOnEvent();
					newNoteOffEv = new NoteOffEvent();
					newNoteEv->noteOffEvent = newNoteOffEv;
					newNoteOffEv->noteOnEvent = newNoteEv;

					newNoteEv->track = minTrackIdx;
					newNoteOffEv->track = minTrackIdx;

					CopyChildNoteOnEv(newNoteEv, newNoteOffEv, ev, i);
					childNoteOnEvents.push_back(ev);
				}
			}

			if (newNoteEv != nullptr) {
				newNoteEv->AppendChildNoteOnEvents(childNoteOnEvents);
				childNoteOnEvents.clear();
				simpleModeTrackNotes.AddLast(newNoteEv);
				simpleModeTrackNotes.AddLast(newNoteOffEv);
			}
		}

		//对simpleModeNoteTrack中的note按时间顺序，由小到大排序
		simpleModeTrackNotes.Sort(NoteCmp);


		//按时长大小简化合并音符
		if (mergeSimpleSrcNoteLimitSec <= 0 || mergeSimpleDestNoteLimitSec <= 0) {
			simpleModeTrackNotesOffset = simpleModeTrackNotes.GetHeadNode();
			return;
		}


		NoteOnEvent* curt = nullptr, * prev = nullptr;
		MidiEvent* prevOff = nullptr;
		float prevCostSec = 0;
		LinkedListNode<MidiEvent*>* node = simpleModeTrackNotes.GetHeadNode();
		LinkedListNode<MidiEvent*>* next = nullptr, * next2;
		for (; node; node = next)
		{
			if (node->elem->type == MidiEventType::NoteOff) {
				next = node->next;
				continue;
			}

			curt = (NoteOnEvent*)node->elem;

			if (curt->endSec - curt->startSec <= mergeSimpleSrcNoteLimitSec)
			{
				if (prev != nullptr && prevCostSec <= mergeSimpleDestNoteLimitSec && curt->startSec - prev->endSec <= 0.4f)
				{
					prevOff->startTick = prev->endTick = curt->endTick;
					prevOff->endSec = prevOff->startSec = prev->endSec = curt->endSec;
					prev->AppendChildNoteOnEvents(curt->childNoteOnEvents, curt->childNoteOnEventCount);
					prevCostSec = prev->endSec - prev->startSec;

					next = FindFitNoteOffEventNode(node);
					next2 = simpleModeTrackNotes.Remove(node);
					DEL(node);

					if (next == next2)
					{
						next2 = next->next;
						node = simpleModeTrackNotes.Remove(next);
						DEL(next);
					}
					else {
						simpleModeTrackNotes.Remove(next);
						DEL(next);
					}

					next = next2;
				}
				else {
					prev = curt;
					prevOff = curt->noteOffEvent;
					prevCostSec = prev->endSec - prev->startSec;
					next = node->next;
				}
			}
			else {
				prev = curt;
				prevOff = curt->noteOffEvent;
				prevCostSec = prev->endSec - prev->startSec;
				next = node->next;
			}
		}

		//
		simpleModeTrackNotesOffset = simpleModeTrackNotes.GetHeadNode();
	}

	LinkedListNode<MidiEvent*>* MidiEditor::FindFitNoteOffEventNode(
		LinkedListNode<MidiEvent*>* startNode)
	{
		LinkedListNode<MidiEvent*>* node;
		for (node = startNode->next; node; node = node->next)
		{
			if (node->elem->type == MidiEventType::NoteOff)
			{
				NoteOffEvent* noteOffEv = (NoteOffEvent*)node->elem;
				if (noteOffEv->noteOnEvent == startNode->elem)
					return node;
			}

		}

		return nullptr;
	}


	void MidiEditor::CopyChildNoteOnEv(
		NoteOnEvent* newNoteEv, NoteOffEvent* newNoteOffEv,
		NoteOnEvent* ev, int note)
	{
		newNoteEv->startTick = ev->startTick;
		newNoteOffEv->startTick = newNoteEv->endTick = ev->endTick;
		newNoteEv->startSec = ev->startSec;
		newNoteOffEv->endSec = newNoteOffEv->startSec = newNoteEv->endSec = ev->endSec;
		newNoteOffEv->note = newNoteEv->note = note;
		newNoteOffEv->playType = newNoteEv->playType = ev->playType;
	}

}