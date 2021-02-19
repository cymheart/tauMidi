#include"Modulator.h"

namespace ventrue
{
    Modulator::Modulator()
    {
        inputInfos = new ModInputInfoList;
    }

    Modulator::~Modulator()
    {
        DEL_OBJS_VECTOR(inputInfos);
    }

    //判断是否和modulator相似
    bool Modulator::IsSame(Modulator* modulator)
    {
        if (inputInfos == nullptr || modulator->inputInfos == nullptr)
            return false;

        if (inputInfos->size() != modulator->inputInfos->size())
            return false;

        if (GetOutTargetGeneratorType() == GeneratorType::None ||
            modulator->GetOutTargetGeneratorType() == GeneratorType::None ||
            GetOutTargetGeneratorType() != modulator->GetOutTargetGeneratorType())
        {
            return false;
        }

        if (sourceTransTypes[0] != modulator->sourceTransTypes[0] ||
            sourceTransTypes[1] != modulator->sourceTransTypes[1] ||
            modTransformFunc[0] != modulator->modTransformFunc[0] ||
            modTransformFunc[1] != modulator->modTransformFunc[1])
        {
            return false;
        }

        if (polar[0] != modulator->polar[0] ||
            polar[1] != modulator->polar[1] ||
            dir[0] != modulator->dir[0] ||
            dir[1] != modulator->dir[1])
        {
            return false;
        }

        for (int i = 0; i < inputInfos->size(); i++)
        {
            ModInputInfo& a = *((*inputInfos)[i]);
            ModInputInfo& b = *((*modulator->inputInfos)[i]);

            if (a.ctrlType == b.ctrlType &&
                a.inputModulator == b.inputModulator &&
                a.inputNativeValue == b.inputNativeValue &&
                a.inputNativeValueMax == b.inputNativeValueMax &&
                a.inputNativeValueMin == b.inputNativeValueMin &&
                a.inputPort == b.inputPort &&
                a.inputPreset == b.inputPreset &&
                a.inputType == b.inputType)
            {
                continue;
            }

            return false;
        }

        return true;
    }

    //<summary>
    //增加一个输入到调制器的指定端口
    //< / summary>
    //<param name = "inputType">输入类型< / param>
    //<param name = "inputPort">输入端口< / param>
    //<param name = "inputNativeValueMin">输入的原生最小值< / param>
    //<param name = "inputNativeValueMax">输入的原生最大值< / param>
    void Modulator::AddInputInfo(
        ModInputType inputType, ModInputPreset inputPreset, MidiControllerType ctrlType,
        int inputPort, float inputNativeValueMin, float inputNativeValueMax)
    {
        ModInputInfo* info = new ModInputInfo();
        info->inputType = inputType;
        info->inputPreset = inputPreset;
        info->ctrlType = ctrlType;
        info->inputPort = inputPort;
        info->inputNativeValueMin = inputNativeValueMin;
        info->inputNativeValueMax = inputNativeValueMax;
        info->outputModulator = this;
        inputInfos->push_back(info);
        usePortCount[(int)inputPort]++;
    }


    //<summary>
   //增加一个预设输入到调制器的指定端口
   //< / summary>
   //<param name = "inputPort">输入端口< / param>
   //<param name = "inputNativeValueMin">输入的原生最小值< / param>
   //<param name = "inputNativeValueMax">输入的原生最大值< / param>
    void Modulator::AddPresetInputInfo(
        ModInputPreset inputPreset, int inputPort,
        float inputNativeValueMin, float inputNativeValueMax)
    {
        ModInputInfo* info = new ModInputInfo();
        info->inputType = ModInputType::Preset;
        info->inputPreset = inputPreset;
        info->ctrlType = MidiControllerType::CC_None;
        info->inputPort = inputPort;
        info->inputNativeValueMin = inputNativeValueMin;
        info->inputNativeValueMax = inputNativeValueMax;
        info->outputModulator = this;
        inputInfos->push_back(info);
        usePortCount[(int)inputPort]++;
    }

