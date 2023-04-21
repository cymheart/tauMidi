#include"Synther.h"
#include"Synth/Tau.h"
#include"Synth/VirInstrument.h"
#include"Synth/Editor/MidiEditor.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"


namespace tau
{

	/// <summary>
	/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在将在虚拟乐器列表中自动创建它
	/// </summary>
	/// <param name="deviceChannel">乐器所在设备通道</param>
	/// <param name="bankSelectMSB">声音库选择0</param>
	/// <param name="bankSelectLSB">声音库选择1</param>
	/// <param name="instrumentNum">乐器编号</param>
	/// <returns></returns>
	VirInstrument* Synther::EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{

		Channel* channel = GetDeviceChannel(deviceChannelNum);
		if (channel == nullptr)
		{
			channel = new Channel(deviceChannelNum);
			channel->isDeviceChannel = true;
			deviceChannelMap[deviceChannelNum] = channel;
		}

		return EnableVirInstrument(channel, bankSelectMSB, bankSelectLSB, instrumentNum);
	}


	// 删除虚拟乐器
	void Synther::DelVirInstrument(VirInstrument* virInst)
	{
		if (virInst == nullptr)
			return;

		RemoveVirInstFromList(virInst);

		//移除对应的设备通道
		Channel* channel = virInst->GetChannel();

		if (channel->IsDeviceChannel() &&
			channel->GetVirInstrumentCount() == 1)
		{
			int num = channel->GetChannelNum();
			auto itc = deviceChannelMap.find(num);
			if (itc != deviceChannelMap.end())
			{
				deviceChannelMap.erase(itc);
				DEL(channel);
			}
		}

		//
		DEL(virInst);
	}

	//根据设备通道号获取设备通道
	//查找不到将返回空值
	Channel* Synther::GetDeviceChannel(int deviceChannelNum)
	{
		auto it = deviceChannelMap.find(deviceChannelNum);
		if (it != deviceChannelMap.end()) {
			return it->second;
		}

		return nullptr;
	}



}
