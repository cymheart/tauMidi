#ifndef _MidiMarker_h_
#define _MidiMarker_h_

#include "Synth/TauTypes.h"
#include "Midi/MidiTypes.h"
#include "Tempo.h"

namespace tau
{
	/// <summary>
	/// Midi标记类
	/// by cymheart, 2021.
	/// </summary> 
	class MidiMarker
	{
	public:
		MidiMarker();
		~MidiMarker();

		void Copy(MidiMarker& cpyMidiMarker);

		inline uint32_t GetStartTick()
		{
			return startTick;
		}

		inline double GetStartSec()
		{
			return startSec;
		}

		inline void SetStartTick(uint32_t tick)
		{
			startTick = tick;
		}

		inline void SetStartSec(double sec)
		{
			startSec = sec;
		}

		inline float GetMicroTempo()
		{
			return microTempo;
		}

		inline uint32_t GetTickForQuarterNote()
		{
			return tickForQuarterNote;
		}

		//速度设置
		inline void SetTempo(float microTempo_, uint32_t tickForQuarterNote_, bool isEnable)
		{
			isEnableTempo = isEnable;
			tickForQuarterNote = tickForQuarterNote_;
			microTempo = microTempo_;
		}

		//节拍设置
		inline void SetTimeSignature(float numerator_, float denominator_, bool isEnable)
		{
			isEnableTimeSignature = isEnable;
			numerator = numerator_;
			denominator = denominator_;
		}


		//调号设置
		inline void SetKeySignature(float sf_, float mi_, bool isEnable)
		{
			isEnableKeySignature = isEnable;
			sf = sf_;
			mi = mi_;
		}

		//计算速度
		void ComputeTempo();

	private:

		//是否开启速度设置
		bool isEnableTempo = false;

		//标记起始的tick数
		uint32_t startTick = 0;

		//标记起始的时间点
		double startSec = 0;

		// 一个四分音符的微秒数
		float microTempo = 0;

		// 一个四分音符的tick数
		uint32_t tickForQuarterNote = 480;

		//
		//是否开启节拍设置
		bool isEnableTimeSignature = false;

		// 拍号分子
		int numerator = 4;

		// 拍号分母
		// denominatorResult = pow(2, denominator);
		int denominator = 2;

		//
		 //是否开启调号设置
		bool isEnableKeySignature = false;

		/// <summary>
		/// sf指明乐曲曲调中升号、降号的数目。
		/// 也就是说，升号数目写成0x，降号数目写成8x
		/// sf = -7: 7 flats 7个降号
		/// sf = -1: 1 flat  1个降号
		/// sf = 0: key of C C调
		/// sf = 1: 1 sharp  1个升调
		/// sf = 7: 7 sharps 7个升调
		/// </summary>
		int sf = 0;

		/// <summary>
		/// 大调小调
		/// 0:major (大调)
		/// 1:minor(小调)
		/// </summary>
		int mi = 0;

		Tempo tempo;

		friend class MidiMarkerList;
	};
}

#endif
