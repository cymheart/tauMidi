#include"Tau.h"
#include"Synther/Synther.h"
#include"SoundFont.h"
#include <dsignal/MorphLBHFilter.h>
#include <dsignal/Bode.h>
#include "Editor/Editor.h"
#include"FX/TauEffect.h"
#include"FX/EffectList.h"

using namespace dsignal;

/*
* Tau是一个soundfont合成器
* by cymheart, 2020--2023.
*/
namespace tau {

	Tau::Tau()
	{
#ifdef _WIN32
		timeBeginPeriod(1);
#endif
		presetBankReplaceMap = new unordered_map<uint32_t, uint32_t>;

		mainSynther = new Synther(this);
		editor = new Editor(this);
		mainSynther->CreateMidiEditor();
		editor->SetMidiEditor(mainSynther->midiEditor);


		SetFrameSampleCount(frameSampleCount);
		SetSampleProcessRate(sampleProcessRate);

	}

	Tau::~Tau()
	{

#ifdef _WIN32
		timeEndPeriod(1);
#endif

		DEL(editor);
		DEL(mainSynther);
		DEL(presetBankReplaceMap);

	}

	//打开
	void Tau::Open()
	{
		if (isOpened)
			return;

		mainSynther->Open();
		isOpened = true;
	}

	//关闭
	void Tau::Close()
	{
		if (!isOpened)
			return;

		Remove();

		mainSynther->Close();
		isOpened = false;
	}


	// 设置样本处理采样率
	void Tau::SetSampleProcessRate(int rate)
	{
		if (isOpened)
			return;

		sampleProcessRate = (float)rate;
		invSampleProcessRate = 1.0 / (double)sampleProcessRate;
		stk::Stk::setSampleRate(44100);

		unitSampleSec = invSampleProcessRate * childFrameSampleCount;
	}


	//设置帧样本数量
	//这个值越小，声音的实时性越高（在实时演奏时，值最好在1024以下，最合适的值为512）,
	//当这个值比较小时，cpu内耗增加
	void Tau::SetFrameSampleCount(int count)
	{
		if (isOpened)
			return;

		frameSampleCount = count;
		if (frameSampleCount < 256) frameSampleCount = 256;
		if (childFrameSampleCount > frameSampleCount) childFrameSampleCount = frameSampleCount;
		else if (childFrameSampleCount < 1)childFrameSampleCount = 1;

		unitSampleSec = invSampleProcessRate * childFrameSampleCount;

		//
		mainSynther->SetFrameSampleCount(count);
	}

	//设置是否使用多线程
	//使用多线程渲染处理声音
	//多线程渲染在childFrameSampleCount比较小的情况下(比如小于64时)，由于在一大帧中线程调用太过频繁，线程切换的消耗大于声音渲染的时间
	//当childFrameSampleCount >= 256时，多线程效率会比较高
	//在播放midi音乐时，并不适合开启多线程，因为midi播放事件要求一个非常小的childFrameSampleCount值
	//但在测试同时发音数量很高的midi音乐时，多线程的效率非常高，播放也稳定
	void Tau::SetUseMulThread(bool use)
	{
		if (isOpened)
			return;

		useMulThreads = use;

		//
		mainSynther->SetUseMulThread(useMulThreads);
	}



