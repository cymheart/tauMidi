#ifndef _Tempo_h_
#define _Tempo_h_

#include "Synth/TauTypes.h"
#include "InstFragment.h"

namespace tau
{
	// 速度
	//by cymheart, 2021.
	class Tempo
	{
	public:
		void Copy(Tempo& cpyTempo);

		// <summary>
	    /// 设置轨道速度
	    /// </summary>
	    /// <param name="bpm">节拍（每分钟的拍子数）</param>
	    /// <param name="tickForQuarterNote">一个四分音符的tick数</param>
		void SetTempoByBPM(float bpm, float tickForQuarterNote, int startTickCount, double startSec);

		/// <summary>
		/// 设置轨道速度
		/// </summary>
		/// <param name="microTempo">一个四分音符的微秒数</param>
		/// <param name="tickForQuarterNote">一个四分音符的tick数</param>
		void SetTempo(float microTempo, float tickForQuarterNote, int startTickCount, double startSec);

		//根据给定时间点获取tick的数量
		//speed:播放速率(相对于正常播放速率1.0的倍率)
		uint32_t GetTickCount(double sec, float speed = 1.0f);

		//根据给定tick数量获取时间点
		//speed:播放速率(相对于正常播放速率1.0的倍率)
		double GetTickSec(uint32_t tickCount, float speed = 1.0f);

	private:
		/// <summary>
		/// 在音乐中我们一般用BPM来表述乐曲的速度，BPM(Beat per Minute)的意思是每分钟的拍子数。
		/// 例如，BPM=100，表示该歌曲的速度是每分钟100拍。注意，对于音乐家来说，BPM中的一拍是指一个四分音符所发音的时间，
		/// 而不管歌曲的拍号是多少.例如，假设歌曲的拍号是3/8拍(以八分音符为一拍，每小节3拍)，BPM=100，
		/// 那么，音乐家依然会把歌曲的速度认为是以四分音符(非八分音符)为一拍，每分钟100拍。
		/// 因此，BPM被称为是“音乐速度(Musical Tempo)”
		/// </summary>
		float BPM = 120;

		/// <summary>
		/// 每tick的毫秒数
		/// 预设4.166ms
		/// </summary>
		float msPerTick = 4.166f;

		// 一个四分音符的tick数
		uint32_t tickForQuarterNote = 480;

		// 当前速度开始的tick数
		uint32_t curtTempoStartTick = 0;

		// 当前速度开始时间点
		double curtTempoStartSec = 0;
	};
}

#endif
