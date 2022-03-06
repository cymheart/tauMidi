#include"Synther.h"
#include"Synth/VirInstrument.h"
#include"Synth/RegionSounderThread.h"
#include"Synth/Channel.h"
#include"Synth/Preset.h"
#include"Synth/SyntherEvent.h"
#include"Synth/MidiTrackRecord.h"

namespace tau
{

	void Synther::CombineSynthersFrameBufsTask()
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _CombineSynthersFrameBufsTask;
		PostTask(ev);
	}

	void Synther::_CombineSynthersFrameBufsTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther._CombineSynthersFrameBufs();
	}


	void Synther::AddAssistSyntherTask(Semaphore* waitSem, Synther* assistSynther)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _AddAssistSyntherTask;
		ev->ptr = assistSynther;
		ev->sem = waitSem;
		PostTask(ev);
	}

	void Synther::_AddAssistSyntherTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.AddAssistSynther((Synther*)se->ptr);
		se->sem->set();
	}

	void Synther::RemoveAssistSyntherTask(Synther* assistSynther)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _RemoveAssistSyntherTask;
		ev->ptr = assistSynther;
		PostTask(ev);
	}

	void Synther::_RemoveAssistSyntherTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.RemoveAssistSynther((Synther*)se->ptr);
	}

	// 请求删除合成器
	void Synther::ReqDeleteTask()
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _ReqDeleteTaskTask;
		PostTask(ev);
	}

	void Synther::_ReqDeleteTaskTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.ReqDelete();
	}

	//添加替换乐器
	void Synther::AppendReplaceInstrumentTask(
		int orgBankMSB, int orgBankLSB, int orgInstNum,
		int repBankMSB, int repBankLSB, int repInstNum)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _AppendReplaceInstrumentTask;
		ev->exValue[0] = (float)orgBankMSB;
		ev->exValue[1] = (float)orgBankLSB;
		ev->exValue[2] = (float)orgInstNum;
		ev->exValue[3] = (float)repBankMSB;
		ev->exValue[4] = (float)repBankLSB;
		ev->exValue[5] = (float)repInstNum;
		PostTask(ev);
	}

	void Synther::_AppendReplaceInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.AppendReplaceInstrument(
			(int)se->exValue[0], (int)se->exValue[1], (int)se->exValue[2],
			(int)se->exValue[3], (int)se->exValue[4], (int)se->exValue[5]);
	}


	//移除替换乐器
	void Synther::RemoveReplaceInstrumentTask(int orgBankMSB, int orgBankLSB, int orgInstNum)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _RemoveReplaceInstrumentTask;
		ev->exValue[0] = (float)orgBankMSB;
		ev->exValue[1] = (float)orgBankLSB;
		ev->exValue[2] = (float)orgInstNum;
		PostTask(ev);
	}

	void Synther::_RemoveReplaceInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.RemoveReplaceInstrument((int)se->exValue[0], (int)se->exValue[1], (int)se->exValue[2]);
	}


	//增加效果器
	void Synther::AddEffectTask(TauEffect* effect)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _AddEffectTask;
		ev->ptr = effect;
		PostTask(ev);
	}

	void Synther::_AddEffectTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.AddEffect((TauEffect*)se->ptr);
	}

	//设置是否开启所有乐器效果器
	void Synther::SetEnableAllVirInstEffectsTask(bool isEnable)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetEnableAllVirInstEffectsTask;
		ev->value = isEnable;
		PostTask(ev);
	}

	void Synther::_SetEnableAllVirInstEffectsTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.SetEnableAllVirInstEffects(se->value);

	}



	// 设置乐器Bend值
	void Synther::SetVirInstrumentPitchBendTask(VirInstrument* virInst, int value)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetVirInstrumentPitchBendTask;
		ev->value = value;
		ev->ptr = virInst;
		PostTask(ev);
	}

	void Synther::_SetVirInstrumentPitchBendTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.SetVirInstrumentPitchBend((VirInstrument*)se->ptr, se->value);
	}

	// 设置乐器PolyPressure值
	void Synther::SetVirInstrumentPolyPressureTask(VirInstrument* virInst, int key, int pressure)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetVirInstrumentPolyPressureTask;
		ev->value = key;
		ev->exValue[0] = (float)pressure;
		ev->ptr = virInst;
		PostTask(ev);
	}

	void Synther::_SetVirInstrumentPolyPressureTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.SetVirInstrumentPolyPressure((VirInstrument*)se->ptr, se->value, (int)se->exValue[0]);
	}


	// 设置乐器Midi控制器值
	void Synther::SetVirInstrumentMidiControllerValueTask(VirInstrument* virInst, MidiControllerType midiController, int value)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetVirInstrumentlMidiControllerValueTask;
		ev->midiCtrlType = midiController;
		ev->value = value;
		ev->ptr = virInst;
		PostTask(ev);
	}

	void Synther::_SetVirInstrumentlMidiControllerValueTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.SetVirInstrumentMidiControllerValueTask((VirInstrument*)se->ptr, se->midiCtrlType, se->value);
	}

	VirInstrument* Synther::EnableVirInstrumentTask(Channel* channel, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		thread_local VirInstrument* inst = 0;
		thread_local Semaphore waitGetInstrumentSem;

		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _EnableInstrumentTask;
		ev->bankSelectLSB = bankSelectLSB;
		ev->bankSelectMSB = bankSelectMSB;
		ev->instrumentNum = instrumentNum;
		ev->exPtr[0] = channel;
		ev->ptr = (void*)&inst;
		ev->sem = &waitGetInstrumentSem;
		PostTask(ev);

		//
		waitGetInstrumentSem.wait();
		return inst;
	}

	void Synther::_EnableInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		VirInstrument* vinst =
			synther.EnableVirInstrument(
				(Channel*)se->exPtr[0],
				se->bankSelectMSB,
				se->bankSelectLSB,
				se->instrumentNum);

		VirInstrument** threadVInst = (VirInstrument**)se->ptr;
		*threadVInst = vinst;
		se->sem->set();
	}

	void Synther::SetVirInstrumentProgramTask(VirInstrument* virInst, int bankSelectMSB, int bankSelectLSB, int instrumentNum)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _SetVirInstrumentProgramTask;
		ev->bankSelectLSB = bankSelectLSB;
		ev->bankSelectMSB = bankSelectMSB;
		ev->instrumentNum = instrumentNum;
		ev->exPtr[0] = virInst;
		PostTask(ev);
	}

	void Synther::_SetVirInstrumentProgramTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);

		synther.SetVirInstrumentProgram(
			(VirInstrument*)se->exPtr[0],
			se->bankSelectMSB,
			se->bankSelectLSB,
			se->instrumentNum);
	}

	// 移除乐器
	void Synther::RemoveVirInstrumentTask(VirInstrument* virInst, bool isFade)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _RemoveInstrumentTask;
		ev->ptr = (void*)virInst;
		ev->value = isFade ? 1 : 0;
		PostTask(ev);
	}

	void Synther::_RemoveInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		VirInstrument* virInst = (VirInstrument*)se->ptr;
		synther.RemoveVirInstrument(virInst, se->value);
	}

	// 移除所有乐器
	void Synther::RemoveAllVirInstrumentTask(bool isFade)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _RemoveAllInstrumentTask;
		ev->value = isFade ? 1 : 0;
		PostTask(ev);
	}

	void Synther::_RemoveAllInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.RemoveAllVirInstrument(se->value);
	}

	// 删除乐器
	void Synther::DelVirInstrumentTask(VirInstrument* virInst)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _DelInstrumentTask;
		ev->ptr = (void*)virInst;
		PostTask(ev);
	}

	void Synther::_DelInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		VirInstrument* virInst = (VirInstrument*)se->ptr;
		synther.DelVirInstrument(virInst);
	}


	// 打开虚拟乐器
	void Synther::OnVirInstrumentTask(VirInstrument* virInst, bool isFade)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _OnInstrumentTask;
		ev->ptr = (void*)virInst;
		ev->value = isFade ? 1 : 0;
		PostTask(ev);
	}

	void Synther::_OnInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		VirInstrument* virInst = (VirInstrument*)se->ptr;
		synther.OnVirInstrument(virInst, se->value);
	}

	// 关闭虚拟乐器
	void Synther::OffVirInstrumentTask(VirInstrument* virInst, bool isFade)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _OffInstrumentTask;
		ev->ptr = (void*)virInst;
		ev->value = isFade ? 1 : 0;
		PostTask(ev);
	}

	void Synther::_OffInstrumentTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		VirInstrument* virInst = (VirInstrument*)se->ptr;
		synther.OffVirInstrument(virInst, se->value);
	}


	/// <summary>
	/// 获取虚拟乐器列表的备份
	/// </summary>
	vector<VirInstrument*>* Synther::TakeVirInstrumentListTask()
	{
		thread_local vector<VirInstrument*>* insts = 0;
		thread_local Semaphore waitGetInstsSem;

		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _TakeVirInstrumentListTask;
		ev->ptr = (void*)&insts;
		ev->sem = &waitGetInstsSem;

		PostTask(ev);

		waitGetInstsSem.wait();
		return insts;

	}

	void Synther::_TakeVirInstrumentListTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		vector<VirInstrument*>** threadVInsts = (vector<VirInstrument*>**)se->ptr;
		*threadVInsts = synther.TakeVirInstrumentList();
		se->sem->set();
	}

	// 录制所有乐器弹奏为midi
	void Synther::RecordMidiTask()
	{
		RecordMidiTask(nullptr);
	}

	/// <summary>
	/// 录制指定乐器弹奏为midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	void Synther::RecordMidiTask(VirInstrument* virInst)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->virInst = virInst;
		ev->processCallBack = _RecordMidiTask;
		PostTask(ev);
	}

	void Synther::_RecordMidiTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.RecordMidi(se->virInst);
	}

	/// <summary>
	/// 停止所有乐器当前midi录制
	/// </summary>
	void Synther::StopRecordMidiTask()
	{
		StopRecordMidiTask(nullptr);
	}

	/// <summary>
	/// 停止录制指定乐器弹奏midi
	/// </summary>
	/// <param name="virInst">如果为null,将录制所有乐器</param>
	void Synther::StopRecordMidiTask(VirInstrument* virInst)
	{
		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->virInst = virInst;
		ev->processCallBack = _StopRecordMidiTask;
		PostTask(ev);
	}


	void Synther::_StopRecordMidiTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		synther.StopRecordMidi(se->virInst);
	}

	// 获取录制的midi轨道
	vector<MidiTrack*>* Synther::TakeRecordMidiTracksTask(
		float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos)
	{
		return TakeRecordMidiTracksTask(nullptr, 0, recordMidiTickForQuarterNote, tempos);
	}


	// 获取录制的midi轨道
	vector<MidiTrack*>* Synther::TakeRecordMidiTracksTask(
		VirInstrument** virInst, int size, float recordMidiTickForQuarterNote, vector<RecordTempo>* tempos)
	{
		thread_local vector<MidiTrack*>* tracks = 0;
		thread_local Semaphore waitSem;

		SyntherEvent* ev = SyntherEvent::New();
		ev->synther = this;
		ev->processCallBack = _TakeRecordMidiTracksTask;
		ev->ptr = (void*)&tracks;
		ev->exPtr[0] = virInst;
		ev->exValue[0] = (float)size;
		ev->exValue[1] = recordMidiTickForQuarterNote;
		ev->exPtr[1] = tempos;
		ev->sem = &waitSem;

		PostTask(ev);

		waitSem.wait();
		return tracks;

	}

	void Synther::_TakeRecordMidiTracksTask(Task* ev)
	{
		SyntherEvent* se = (SyntherEvent*)ev;
		Synther& synther = (Synther&)*(se->synther);
		vector<MidiTrack*>** threadTracks = (vector<MidiTrack*>**)se->ptr;

		*threadTracks = synther.TakeRecordMidiTracks(
			(VirInstrument**)(se->exPtr[0]), (int)se->exValue[0], se->exValue[1], (vector<RecordTempo>*)(se->exPtr[1]));

		se->sem->set();
	}


}
