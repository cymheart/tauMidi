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
		for (int i = 0; i < midiMarkers.size(); i++)
		{
			midiMarker = midiMarkers[i];

			if (!midiMarker->IsEnableTempo() &&
				!midiMarker->IsEnableTimeSignature())
				continue;

			startSec = midiMarker->GetStartSec();
			startTick = midiMarker->GetStartTick();

			for (int j = i; j < midiMarkers.size(); j++) 
			{
				midiMarker = midiMarkers[j];
				if (midiMarker->GetStartTick() != startTick) {
					i = j - 1;
					break;
				}

				if (midiMarker->IsEnableTempo()) {
					microTempo = midiMarker->GetMicroTempo() * 0.000001f;
				}

				if (midiMarker->IsEnableTimeSignature())
				{
					num = midiMarker->GetNumerator();
					den = pow(2, midiMarker->GetDenominator());
				}
			}

			MidiMarkerInfo info;
			info.startSec = startSec;
			info.startTick = startTick;
			info.microTempo = microTempo;
			info.num = num;
			info.den = den;
			midiMarkerInfos.push_back(info);
		}

		MidiMarkerInfo info;
		info.startSec = 0;
		info.startTick = midiEndTick;
		info.microTempo = 0;
		info.num = 0;
		info.den = 0;
		midiMarkerInfos.push_back(info);


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
			beatCostSec = 4.0f / den * mInfoA.microTempo;
			beatCostTicks = 4.0f / den * qNoteTicks;
			measureCostSec = beatCostSec * num;
			measureCostTicks = beatCostTicks * num;

			float curtSec = mInfoA.startSec;
			int curtTick = mInfoA.startTick;
			int endTick = mInfoB.startTick;

			while (curtTick < endTick)
			{
				++mIdx;
				measure[mIdx].startSec = curtSec;
				measure[mIdx].startTick = curtTick;
				measure[mIdx].beatNum = num;
				measure[mIdx].beatCostSec = beatCostSec;
				curtSec += measureCostSec;
				curtTick += measureCostTicks;

				//
				if (curtTick >= endTick && i + 1 == midiMarkerInfos.size() - 1)
				{
					mEndTick = curtTick - 1;
					mEndSec = curtSec;
				}

			}	
		}	

		measureNum = mIdx + 1;
	}


}