    //<summary>
    //增加一个Midi控制器输入到调制器的指定端口
    //< / summary>
    //<param name = "inputType">输入类型< / param>
    //<param name = "inputPort">输入端口< / param>
    //<param name = "inputNativeValueMin">输入的原生最小值< / param>
    //<param name = "inputNativeValueMax">输入的原生最大值< / param>
    void Modulator::AddMidiCtrlInputInfo(
        MidiControllerType ctrlType, int inputPort,
        float inputNativeValueMin, float inputNativeValueMax)
    {
        ModInputInfo* info = new ModInputInfo();
        info->inputType = ModInputType::MidiController;
        info->inputPreset = ModInputPreset::None;
        info->ctrlType = ctrlType;
        info->inputPort = inputPort;
        info->inputNativeValueMin = inputNativeValueMin;
        info->inputNativeValueMax = inputNativeValueMax;
        info->outputModulator = this;
        inputInfos->push_back(info);
        usePortCount[(int)inputPort]++;
    }


    //移除指定端口上的所有输入信息
    //<param name = "port">指定端口< / param>
    void Modulator::RemoveAllInputInfoFromPort(int port)
    {
        ModInputInfo* inputInfo = nullptr;
        ModInputInfoList::iterator it = inputInfos->begin();
        ModInputInfoList::iterator end = inputInfos->end();
        for (; it != end;)
        {
            inputInfo = *it;
            if (inputInfo != nullptr &&
                inputInfo->inputPort == port)
            {
                if (inputInfo->inputType == ModInputType::Modulator &&
                    inputInfo->inputModulator != nullptr)
                {
                    inputInfo->inputModulator->SetOutTarget(nullptr, 0);
                }
                else
                {
                    usePortCount[(int)inputInfo->inputPort]--;
                }

                DEL(inputInfo);
                it = inputInfos->erase(it);
            }
            else
            {
                it++;
            }
        }

        usePortCount[(int)port] = 0;
    }


    //设置输出目标到生成器类型
    void Modulator::SetOutTarget(GeneratorType generatorType)
    {
        if (outTargetModulator != nullptr)
            outTargetModulator->RemoveInputModInfo(this);

        outTargetModulator = nullptr;
        outTargetGeneratorType = generatorType;
    }


    //设置输出目标为另一个调制器targetMod的port
    void Modulator::SetOutTarget(Modulator* targetMod, int port)
    {
        if (targetMod == outTargetModulator)
            return;

        if (outTargetModulator != nullptr)
            outTargetModulator->RemoveInputModInfo(this);

        outTargetModulator = targetMod;
        outTargetModulatorPort = port;
        outTargetGeneratorType = GeneratorType::None;

        if (targetMod == nullptr)
            return;

        targetMod->RemoveAllCommonInputInfoFromPort(outTargetModulatorPort);
        targetMod->AddInputModulator(this, port);

    }

    //设置控制器调制器输入信息到列表
    void Modulator::SetCtrlModulatorInputInfo(
        MidiControllerType ctrlType,
        int inputPort, float inputNativeValueMin, float inputNativeValueMax)
    {
        RemoveAllInputInfoFromPort(inputPort);

        AddInputInfo(
            ModInputType::MidiController,
            ModInputPreset::None, ctrlType,
            inputPort,
            inputNativeValueMin,
            inputNativeValueMax);
    }

    // 设置一个已有的调制器输入信息到列表
    void Modulator::SetModulatorInputInfo(
        ModInputType inputType, ModInputPreset inputPreset, MidiControllerType ctrlType,
        int inputPort, float inputNativeValueMin, float inputNativeValueMax)
    {
        RemoveAllInputInfoFromPort(inputPort);
        AddInputInfo(inputType, inputPreset, ctrlType, inputPort, inputNativeValueMin, inputNativeValueMax);
    }

    //往调制器端口输入值
    void Modulator::Input(int port, float value)
    {
        (*inputInfos)[(int)port]->inputNativeValue = value;
        Modulator* lastModualtor = GetLastOutTargetModulator();
        lastModualtor->ioState = ModIOState::Inputed;
    }

