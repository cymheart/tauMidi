#include"Channel.h"
#include"UnitTransform.h"
#include"Modulator.h"

namespace ventrue
{
    Channel::Channel(int channelNum)
    {
        Clear();
        this->channelNum = channelNum;
    }

    Channel::~Channel()
    {

    }

    //手动选择乐器
    void Channel::SelectProgram(int bankMSB, int bankLSB, int instNum)
    {
        SetControllerValue(MidiControllerType::BankSelectMSB, bankMSB);
        SetControllerValue(MidiControllerType::BankSelectLSB, bankLSB);
        SetProgramNum(instNum);
    }

    void Channel::SetProgramNum(int num)
    {
        bankMSB = ccValue[(int)MidiControllerType::BankSelectMSB];
        bankLSB = ccValue[(int)MidiControllerType::BankSelectLSB];
        programNum = num;

        //录制
        if (midiTrackRecord)
            midiTrackRecord->RecordSetProgramNum(programNum, channelNum);
    }

    //设置滑音
    void Channel::SetPitchBlend(int value)
    {
        pitchBlend = (float)value;
        AddUsedPresetType(ModInputPreset::PitchWheel);

        //录制
        if (midiTrackRecord)
            midiTrackRecord->RecordSetPitchBend(value, channelNum);
    }

    //设置控制器值
    //0~31(MSB) 和 32~64(LSB)成对出现
    //如果先收到一个MSB值，那么
    
    void Channel::SetControllerValue(MidiControllerType type, int value)
    {
       
        int itype = (int)type;

        ccValue[itype] = value;

        //如果接收道一个0~31的MSB值,那对应的32~63的LSB值被重置为0
        if (itype <= 31)
        {
            ccValue[itype + 32] = 0;
        }

        if (itype >= 0 && itype <= 63)
        {
            int highResType;
            if (itype <= 31) { highResType = itype; }
            else { highResType = itype - 32; }
            ComputeControllerHighResValue(highResType);
            AddUsedControllerType((MidiControllerType)highResType);
        }
        else if (itype <= 127)
        {
            ccComputedValue[itype] = MapValueToSys(type, (float)value, false);
            AddUsedControllerType(type);
        }


        //录制
        if(midiTrackRecord)
            midiTrackRecord->RecordSetController(type, value, channelNum);
    }

    //增加使用的控制器类型
    void Channel::AddUsedControllerType(MidiControllerType type)
    {
        for (int i = 0; i < usedControllerTypeList.size(); i++)
        {
            if (usedControllerTypeList[i] == type)
                return;
        }

        usedControllerTypeList.push_back(type);
    }

    //增加使用的预设类型
    void Channel::AddUsedPresetType(ModInputPreset type)
    {
        for (int i = 0; i < usedPresetTypeList.size(); i++)
        {
            if (usedPresetTypeList[i] == type)
                return;
        }

        usedPresetTypeList.push_back(type);
    }

    MidiControllerTypeList& Channel::GetUsedControllerTypeList()
    {
        return usedControllerTypeList;
    }

    ModPresetTypeList& Channel::GetUsedPresetTypeList()
    {
        return usedPresetTypeList;
    }

    int Channel::GetControllerValue(MidiControllerType type)
    {
        return ccValue[(int)type];
    }

    float Channel::GetModPresetValue(ModInputPreset presetType)
    {
        switch (presetType)
        {
        case ventrue::ModInputPreset::None:
            break;
        case ventrue::ModInputPreset::NoteOnVelocity:
            break;
        case ventrue::ModInputPreset::NoteOnKey:
            break;
        case ventrue::ModInputPreset::PolyPressure:
            break;
        case ventrue::ModInputPreset::ChannelPressure:
            break;
        case ventrue::ModInputPreset::PitchWheel:return pitchBlend;
        case ventrue::ModInputPreset::PitchWheelSensivity:
            break;
        default:
            break;
        }

        return 0;
    }

    float Channel::GetControllerComputedValue(MidiControllerType type)
    {
        int itype = (int)type;

        if (itype <= 31 || itype > 63)
        {
            return ccComputedValue[itype];
        }
        else
        {
            return ccComputedValue[itype - 32];
        }
    }


    void Channel::ComputeControllerHighResValue(int type)
    {
        int msbType = type;
        int lsbType = type + 32;

        if (ccValue[lsbType] != 0)
        {
            float value = (float)(ccValue[msbType] << 7 | ccValue[lsbType]);
            ccComputedValue[type] = MapValueToSys((MidiControllerType)type, value, true);
        }
        else
        {
            ccComputedValue[type] = MapValueToSys((MidiControllerType)type, (float)ccValue[msbType], false);
        }
    }

    float Channel::MapValueToSys(MidiControllerType type, float value, bool isHighResValue)
    {
        if (isHighResValue) { return value / 16384.0f; }
        return value / 127.0f;
    }


    void Channel::Clear()
    {
        memset(ccValue, 0, sizeof(int) * 128);
        memset(ccComputedValue, 0, sizeof(float) * 128);

        ccComputedValue[(int)MidiControllerType::PanMSB] = 0;
        ccComputedValue[(int)MidiControllerType::ExpressionControllerMSB] = 1;
        ccComputedValue[(int)MidiControllerType::ChannelVolumeMSB] = 1;
    }
}