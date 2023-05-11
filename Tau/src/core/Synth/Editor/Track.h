#ifndef _Track_h_
#define _Track_h_

#include "Synth/TauTypes.h"
#include "InstFragment.h"
#include"Midi/MidiTypes.h"

namespace tau
{

	// 演奏音轨
	//by cymheart, 2020--2021.
	class Track
	{
	public:
		Track(MidiEditor* midiEditor);
		~Track();

		inline MidiEditor* GetMidiEditor()
		{
			return midiEditor;
		}

		inline Channel* GetChannel()
		{
			return channel;
		}

		inline double GetEndSec()
		{
			return endSec;
		}

		inline MidiEventPlayType GetPlayType()
		{
			return playType;
		}

		void SetChannelNum(int channelNum);
		int GetChannelNum();

		void SetName(string& _name);

		void Clear();

		//新建分径
		void NewBranch();
		int GetBranchCount();

		void AddInstFragment(InstFragment* instFragment, int branchIdx = 0);
		void RemoveInstFragment(InstFragment* instFragment);

		const vector<list<InstFragment*>*>& GetInstFragmentBranchs()
		{
			return instFragmentBranchs;
		}

		InstFragment* GetInstFragment(int branchIdx, int instFragIdx);

	protected:

		MidiEditor* midiEditor = nullptr;

		//弹奏方式
		MidiEventPlayType playType = MidiEventPlayType::Background;

		//是否禁止播放
		bool isDisablePlay = false;

		// 通道
		Channel* channel = nullptr;

		vector<list<InstFragment*>*> instFragmentBranchs;

		//结束时间点
		double endSec = 0;

		//结束tick点
		int32_t endTick = 0;


	private:

		string name;


		//重新处理当前时间点在事件处理时间中间时，可以重新启用此时间
		vector<MidiEvent*> reProcessMidiEvents;
		vector<InstFragment*> _updateInstFrags;
		bool _isUpdatePlayPos = false;
		bool _isUpdatePlayPrevPos = false;

		friend class MidiEditor;
		friend class MeiExporter;
	};


}

#endif