    //计算输出值
    float Modulator::Output()
    {
        float mapValue[2] = { 0, 0 };
        float inputValue[2] = { 0, 0 };
        float inputMinValue[2] = { 0, 0 };
        float inputMaxValue[2] = { 0, 0 };
        int port;
        size_t size = inputInfos->size();
        for (int i = 0; i < size; i++)
        {
            port = (int)(*inputInfos)[i]->inputPort;
            if ((*inputInfos)[i]->inputType == ModInputType::Modulator)
            {
                inputValue[port] = (*inputInfos)[i]->inputModulator->Output();
                RangeFloat range = (*inputInfos)[i]->inputModulator->CalOutputRange();
                inputMinValue[port] = range.min;
                inputMaxValue[port] = range.max;
            }
            else
            {
                inputValue[port] = (*inputInfos)[i]->inputNativeValue;
                inputMinValue[port] = (*inputInfos)[i]->inputNativeValueMin;
                inputMaxValue[port] = (*inputInfos)[i]->inputNativeValueMax;
            }

            if (sourceTransTypes[port] != ModSourceTransformType::Func)
                mapValue[port] += MapValue(inputValue[port], inputMinValue[port], inputMaxValue[port], sourceTransTypes[port], dir[port], polar[port]);
            else
                mapValue[port] += MapValue(inputValue[port], inputMinValue[port], inputMaxValue[port], modTransformFunc[port], inValueRange[port]);
        }

        if (usePortCount[0] == 0) { mapValue[0] = 1; }
        if (usePortCount[1] == 0) { mapValue[1] = 1; }

        outputValue = amount * mapValue[0] * mapValue[1];

        if (absType == ModTransformType::Absolute)
            outputValue = abs(outputValue);

        ioState = ModIOState::Ouputed;
        return outputValue;
    }



    //计算输出值的范围
    RangeFloat Modulator::CalOutputRange()
    {
        RangeFloat range0 = outValueRange[0];
        RangeFloat range1 = outValueRange[1];

        if (usePortCount[0] == 0)
        {
            range0.min = 1;
            range0.max = 1;
        }
        if (usePortCount[1] == 0)
        {
            range1.min = 1;
            range1.max = 1;
        }

        float minval = min(range0.min, range1.min);
        float maxval = max(range0.max, range1.max);

        outputValueMin = amount * minval;
        outputValueMax = amount * maxval;

        return GetOutputRange();
    }

    //获取最终输出目标生成器类型
    GeneratorType Modulator::GetLastOutTargetGeneratorType()
    {
        if (outTargetModulator != nullptr && outTargetGeneratorType == GeneratorType::None)
            return outTargetModulator->GetLastOutTargetGeneratorType();
   
        return outTargetGeneratorType;
    }

    //获取最终输出目标Modulator
    Modulator* Modulator::GetLastOutTargetModulator()
    {
        if (outTargetModulator != nullptr && outTargetGeneratorType == GeneratorType::None)
            return outTargetModulator->GetLastOutTargetModulator();

        return this;
    }


    //在调制器上增加一个输入的调制器inputMod到指定端口inputPort
    void Modulator::AddInputModulator(Modulator* inputMod, int inputPort)
    {
        ModInputInfo* info = new ModInputInfo();
        info->inputType = ModInputType::Modulator;
        info->inputPort = inputPort;
        info->inputModulator = inputMod;
        info->outputModulator = this;
        usePortCount[(int)inputPort]++;
        inputInfos->push_back(info);
    }

    //移除调制器上的某个端口上的输入调制器inputMod，并返回移除的输入信息
    //<param name = "inputMod">输入到端口的调制器< / param>
    void Modulator::RemoveInputModInfo(Modulator* inputMod)
    {
        ModInputInfo* modInputInfo = nullptr;
        ModInputInfoList::iterator it = inputInfos->begin();
        for (; it != inputInfos->end(); )
        {
            modInputInfo = *it;

            if (modInputInfo != nullptr &&
                modInputInfo->inputType == ModInputType::Modulator &&
                modInputInfo->inputModulator == inputMod)
            {
                usePortCount[(int)modInputInfo->inputPort]--;
                it = inputInfos->erase(it);
                DEL(modInputInfo);
            }
            else
                it++;
        }
    }

