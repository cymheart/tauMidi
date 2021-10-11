#include"Track.h"
#include"Synth/Channel.h"
#include"MidiEditor.h"


namespace tau
{
	Track::Track(MidiEditor* midiEditor)
	{
		this->midiEditor = midiEditor;
		channel = new Channel(0);

		instFragments.push_back(new list<InstFragment*>);
	}

	Track::~Track()
	{
		DEL(channel);
	}

	void Track::Clear()
	{
		isEnded = false;
		endSec = 0;

		//
		channel->Clear();


		for (int i = 0; i < instFragments.size(); i++)
		{
			list<InstFragment*>::iterator frag_it = instFragments[i]->begin();
			list<InstFragment*>::iterator frag_end = instFragments[i]->end();
			for (; frag_it != frag_end; frag_it++)
			{
				(*frag_it)->Clear();
			}
		}
	}

	//新建分径
	void Track::NewBranch()
	{
		instFragments.push_back(new list<InstFragment*>);
	}

	int Track::GetBranchCount()
	{
		return instFragments.size();
	}

	void Track::SetChannelNum(int channelNum)
	{
		channel->SetChannelNum(channelNum);
	}

	int Track::GetChannelNum()
	{
		return channel->GetChannelNum();
	}


	void Track::AddInstFragment(InstFragment* instFragment, int branchIdx)
	{
		branchIdx = Clamp(branchIdx, 0, instFragments.size() - 1);

		instFragment->SetTrack(this, branchIdx);
		instFragments[branchIdx]->push_back(instFragment);
	}

	void Track::RemoveInstFragment(InstFragment* instFragment)
	{
		instFragments[instFragment->GetBranchIdx()]->remove(instFragment);
		instFragment->SetTrack(nullptr);
	}


}
