#include"MeasureInfo.h"

namespace tau
{
	//生成小节信息
	void MeasureInfo::Create(MidiMarkerList& midiMarkerList, int midiEndTick)
	{
		measureNum = 0;
		int mIdx = -1;
		double curtSec = 0;
		int curtTick = 0;

		//一个四分音符的秒数
		double microTempo = 0;
		// 拍号分子(以den分音符为一拍，每小节有num拍)
		int num = -1;
		// 拍号分母(以den分音符为一拍)
		int den = -1;

		float beatCostSec; //每拍花费多少秒
		int beatCostTicks = 0; //每拍花费多少tick
		float measureCostSec; //每节花费多少秒
		int measureCostTicks = 0; //每节花费多少tick
		MidiMarker* midiMarker;
		vector<MidiMarker*>& midiMarkers = midiMarkerList.GetMidiMarkers();
		int qNoteTicks = midiMarkerList.GetTickForQuarterNote();
		int startTick = 0;
		float startSec = 0;
		vector<MidiMarkerInfo> midiMarkerInfos;
		MidiMarkerInfo info;
		for (int i = 0; i < midiMarkers.size(); i++)
		{
			midiMarker = midiMarkers[i];
			if (midiMarker->IsEnableTempo()) {
				microTempo = midiMarker->GetMicroTempo() * 0.000001f;
				//
				if (!midiMarkerInfos.empty() && 
					midiMarkerInfos[midiMarkerInfos.size() - 1].startTick == midiMarker->GetStartTick())
					midiMarkerInfos[midiMarkerInfos.size() - 1].microTempo = microTempo;
			}

			if (midiMarker->IsEnableTimeSignature())
			{
				startSec = midiMarker->GetStartSec();
				startTick = midiMarker->GetStartTick();
				num = midiMarker->GetNumerator();
				den = pow(2, midiMarker->GetDenominator());
				//
				info.startSec = startSec;
				info.startTick = startTick;
				info.microTempo = microTempo;
				info.num = num;
				info.den = den;
				midiMarkerInfos.push_back(info);
			}	
		}

		//
		info.startSec = 0;
		info.startTick = midiEndTick;
		info.microTempo = 0;
		info.num = 0;
		info.den = 0;
		midiMarkerInfos.push_back(info);

		int mEndTick = 0;

		//生成小节信息
		for (int i = 0; i < midiMarkerInfos.size() - 1; i++) 
		{
			MidiMarkerInfo& mInfoA = midiMarkerInfos[i];
			MidiMarkerInfo& mInfoB = midiMarkerInfos[i+1];

			// 拍号分母(以den分音符为一拍)
			den = mInfoA.den;
			// 拍号分子(以den分音符为一拍，每小节有num拍)
			num = mInfoA.num;

			//根据每4分音符为一拍的时间microTempo， 按比值计算den分音符作为一拍的花费时间
			//例如1/2拍, 即以2分音符为一拍，每小节有4拍，2分音符是4分音符时值的2倍，即4.0f / den倍
			beatCostTicks = 4.0f / den * qNoteTicks;
			measureCostTicks = beatCostTicks * num;
	
			int curtTick = mInfoA.startTick;
			int endTick = mInfoB.startTick;

			while (curtTick < endTick)
			{
				++mIdx;
				measure[mIdx].startSec = 0;
				measure[mIdx].startTick = curtTick;
				measure[mIdx].beatNum = num;
				curtTick += measureCostTicks;

				//
				if (curtTick >= endTick && i + 1 == midiMarkerInfos.size() - 1)
					mEndTick = curtTick - 1;
			}	
		}	

		measureNum = mIdx + 1;
		measure[measureNum].startSec = 0;
		measure[measureNum].startTick = mEndTick + 1;
		measure[measureNum].beatNum = 0;


		//生成拍信息
		int idx = GetNextTempoIdx(midiMarkers, 0);
		MidiMarker* nextTempo = midiMarkers[idx];
		double nextMicroTempo = nextTempo->GetMicroTempo() * 0.000001f;

		//
		int midiMarkerIdx;
		int endTick;
		int nTicks;
		int measureStartTick, measureEndTick;
		int measureTicks;
		int beatTicks;
		int beatStartTick, beatEndTick;
		int beatIdx = 0;
		curtSec = 0;
		curtTick = 0;

		for (int i = 0; i < measureNum; i++) 
		{
			measure[i].beatSecs = beatSecs + beatIdx;
			measure[i].startSec = (float)curtSec;
			measureStartTick = measure[i].startTick;
			measureEndTick = measure[i+1].startTick - 1;
			measureTicks = measureEndTick - measureStartTick + 1;
			beatTicks = measureTicks / measure[i].beatNum;

			for (int j = 0; j < measure[i].beatNum; j++) 
			{
				beatStartTick = measureStartTick + j * beatTicks;
				beatEndTick = beatStartTick + beatTicks - 1;
				//
				while (nextTempo != nullptr && 
					nextTempo->GetStartTick() <= beatEndTick)
				{
					microTempo = nextMicroTempo;
					int nTicks = nextTempo->GetStartTick() - curtTick;
					curtSec += nTicks * microTempo / qNoteTicks;
					curtTick += nTicks;
					//
					idx = GetNextTempoIdx(midiMarkers, idx+1);
					if (idx >= midiMarkers.size()) {
						nextTempo = nullptr;
					}else {
						nextTempo = midiMarkers[idx];
						nextMicroTempo = nextTempo->GetMicroTempo() * 0.000001f;
					}
				}

				nTicks = beatEndTick - curtTick + 1;
				curtSec += nTicks * microTempo / qNoteTicks;
				curtTick = beatEndTick+1;
				beatSecs[beatIdx++] = (float)curtSec;
			}
		}

		measure[measureNum].startSec = (float)curtSec;
		beatCount = beatIdx;
	}


	int MeasureInfo::GetNextTempoIdx(vector<MidiMarker*>& midiMarkers, int idx)
	{
		for (; idx < midiMarkers.size(); idx++) {
			if (midiMarkers[idx]->IsEnableTempo())
				return idx;		
		}
		return (int)midiMarkers.size();
	}

}