    //移除端口上的所有非输入调制器的输入信息，并返回移除的输入信息组
    void Modulator::RemoveAllCommonInputInfoFromPort(int port)
    {
        ModInputInfoList::iterator it = inputInfos->begin();
        for (; it != inputInfos->end(); )
        {
            if ((*it)->inputType != ModInputType::Modulator &&
                (*it)->inputPort == port)
            {
                DEL(*it);
                usePortCount[(int)(*it)->inputPort]--;
                it = inputInfos->erase(it);
            }
            else
            {
                it++;
            }
        }
    }


    /// <summary>
    /// 映射输入值到控制类型值域范围
    /// </summary>
    /// <param name="inValue">输入值</param>
    /// <param name="inValueMin">输入值的最小值限制</param>
    /// <param name="inValueMax">输入值的最大值限制</param>
    /// <param name="tranformFunc">变换自定义方法</param>
    /// <param name="funcInRange">输入数值范围</param>
    /// <returns></returns>
    float Modulator::MapValue(float inValue, float inValueMin, float inValueMax, ModTransformCallBack tranformFunc, RangeFloat funcInRange)
    {
        if (inValueMin - inValueMax == 0)
            return 1;

        float mapNormalInputValue = MappingToNormalValue(inValue, inValueMin, inValueMax, funcInRange.min, funcInRange.max);
        float mapValue = tranformFunc(mapNormalInputValue);
        return mapValue;
    }

    /// <summary>
    /// 映射输入值到控制类型值域范围
    /// </summary>
    /// <param name="inValue">输入值</param>
    /// <param name="inValueMin">输入值的最小值限制</param>
    /// <param name="inValueMax">输入值的最大值限制</param>
    /// <param name="sourceType">变换类型</param>
    /// <param name="dir">变换线方向 0: Positive minY->maxY, 1: Negative maxY->minY</param>
    /// <param name="polar">变换线极性  0: Unipolar y值范围: 0->1, 1: Bipolar  y值范围: -1->1</param>
    /// <returns>映射值</returns>
    float Modulator::MapValue(float inValue, float inValueMin, float inValueMax, ModSourceTransformType sourceTransType, int dir, int polar)
    {
        if (inValueMin - inValueMax == 0)
            return 1;

        float mapNormalInputValue;
        float mapValue = 0;
        RangeFloat mapValueRange = GetMapValueRange(polar);
        mapNormalInputValue = MappingToNormalValue(inValue, inValueMin, inValueMax, mapValueRange.min, mapValueRange.max);


        switch (sourceTransType)
        {
        case ModSourceTransformType::Linear:
            if (dir == 0)
            {
                if (polar == 0) { mapValue = LinearPositiveBipolar(mapNormalInputValue); }
                else { mapValue = LinearPositiveUnipolar(mapNormalInputValue); }
            }
            else if (dir == 1)
            {
                if (polar == 0) { mapValue = LinearNegativeBipolar(mapNormalInputValue); }
                else { mapValue = LinearNegativeUnipolar(mapNormalInputValue); }
            }
            break;

        case ModSourceTransformType::Convex:
            if (dir == 0)
            {
                if (polar == 0) { mapValue = ConvexPositiveBipolar(mapNormalInputValue); }
                else { mapValue = ConvexPositiveUnipolar(mapNormalInputValue); }
            }
            else if (dir == 1)
            {
                if (polar == 0) { mapValue = ConvexNegativeBipolar(mapNormalInputValue); }
                else { mapValue = ConvexNegativeUnipolar(mapNormalInputValue); }
            }
            break;

        case ModSourceTransformType::Concave:
            if (dir == 0)
            {
                if (polar == 0) { mapValue = ConcavePositiveBipolar(mapNormalInputValue); }
                else { mapValue = ConcavePositiveUnipolar(mapNormalInputValue); }
            }
            else if (dir == 1)
            {
                if (polar == 0) { mapValue = ConcaveNegativeBipolar(mapNormalInputValue); }
                else { mapValue = ConcaveNegativeUnipolar(mapNormalInputValue); }
            }
            break;


        case ModSourceTransformType::Switch:
            if (dir == 0)
            {
                if (polar == 0) { mapValue = SwitchPositiveBipolar(mapNormalInputValue); }
                else { mapValue = SwitchPositiveUnipolar(mapNormalInputValue); }
            }
            else if (dir == 1)
            {
                if (polar == 0) { mapValue = SwitchNegativeBipolar(mapNormalInputValue); }
                else { mapValue = SwitchNegativeUnipolar(mapNormalInputValue); }
            }
            break;
        }

        return mapValue;
    }


