#ifndef _Channel_h_
#define _Channel_h_

#include"TauTypes.h"
#include"Midi/MidiTypes.h"

namespace tau
{

	class  Channel
	{

	public:
		Channel(int channelNum);
		~Channel();

		//是否为设备通道
		inline bool IsDeviceChannel()
		{
			return isDeviceChannel;
		}

		//获取通道号
		inline int GetChannelNum()
		{
			return channelNum;
		}


		//设置通道号
		void SetChannelNum(int num);

		//设置声音增益(单位:dB)
		void SetVolumeGain(float gainDB) {
			volumeGainDB = gainDB;
		}

		//获取声音增益(单位:dB)
		float GetVolumeGain() {
			return volumeGainDB;
		}

		//增加对应乐器
		inline void AddVirInstrument(VirInstrument* vinst)
		{
			insts.push_back(vinst);
		}

		void DelVirInstrument(VirInstrument* vinst)
		{
			for (auto it = insts.begin(); it != insts.end(); ++it)
			{
				if (*it == vinst) {
					insts.erase(it);
					break;
				}
			}
		}

		//获取对应乐器
		inline vector<VirInstrument*>& GetVirInstruments()
		{
			return insts;
		}

		//获取乐器的数量
		inline int GetVirInstrumentCount()
		{
			return insts.size();
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
		vector<ModInputPreset>& GetUsedPresetTypeList();

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

		float MapValueToSys(MidiControllerType type, float value, bool isHighResValue)
		{
			return isHighResValue ? value / 16384.0f : value / 127.0f;
		}


		//增加使用的控制器类型
		void AddUsedControllerType(MidiControllerType type);

		//增加使用的预设类型
		void AddUsedPresetType(ModInputPreset type);

	private:

		//是否为设备通道
		bool isDeviceChannel = false;

		//通道号
		int channelNum = 0;

		//所在乐器
		vector<VirInstrument*> insts;

		//
		int bankMSB = 0;
		int bankLSB = 0;
		int programNum = 0;

		//
		bool ccUsed[128] = { false };
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

		//声音增益(单位:dB)
		float volumeGainDB = 0;

		MidiControllerTypeList usedControllerTypeList;
		vector<ModInputPreset> usedPresetTypeList;

		friend class VirInstrument;
		friend class Synther;

	};
}

#endif
