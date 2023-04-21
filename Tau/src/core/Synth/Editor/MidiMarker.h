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

		inline int GetTrack()
		{
			return track;
		}

		bool IsEnableMarkerText()
		{
			return isEnableMarkerText;
		}

		bool IsEnableTempo()
		{
			return isEnableTempo;
		}

		inline int32_t GetStartTick()
		{
			return startTick;
		}

		inline double GetStartSec()
		{
			return startSec;
		}

		inline void SetStartTick(int32_t tick)
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

		inline int32_t GetTickForQuarterNote()
		{
			return tickForQuarterNote;
		}

		bool IsEnableTimeSignature()
		{
			return isEnableTimeSignature;
		}


		inline int GetNumerator()
		{
			return numerator;
		}

		inline int GetDenominator()
		{
			return denominator;
		}

		bool IsEnableKeySignature()
		{
			return isEnableKeySignature;
		}

		inline int GetSf()
		{
			return sf;
		}

		inline int GetMi()
		{
			return mi;
		}

		//设置标题
		inline void SetTitleName(string& name)
		{
			titleName = name;
		}

		//marker文本设置
		inline void SetMarkerText(string& text)
		{
			titleName = text;
			isEnableMarkerText = true;
		}

		//获取标题
		string GetTitle()
		{
			return titleName;
		}


		//速度设置
		inline void SetTempo(float microTempo_, int32_t tickForQuarterNote_, bool isEnable)
		{
			titleName = "速度变更";
			isEnableTempo = isEnable;
			tickForQuarterNote = tickForQuarterNote_;
			microTempo = microTempo_;
		}

		//节拍设置
		inline void SetTimeSignature(float numerator_, float denominator_, bool isEnable)
		{
			titleName = "节拍变更";
			isEnableTimeSignature = isEnable;
			numerator = numerator_;
			denominator = denominator_;
		}


		//调号设置
		inline void SetKeySignature(float sf_, float mi_, bool isEnable)
		{
			titleName = "调号变更";
			isEnableKeySignature = isEnable;
			sf = sf_;
			mi = mi_;
		}

		//获取调号序号
		int GetIdx() {
			return (sf < 0 ? 7 - sf : sf);
		}

		//获取调号名称
		string GetKeySignatureName()
		{
			int n = GetIdx();
			return (mi == 0 ? majorName[n] : minorName[n]);
		}


		// 获取调号全名
		string GetKeySignatureFullName()
		{
			int n = GetIdx();
			return (mi == 0 ? majorFullName[n] : minorFullName[n]);
		}

		//计算速度
		void ComputeTempo();

	private:
		static string majorName[];
		static string majorFullName[];

		static string minorName[];
		static string minorFullName[];

	private:

		//所在轨道
		int track = 0;

		//是否开启marker文本
		bool isEnableMarkerText = false;

		string titleName;

		//是否开启速度设置
		bool isEnableTempo = false;

		//标记起始的tick数
		int32_t startTick = 0;

		//标记起始的时间点
		double startSec = 0;

		// 一个四分音符的微秒数
		float microTempo = 0;

		// 一个四分音符的tick数
		int32_t tickForQuarterNote = 480;

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
