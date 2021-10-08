#ifndef _Channel_h_
#define _Channel_h_

#include"TauTypes.h"
#include"Midi/MidiTypes.h"

namespace tau
{
	class Channel
	{

	public:
		Channel(int channelNum);
		~Channel();

		//获取通道号
		inline int GetChannelNum()
		{
			return channelNum;
		}

		//设置通道号
		void SetChannelNum(int num);

		//设置对应乐器
		inline void SetVirInstrument(VirInstrument* vinst)
		{
			inst = vinst;
		}

		//获取对应乐器
		inline VirInstrument* GetVirInstrument()
		{
			return inst;
		}

		void Clear();

		inline int GetBankSelectMSB()
		{
			return bankMSB;
		}

		inline int GetBankSelectLSB()
		{
			return bankLSB;
		}

		inline int GetProgramNum()
		{
			return programNum;
		}

		//手动选择乐器
		void SelectProgram(int bankMSB, int bankLSB, int instNum);

		//设置乐器
		void SetProgramNum(int num);

		void SetControllerValue(MidiControllerType type, int value);
		int GetControllerValue(MidiControllerType type);

		float GetModPresetValue(ModInputPreset presetType);

		///获取控制器计算值
		float GetControllerComputedValue(MidiControllerType type);

		MidiControllerTypeList& GetUsedControllerTypeList();
		ModPresetTypeList& GetUsedPresetTypeList();

		//设置滑音
		void SetPitchBend(int value);


		//获取滑音范围
		inline float GetPitchBendRange()
		{
			return pitchBendRange;
		}

		//设置PolyPressure
		void SetPolyPressure(int pressure);

		//设置按键
		void SetNoteOnKey(int keyNum, float velocity);

	private:

		void ComputeControllerHighResValue(int type);
		float MapValueToSys(MidiControllerType type, float value, bool isHighResValue);

		//增加使用的控制器类型
		void AddUsedControllerType(MidiControllerType type);

		//增加使用的预设类型
		void AddUsedPresetType(ModInputPreset type);

	private:
		//通道号
		int channelNum = 0;

		//所在乐器
		VirInstrument* inst = nullptr;

		//
		int bankMSB = 0;
		int bankLSB = 0;
		int programNum = 0;

		//
		int ccValue[128] = { 0 };
		float ccCombValue[128] = { 0 };
		float ccComputedValue[128] = { 0 };
		int noteOnKeyNum = 0;
		float noteOnKeyVelocity = 0;
		float pitchBend = 0;
		float pitchBendRange = 2;
		float pressure = 1;
		float fineTune = 0;
		float coarseTune = 0;

		MidiControllerTypeList usedControllerTypeList;
		ModPresetTypeList usedPresetTypeList;

		friend class VirInstrument;
		friend class Synther;

	};
}

#endif
