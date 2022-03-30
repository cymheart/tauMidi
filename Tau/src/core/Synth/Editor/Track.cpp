#include"Track.h"
#include"Synth/Channel.h"
#include"MidiEditor.h"


namespace tau
{
	Track::Track(MidiEditor* midiEditor)
	{
		this->midiEditor = midiEditor;
		channel = new Channel(0);

		instFragmentBranchs.push_back(new list<InstFragment*>);
	}

	Track::~Track()
	{
		DEL(channel);

		for (int i = 0; i < instFragmentBranchs.size(); i++)
		{
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[i]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[i]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				DEL(*frag_it);
			}
		}

		instFragmentBranchs.clear();
	}

	void Track::Clear()
	{
		reProcessMidiEvents.clear();

		//
		channel->Clear();


		for (int i = 0; i < instFragmentBranchs.size(); i++)
		{
			list<InstFragment*>::iterator frag_it = instFragmentBranchs[i]->begin();
			list<InstFragment*>::iterator frag_end = instFragmentBranchs[i]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				(*frag_it)->Clear();
			}
		}
	}

	//新建分径
	void Track::NewBranch()
	{
		instFragmentBranchs.push_back(new list<InstFragment*>);
	}

	int Track::GetBranchCount()
	{
		return instFragmentBranchs.size();
	}

	void Track::SetChannelNum(int channelNum)
	{
		channel->SetChannelNum(channelNum);
	}

	void Track::SetName(string& _name)
	{
		name = _name;
	}

	int Track::GetChannelNum()
	{
		return channel->GetChannelNum();
	}

	InstFragment* Track::GetInstFragment(int branchIdx, int instFragIdx)
	{
		auto instFragList = instFragmentBranchs[branchIdx];
		int i = 0;
		for (auto it = instFragList->begin(); it != instFragList->end(); it++)
		{
			if (i == instFragIdx)
				return *it;
			i++;
		}

		return nullptr;
	}


	void Track::AddInstFragment(InstFragment* instFragment, int branchIdx)
	{
		branchIdx = Clamp(branchIdx, 0, instFragmentBranchs.size() - 1);

		instFragment->SetTrack(this, branchIdx);
		instFragmentBranchs[branchIdx]->push_back(instFragment);
	}

	void Track::RemoveInstFragment(InstFragment* instFragment)
	{
		instFragmentBranchs[instFragment->GetBranchIdx()]->remove(instFragment);
		instFragment->SetTrack(nullptr);
	}


}
