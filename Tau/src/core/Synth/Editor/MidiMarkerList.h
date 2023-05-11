#ifndef _MidiMarkerList_h_
#define _MidiMarkerList_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"
#include "MidiMarker.h"

namespace tau
{
	/// <summary>
	/// Midi标记列表类
	/// by cymheart, 2021.
	/// </summary> 
	class MidiMarkerList
	{
	public:
		MidiMarkerList();
		~MidiMarkerList();

		void Clear();

		void Copy(MidiMarkerList& cpyMidiMarkerList);

		vector<MidiMarker*>& GetMidiMarkers()
		{
			return midiMarkers;
		}

		// 设置一个四分音符的tick数
		void SetTickForQuarterNote(uint32_t tick)
		{
			tickForQuarterNote = tick;
			defaultTempo.SetTempoByBPM(120, tickForQuarterNote, 0, 0);
		}

		// 获取一个四分音符的tick数
		int GetTickForQuarterNote() {
			return tickForQuarterNote;
		}

		//获取当前速度
		Tempo* GetTempo(double sec);
		//获取当前速度
		Tempo* GetTempo(int tick);

		//
		void AppendMidiMarker(MidiMarker* midiMarker);
		void AppendFromMidiEvents(LinkedList<MidiEvent*>& midiEvents);

	private:

		//获取当前速度
		//tick >= 0 时使用tick
		//sec >= 0 时使用sec判断
		Tempo* GetTempo(int tick, double sec, int startIdx = 0);

		//计算速度和起始时间
		void ComputeTempoAndStartSec();
		static bool MidiMarkerTickCompare(MidiMarker* a, MidiMarker* b);

	private:
		vector<MidiMarker*> midiMarkers;
		int lastGetTempoIdx = 0;

		// 一个四分音符的tick数
		uint32_t tickForQuarterNote;

		//默认速度
		Tempo defaultTempo;

	};
}

#endif