	// 获取乐器预设
	Preset* Tau::GetInstrumentPreset(int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		if (soundFont == nullptr)
			return nullptr;

		return soundFont->GetInstrumentPreset(bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	// 获取乐器预设
	Preset* Tau::GetInstrumentPreset(int key)
	{
		if (soundFont == nullptr)
			return nullptr;

		return soundFont->GetInstrumentPreset(key);
	}


	//增加效果器
	void Tau::AddEffect(TauEffect* effect)
	{
		TauLock(this);
		mainSynther->AddEffect(effect);
	}

	// 按下按键
	void Tau::OnKey(int key, float velocity, VirInstrument* virInst, int id)
	{
		TauLock(this);
		mainSynther->OnKey(key, velocity, virInst, id);
	}

	// 释放按键 
	void Tau::OffKey(int key, float velocity, VirInstrument* virInst, int id)
	{
		TauLock(this);
		mainSynther->OffKey(key, velocity, virInst, id);
	}


	//添加替换乐器
	void Tau::AppendReplaceInstrument(
		int orgBankMSB, int orgBankLSB, int orgInstNum,
		int repBankMSB, int repBankLSB, int repInstNum)
	{
		int orgKey = orgBankMSB << 16 | orgBankLSB << 8 | orgInstNum;
		int repKey = repBankMSB << 16 | repBankLSB << 8 | repInstNum;
		(*presetBankReplaceMap)[orgKey] = repKey;

		//
		TauLock(this);
		mainSynther->AppendReplaceInstrument(
			orgBankMSB, orgBankLSB, orgInstNum,
			repBankMSB, repBankLSB, repInstNum);
	}


	//移除替换乐器
	void Tau::RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum)
	{
		int orgKey = orgBankMSB << 16 | orgBankLSB << 8 | orgInstNum;
		presetBankReplaceMap->erase(orgKey);

		//
		TauLock(this);
		mainSynther->RemoveReplaceInstrument(orgBankMSB, orgBankLSB, orgInstNum);
	}


	// 设置乐器Bend值
	void Tau::SetVirInstrumentPitchBend(VirInstrument* virInst, int value)
	{
		TauLock(this);
		mainSynther->SetVirInstrumentPitchBend(virInst, value);
	}

	// 设置乐器按键压力值
	void Tau::SetVirInstrumentPolyPressure(VirInstrument* virInst, int key, int pressure)
	{
		TauLock(this);
		mainSynther->SetVirInstrumentPolyPressure(virInst, key, pressure);
	}


	// 设置乐器Midi控制器值
	void Tau::SetVirInstrumentMidiControllerValue(VirInstrument* virInst, MidiControllerType midiController, int value)
	{
		TauLock(this);
		mainSynther->SetVirInstrumentMidiControllerValue(virInst, midiController, value);
	}

	//设置是否开启所有乐器效果器
	void Tau::SetEnableAllVirInstEffects(bool isEnable)
	{
		if (isOpened)
			return;

		isEnableVirInstEffects = isEnable;
		TauLock(this);
		mainSynther->SetEnableAllVirInstEffects(isEnable);
	}

	// 设置虚拟乐器值
	void Tau::SetVirInstrumentProgram(VirInstrument* virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		TauLock(this);
		mainSynther->SetVirInstrumentProgram(virInst, bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	/// <summary>
	/// 在虚拟乐器列表中，启用指定的虚拟乐器,如果不存在，将在虚拟乐器列表中自动创建它
	/// 注意如果deviceChannelNum已经被使用过，此时会直接修改这个通道上的虚拟乐器的音色到指定音色，
	/// 而不会同时在一个通道上创建超过1个的虚拟乐器
	/// </summary>
	/// <param name="deviceChannel">乐器所在的设备通道</param>
	/// <param name="bankSelectMSB">声音库选择0</param>
	/// <param name="bankSelectLSB">声音库选择1</param>
	/// <param name="instrumentNum">乐器编号</param>
	/// <returns></returns>
	VirInstrument* Tau::EnableVirInstrument(int deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		TauLock(this);
		return mainSynther->EnableVirInstrument(deviceChannelNum, bankSelectMSB, bankSelectLSB, instrumentNum);
	}

	/// <summary>
	/// 移除乐器
	/// </summary>
	void Tau::RemoveVirInstrument(VirInstrument* virInst, bool isFade)
	{
		TauLock(this);
		mainSynther->RemoveVirInstrument(virInst, isFade);
	}

	/// <summary>
	/// 移除所有乐器
	/// </summary>
	void Tau::RemoveAllVirInstrument(bool isFade)
	{
		TauLock(this);
		mainSynther->RemoveAllVirInstrument(isFade);
	}

	/// <summary>
	/// 打开乐器
	/// </summary>
	void Tau::OpenVirInstrument(VirInstrument* virInst, bool isFade)
	{
		TauLock(this);
		mainSynther->OpenVirInstrument(virInst, isFade);
	}

	/// <summary>
	/// 关闭虚拟乐器
	/// </summary>
	void Tau::CloseVirInstrument(VirInstrument* virInst, bool isFade)
	{
		TauLock(this);
		mainSynther->CloseVirInstrument(virInst, isFade);
	}

	/// <summary>
	/// 获取虚拟乐器列表的备份
	/// </summary>
	vector<VirInstrument*>* Tau::TakeVirInstrumentList()
	{
		TauLock(this);
		vector<VirInstrument*>* virInsts = new vector<VirInstrument*>();
		vector<VirInstrument*>* curtVirInsts = mainSynther->TakeVirInstrumentList();
		if (curtVirInsts == nullptr)
			return nullptr;

		for (int j = 0; j < curtVirInsts->size(); j++)
			virInsts->push_back((*curtVirInsts)[j]);

		DEL(curtVirInsts);
		return virInsts;
	}

}
