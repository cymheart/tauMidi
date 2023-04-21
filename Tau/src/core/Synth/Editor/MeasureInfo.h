#ifndef _MeasureInfo_h_
#define _MeasureInfo_h_

#include "Synth/TauTypes.h"
#include"MidiMarkerList.h"


namespace tau
{
	// 小节信息
	//by cymheart, 2022.
	class MeasureInfo
	{
	public:

		//生成小节
		void Create(MidiMarkerList& midiMarkerList, float endSec);

		void Clear()
		{
			mIdx = 0;
			bIdx = 0;
			measureNum = 0;
			atMeasure = 1;
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
			return measure[i * 2];
		}

		//获取指定小节的结束时间点
		float GetMeasureEndSec(int i)
		{
			i = min(measureNum, i);
			if (i == measureNum)
				return beat[bIdx];

			return measure[i * 2];
		}

		//获取小节拍子数量
		int GetMeasureBeatCount(int i)
		{
			i = min(measureNum, i);
			if (i == measureNum) {
				int a = measure[(i - 1) * 2 + 1];
				return bIdx - a + 1;
			}

			//
			int a = measure[(i - 1) * 2 + 1];
			int b = measure[i * 2 + 1];
			return b - a + 1;

		}

		//获取小节指定拍子的结束时间点
		float GetMeasureBeatEndSec(int measureIdx, int beatIdx)
		{
			int a = measure[(measureIdx - 1) * 2 + 1];
			return beat[a + (beatIdx - 1)];
		}


		float* GetMeasureDatas()
		{
			return measure;
		}

		int GetMeasureDataSize()
		{
			return mIdx + 1;
		}

		float* GetBeatDatas()
		{
			return beat;
		}

		int GetBeatDataSize()
		{
			return bIdx + 1;
		}

		//获取指定时间所在的小节
		int GetSecAtMeasure(float sec)
		{
			int left = 0, right = measureNum - 1;
			int curt = 0;

			while (true)
			{
				curt = (right + left) / 2;
				float s = measure[curt * 2];
				if (s > sec) right = curt;
				else if (s < sec) left = curt;
				else return curt + 1;

				if (left + 1 == right || left == right)
					return left + 1;
			}
		}

	private:

		//小节信息
		float measure[10000];

		//拍子信息
		float beat[10000];

		//小节结束位置
		int mIdx = 0;

		//拍子结束位置
		int bIdx = 0;

		//小节数量
		int measureNum = 0;

		//所在小节
		int atMeasure = 1;

	};
}

#endif