    //Concave Controller Curves
    //Concave
    //x : [0, 1]
    //y : [0, 1]
    float Modulator::ConcavePositiveUnipolar(float x)
    {
        Vec2 v = SquareBezier(x, Vec2(0, 0), Vec2(1, 0), Vec2(1, 1));
        return (float)v.y;
    }


    //Concave
    //x : [0, 1]
    //y : [1, 0]
    float Modulator::ConcaveNegativeUnipolar(float x)
    {
        Vec2 v = SquareBezier(x, Vec2(0, 1), Vec2(0, 0), Vec2(1, 0));
        return (float)v.y;
    }


    //Concave
    //x : [-1, 1]
    //y : [-1, 1]
    float Modulator::ConcavePositiveBipolar(float x)
    {
        Vec2 v;
        if (x < 0)
        {
            x = x + 1;
            v = SquareBezier(x, Vec2(-1, -1), Vec2(-1, 0), Vec2(0, 0));
        }
        else
        {
            v = SquareBezier(x, Vec2(0, 0), Vec2(1, 0), Vec2(1, 1));
        }

        return (float)v.y;
    }


    //Concave
    //x : [-1, 1]
    //y : [1, -1]
    float Modulator::ConcaveNegativeBipolar(float x)
    {
        Vec2 v;
        if (x < 0)
        {
            x = x + 1;
            v = SquareBezier(x, Vec2(-1, 1), Vec2(-1, 0), Vec2(0, 0));
        }
        else
        {
            v = SquareBezier(x, Vec2(0, 0), Vec2(1, 0), Vec2(1, -1));
        }

        return (float)v.y;
    }

    //Convex Controller Curves
    //Convex
    //x : [0, 1]
    //y : [0, 1]
    float Modulator::ConvexPositiveUnipolar(float x)
    {
        Vec2 v = SquareBezier(x, Vec2(0, 0), Vec2(0, 1), Vec2(1, 1));
        return (float)v.y;
    }

    //Convex
    //x : [0, 1]
    //y : [1, 0]
    float Modulator::ConvexNegativeUnipolar(float x)
    {
        Vec2 v = SquareBezier(x, Vec2(0, 1), Vec2(1, 1), Vec2(1, 0));
        return (float)v.y;
    }



    //Convex
    //x : [-1, 1]
    //y : [-1, 1]
    float Modulator::ConvexPositiveBipolar(float x)
    {
        Vec2 v;
        if (x < 0)
        {
            x = x + 1;
            v = SquareBezier(x, Vec2(-1, -1), Vec2(0, -1), Vec2(0, 0));
        }
        else
        {
            v = SquareBezier(x, Vec2(0, 0), Vec2(0, 1), Vec2(1, 1));
        }

        return (float)v.y;
    }


    //Convex
    //x : [-1, 1]
    //y : [1, -1]
    float Modulator::ConvexNegativeBipolar(float x)
    {
        Vec2 v;
        if (x < 0)
        {
            x = x + 1;
            v = SquareBezier(x, Vec2(-1, 1), Vec2(0, 1), Vec2(0, 0));
        }
        else
        {
            v = SquareBezier(x, Vec2(0, 0), Vec2(0, -1), Vec2(1, -1));
        }

        return (float)v.y;
    }
}