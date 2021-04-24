#ifndef _Track_h_
#define _Track_h_

#include "VentrueTypes.h"

namespace ventrue
{
	// 演奏音轨
	//by cymheart, 2020--2021.
	class Track
	{
	public:
		Track(int num);
		~Track();

		//设置轨道号
		inline void SetNum(int num)
		{
			this->num = num;
		}

		//获取轨道号
		inline int GetNum()
		{
			return num;
		}

		Channel* operator[] (int n);
		void Clear();


		/// <summary>
		/// 设置轨道速度
		/// </summary>
		/// <param name="microTempo">一个四分音符的微秒数</param>
		/// <param name="tickForQuarterNote">一个四分音符的tick数</param>
		/// <param name="startTick">开始设置速度的tick数</param>
		void SetTempo(float microTempo, float tickForQuarterNote, int startTickCount);

		//根据当前时间点,计算已经过的tick的数量   
		void CalCurtTicksCount(double sec);

		//根据给定时间点获取tick的数量
		uint32_t GetTickCount(double sec);

		//根据给定tick数量获取时间点
		double GetTickSec(int tickCount);

		//禁止播放通道
		void DisablePlayChannel(int n);
		//开启播放通道
		void EnablePlayChannel(int n);

	public:

		/// <summary>
		/// 轨道号
		/// </summary>
		int num = 0;

		/// <summary>
		/// 每tick的毫秒数
		/// 预设4.166ms
		/// </summary>
		float msPerTick = 4.166f;

		/// <summary>
		/// 在音乐中我们一般用BPM来表述乐曲的速度，BPM(Beat per Minute)的意思是每分钟的拍子数。
		/// 例如，BPM=100，表示该歌曲的速度是每分钟100拍。注意，对于音乐家来说，BPM中的一拍是指一个四分音符所发音的时间，
		/// 而不管歌曲的拍号是多少.例如，假设歌曲的拍号是3/8拍(以八分音符为一拍，每小节3拍)，BPM=100，
		/// 那么，音乐家依然会把歌曲的速度认为是以四分音符(非八分音符)为一拍，每分钟100拍。
		/// 因此，BPM被称为是“音乐速度(Musical Tempo)”
		/// </summary>
		float BPM = 120;

		/// <summary>
		/// 对应音轨事件队列的当前处理位置
		/// </summary>
		int eventOffsetIdx = 0;
		// <summary>
		/// 对应音轨事件队列的当前处理位置
		/// </summary>
		list<MidiEvent*>::iterator eventOffsetIter;

		//是否播放结束
		bool isEnded = false;

		//是否禁止播放
		bool isDisablePlay = false;


		// 重新调整播放速度前，已经过tick数量
		uint32_t baseTickCount = 0;
		double baseTickTime = 0;

		/// <summary>
		/// 当前已经过的tick个数
		/// </summary>
		uint32_t curtTickCount = 0;

		/// <summary>
		/// 通道列表
		/// </summary>
		Channel* channels[16];


	};


}

#endif
