#ifndef _MeasureInfo_h_
#define _MeasureInfo_h_

#include "Synth/TauTypes.h"
#include"MidiMarkerList.h"

//cymheart 2025
namespace tau
{
	struct MeasureData {
		float startSec;
		int startTick;
		int beatNum;
		float beatCostSec;
	};

	struct MidiMarkerInfo
	{
		float startSec;
		int32_t startTick;
		double microTempo = 0;
		// 拍号分子(以den分音符为一拍，每小节有num拍)
		int num = -1;
		// 拍号分母(以den分音符为一拍)
		int den = -1;
	};

	// 小节信息
	//by cymheart, 2022.
	class MeasureInfo
	{
	public:

		//生成小节
		void Create(MidiMarkerList& midiMarkerList, int midiEndTick);

		void Clear()
		{
			measureNum = 0;
			atMeasure = 1;
			mEndTick = 0;
		}

		//获取小节数量
		int GetMeasureCount()
		{
			return measureNum;
		}

		//获取指定小节的开始时间点
		float GetMeasureStartSec(int i)
		{
			i = min(measureNum, i) - 1;
			return measure[i].startSec;
		}

		//获取指定小节的结束时间点
		float GetMeasureEndSec(int i)
		{
			i = min(measureNum, i);
			if (i == measureNum)
				return mEndSec;

			return measure[i].startSec;
		}

		//获取指定小节的开始tick点
		float GetMeasureStartTick(int i)
		{
			i = min(measureNum, i) - 1;
			return measure[i].startTick;
		}

		//获取指定小节的结束tick点
		float GetMeasureEndTick(int i)
		{
			i = min(measureNum, i);
			if (i == measureNum)
				return mEndTick;

			return measure[i].startTick - 1;
		}


		//获取小节拍子数量
		int GetMeasureBeatCount(int i)
		{
			i = min(measureNum, i);
			float mSec = GetMeasureEndSec(i) - GetMeasureStartSec(i) + 0.001f;
			return (int)(mSec / measure[i - 1].beatCostSec);
		}

		//获取小节指定拍子的结束时间点
		float GetMeasureBeatEndSec(int measureIdx, int beatIdx)
		{
			int i = min(measureNum, measureIdx);
			float mSec = GetMeasureEndSec(i) - GetMeasureStartSec(i);
			return measure[i - 1].startSec + beatIdx * measure[i - 1].beatCostSec;
		}


		//获取指定时间所在的小节
		int GetSecAtMeasure(float sec)
		{
			int left = 0, right = measureNum - 1;
			int curt = 0;

			while (true)
			{
				curt = (right + left) / 2;
				float s = measure[curt].startSec;
				if (s > sec) right = curt;
				else if (s < sec) left = curt;
				else return curt + 1;

				if (left + 1 == right || left == right)
					return left + 1;
			}
		}

	private:

		//小节信息
		MeasureData measure[3000];

		//小节数量
		int measureNum = 0;

		//所在小节
		int atMeasure = 1;

		int mEndTick = 0;
		float mEndSec = 0;

	};
}

#endif