#include"MeasureInfo.h"

namespace tau
{

	//生成小节信息
	void MeasureInfo::Create(MidiMarkerList& midiMarkerList, float endSec)
	{
		measureNum = 0;
		mIdx = -1;
		bIdx = -1;
		double curtSec = 0;

		//一个四分音符的秒数
		double microTempo = 0;
		// 拍号分子(以den分音符为一拍，每小节有num拍)
		int num = 4;
		// 拍号分母(以den分音符为一拍)
		int den = 4;

		float beatCostSec, newBeatCostSec;
		MidiMarker* midiMarker;
		MidiMarker* nextMarker = nullptr;
		double beatStartSec;
		double nextMarkerStartSec;
		int state = 0;
		int j;

		vector<MidiMarker*>& midiMarkers = midiMarkerList.GetMidiMarkers();

		for (int i = 0; i < midiMarkers.size(); i++)
		{
			midiMarker = midiMarkers[i];
			if (midiMarker->IsEnableTempo()) {
				microTempo = midiMarker->GetMicroTempo() * 0.000001f;
				if (state == 1)
					state = 2;
			}

			if (midiMarker->IsEnableTimeSignature())
			{
				num = midiMarker->GetNumerator();
				den = pow(2, midiMarker->GetDenominator());
				state = 2;
				if (microTempo == 0) {
					state = 1;
					continue;
				}
			}

			if (state != 2)
				continue;
			state = 0;

			//
			beatCostSec = den / 4.0f * microTempo;

			bool flag = true;
			while (flag)
			{
				measure[++mIdx] = curtSec;
				measure[++mIdx] = bIdx + 1;
				measureNum = (mIdx + 1) / 2;
				beatStartSec = curtSec;

				int numIdx = num;
				while (numIdx--)
				{
					curtSec += beatCostSec;
					beat[++bIdx] = curtSec;

					if (nextMarker != nullptr)
					{
						state = 2;
						if (curtSec > nextMarkerStartSec)
						{
							if (nextMarker->IsEnableTempo())
							{
								curtSec = nextMarkerStartSec + (curtSec - nextMarkerStartSec) * newBeatCostSec / beatCostSec;
								beat[bIdx] = curtSec;
								beatCostSec = newBeatCostSec;
							}

							if (nextMarker->IsEnableTimeSignature())
							{
								curtSec -= beatCostSec;

								num = nextMarker->GetNumerator();
								numIdx = num - 1;
								den = pow(2, nextMarker->GetDenominator());
								beatCostSec = den / 4.0f * microTempo;

								curtSec += beatCostSec;
								beat[bIdx] = curtSec;

							}

							state = 0;
							nextMarker = nullptr;
						}
					}

					if (state != 0) {
						if (curtSec >= endSec)
							return;
						continue;
					}


					for (j = i + 1; j < midiMarkers.size(); j++)
					{
						if (midiMarkers[j]->IsEnableTempo())
						{
							nextMarker = midiMarkers[j];
							microTempo = nextMarker->GetMicroTempo() * 0.000001f;
							newBeatCostSec = den / 4.0f * microTempo;
							nextMarkerStartSec = nextMarker->GetStartSec();
						}


						if (midiMarkers[j]->IsEnableTimeSignature()) {
							nextMarker = midiMarkers[j];
							nextMarkerStartSec = nextMarker->GetStartSec();
						}

						if (nextMarker != nullptr && curtSec >= nextMarkerStartSec)
						{
							if (midiMarkers[j]->IsEnableTempo()) {
								curtSec = nextMarkerStartSec + (curtSec - nextMarkerStartSec) * newBeatCostSec / beatCostSec;
								beat[bIdx] = curtSec;
								beatCostSec = newBeatCostSec;
							}

							if (midiMarkers[j]->IsEnableTimeSignature()) {
								curtSec = midiMarkers[j]->GetStartSec();
								beat[bIdx] = curtSec;
								numIdx = 0;
								i = j - 1;
								flag = false;
								nextMarker = nullptr;
								break;
							}
						}
						else {
							i = j;
							break;
						}

					}

					if (j == midiMarkers.size())
						i = j;

					if (curtSec >= endSec)
						return;
				}
			}

			state = 0;
		}
	}

}