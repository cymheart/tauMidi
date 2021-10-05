#include"Tempo.h"

namespace tau
{

	void Tempo::Copy(Tempo& cpyTempo)
	{
		BPM = cpyTempo.BPM;
		msPerTick = cpyTempo.msPerTick;
		tickForQuarterNote = cpyTempo.tickForQuarterNote;
		curtTempoStartTick = cpyTempo.curtTempoStartTick;
		curtTempoStartSec = cpyTempo.curtTempoStartSec;
	}

	/// <summary>
	/// 设置轨道速度
	/// </summary>
	/// <param name="microTempo">一个四分音符的微秒数</param>
	/// <param name="tickForQuarterNote">一个四分音符的tick数</param>
	void Tempo::SetTempo(float microTempo, float tickForQuarterNote, int startTickCount, double startSec)
	{
		curtTempoStartSec = startSec;
		msPerTick = microTempo / tickForQuarterNote * 0.001f;
		BPM = 60000000 / microTempo;  //60000000: 1分钟的微秒数
		curtTempoStartTick = startTickCount;
	}

	//根据给定时间点获取tick的数量
	//speed:播放速率(相对于正常播放速率1.0的倍率)
	uint32_t Tempo::GetTickCount(double sec, float speed)
	{
		return curtTempoStartTick + (uint32_t)((sec - curtTempoStartSec) * 1000 / msPerTick * speed);
	}

	//根据给定tick数量获取时间点
	//speed:播放速率(相对于正常播放速率1.0的倍率)
	double Tempo::GetTickSec(uint32_t tickCount, float speed)
	{
		if (tickCount < curtTempoStartTick)
			tickCount = curtTempoStartTick;
		return curtTempoStartSec + (tickCount - curtTempoStartTick) * msPerTick / speed * 0.001f;
	}
}
