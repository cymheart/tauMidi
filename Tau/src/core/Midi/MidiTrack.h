#ifndef _MidiTrack_h_
#define _MidiTrack_h_

#include"MidiTypes.h"
#include"MidiEvent.h"

namespace tau
{
	struct RecordTempo;

	class MidiTrack
	{
	public:
		MidiTrack();

		~MidiTrack();

		//清空midiTrack列表,但并不真正删除列表中的事件
		void Clear();

		void CreateTempoEvents(float tickForQuarterNote, vector<RecordTempo>& tempos);
		void CreateMidiEvents(const MidiTrack& orgMidiTrack, float tickForQuarterNote, vector<RecordTempo>& tempos);
		void SetMidiEventsChannel(int channel);

		//设置一个四分音符所要弹奏的tick数
		inline void SetTickForQuarterNote(float tickForQuarterNote)
		{
			this->tickForQuarterNote = tickForQuarterNote;
		}

		//添加midi事件
		void AppendMidiEvents(LinkedList<MidiEvent*>& midiEvents);

		//寻找默认乐器改变事件
		void FindDefaultProgramChangeEvent();

		/// <summary>
		/// 增加一个事件
		/// </summary>
		/// <param name="ev"></param>
		LinkedListNode<MidiEvent*>* AddEvent(MidiEvent* ev);

		/// <summary>
		/// 移除一个事件
		/// </summary>
		/// <param name="ev"></param>
		void RemoveEvent(LinkedListNode<MidiEvent*>* midiEventNode);

		/// <summary>
		/// 是否需要按键事件
		/// </summary>
		bool IsNeedNoteOnEvents(int channel, uint32_t startTick, int keepCount);

		/// <summary>
		/// 移除相同StartTick的midi事件
		/// </summary>
		/// <param name="ev"></param>
		void RemoveSameStartTickEvents(LinkedListNode<MidiEvent*>* midiEventNode, int keepCount);


		/// <summary>
		/// 获取事件列表
		/// </summary>
		/// <returns></returns>
		inline LinkedList<MidiEvent*>* GetEventList()
		{
			return &midiEventList;
		}

		/// <summary>
		/// 获取全局事件列表
		/// </summary>
		/// <returns></returns>
		inline LinkedList<MidiEvent*>* GetGolbalEventList()
		{
			return &midiGolbalEventList;
		}

		/// <summary>
		/// 获取通道分类的事件列表
		/// </summary>
		/// <returns></returns>
		inline LinkedList<MidiEvent*>* GetEventListAtChannel()
		{
			return midiEventListAtChannel;
		}


		/// <summary>
		/// 获取事件数量
		/// </summary>
		/// <returns></returns>
		inline int GetEventCount()
		{
			return (int)midiEventList.Size();
		}


		inline void SetChannelNum(int num)
		{
			channelNum = num;
		}

		inline int GetChannelNum()
		{
			return channelNum;
		}

		inline ProgramChangeEvent* GetDefaultProgramChangeEvent()
		{
			return defaultProgramChangeEvent;
		}

		/// <summary>
		/// 寻找匹配的NoteOnEvent
		/// </summary>
		/// <param name="note"></param>
		/// <param name="channel"></param>
		/// <returns></returns>
		LinkedListNode<MidiEvent*>* FindNoteOnEvent(int note, int channel);

	private:
		//获取下一个TempoIndex
		int GetNextTempoIndex(vector<RecordTempo>& tempos, int curtIndex);

	private:
		int channelNum = -1;
		LinkedList<MidiEvent*> midiEventList;
		LinkedList<MidiEvent*> midiGolbalEventList;
		LinkedList<MidiEvent*> midiEventListAtChannel[16];

		ProgramChangeEvent* defaultProgramChangeEvent = nullptr;


		//一个四分音符所要弹奏的tick数,默认值120
		float tickForQuarterNote = 120;

		unordered_map<int, vector<LinkedListNode<MidiEvent*>*>> noteOnEventMap;

	};

}

#endif
