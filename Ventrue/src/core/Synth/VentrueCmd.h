#include <climits>

#ifndef _VentrueCmd_h_
#define _VentrueCmd_h_

#include"VentrueEvent.h"
#include"Midi/MidiFile.h"
#include"MidiPlay.h"
#include"Channel.h"

namespace ventrue
{
	/*
	* Ventrue命令
	* by cymheart, 2020--2021.
	*/
	class DLL_CLASS VentrueCmd
	{
	private:
		VentrueCmd(Ventrue* ventrue);

	public:
		void ProcessTask(TaskCallBack taskCallBack, void* data, int delay = 0);

		//添加替换乐器
		void AppendReplaceInstrument(
			int orgBankMSB, int orgBankLSB, int orgInstNum,
			int repBankMSB, int repBankLSB, int repInstNum);

		//移除替换乐器
		void RemoveReplaceInstrument(int orgBankMSB, int orgBankLSB, int orgInstNum);

		// 按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst);
		// 释放按键 
		void OffKey(int key, float velocity, VirInstrument* virInst);

		//增加效果器
		void AddEffect(VentrueEffect* effect);

		//发送文本
		void SendText(string text);

		// 为midi文件设置打击乐号
		void SetPercussionProgramNum(int midiFileIdx, int num);

		//添加midi文件
		void AppendMidiFile(string midifile);

		//载入midi
		void LoadMidi(int idx);

		//播放midi
		void PlayMidi(int idx);

		//停止播放midi
		void StopMidi(int idx);

		//移除midi
		void RemoveMidi(int idx);

		// 指定midi文件播放的起始时间点
		void MidiGoto(int idx, float sec);

		// 禁止播放指定编号Midi文件的轨道
		void DisableMidiTrack(int midiFileIdx, int trackIdx);
		// 禁止播放Midi的所有轨道
		void DisableAllMidiTrack(int midiFileIdx);

		// 启用播放指定编号Midi文件的轨道
		void EnableMidiTrack(int midiFileIdx, int trackIdx);
		// 启用播放Midi的所有轨道
		void EnableAllMidiTrack(int midiFileIdx);

		// 禁止播放指定编号Midi文件的轨道通道
		void DisableMidiTrackChannel(int midiFileIdx, int trackIdx, int channelIdx);

		// 禁止播放Midi指定轨道上的所有通道
		void DisableMidiTrackAllChannels(int midiFileIdx, int trackIdx);

		// 启用播放指定编号Midi文件的轨道通道
		void EnableMidiTrackChannel(int midiFileIdx, int trackIdx, int channelIdx);

		// 启用播放Midi指定轨道上的所有通道
		void EnableMidiTrackAllChannels(int midiFileIdx, int trackIdx);

		// 设置设备通道Midi控制器值
		void SetDeviceChannelMidiControllerValue(int deviceChannelNum, MidiControllerType midiController, int value);

		// 在虚拟乐器列表中，创建新的指定虚拟乐器
		VirInstrument* NewVirInstrument(int bankSelectMSB, int bankSelectLSB, int instrumentNum);

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
		VirInstrument* EnableVirInstrument(uint32_t deviceChannelNum, int bankSelectMSB, int bankSelectLSB, int instrumentNum);

		/// <summary>
		/// 移除乐器
		/// </summary>
		void RemoveVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 打开乐器
		/// </summary>
		void OnVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 关闭虚拟乐器
		/// </summary>
		void OffVirInstrument(VirInstrument* virInst, bool isFade = true);

		/// <summary>
		/// 获取虚拟乐器列表的备份
		/// </summary>
		vector<VirInstrument*>* TakeVirInstrumentList();

		/// <summary>
		/// 录制所有乐器弹奏为midi
		/// </summary>
		/// <param name="bpm">录制的BPM</param>
		/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
		void RecordMidi(float bpm, float tickForQuarterNote);

		/// <summary>
		/// 录制指定乐器弹奏为midi
		/// </summary>
		/// <param name="virInst">如果为null,将录制所有乐器</param>
		/// <param name="bpm">录制的BPM</param>
		/// <param name="tickForQuarterNote">一个四分音符发音的tick数</param>
		void RecordMidi(VirInstrument* virInst, float bpm = 120.0f, float tickForQuarterNote = 120.0f);

		/// <summary>
		/// 停止所有乐器当前midi录制
		/// </summary>
		void StopRecordMidi();

		/// <summary>
		/// 停止指定乐器当前midi录制
		/// </summary>
		/// <param name="virInst">如果为null,将停止所有乐器的录制</param>
		void StopRecordMidi(VirInstrument* virInst);

		/// <summary>
		/// 生成所有乐器已录制的midi到midiflie object中
		/// </summary>
		/// <returns>midiflie object</returns>
		MidiFile* CreateRecordMidiFileObject();

		/// <summary>        
		/// 根据给定的乐器，生成它的已录制的midi到midiflie object中
		/// </summary>
		/// <param name="virInsts">乐器</param>
		/// <returns>midiflie object</returns>
		MidiFile* CreateRecordMidiFileObject(VirInstrument* virInst);


		/// <summary>
		/// 根据给定的乐器组，生成它们已录制的midi到midiflie object中
		/// </summary>
		/// <param name="virInsts">乐器组</param>
		/// <returns>midiflie object</returns>
		MidiFile* CreateRecordMidiFileObject(VirInstrument** virInsts, int size);


		//保存midiFile到文件
		void SaveMidiFileToDisk(MidiFile* midiFile, string saveFilePath);

	private:
		/// <summary>
		/// 删除乐器
		/// </summary>
		void DelVirInstrument(VirInstrument* virInst);

	private:
		static void _AppendReplaceInstrument(Task* ev);
		static void _RemoveReplaceInstrument(Task* ev);
		static void _OnKey(Task* ev);
		static void _OffKey(Task* ev);
		static void _AddEffect(Task* ev);
		static void _SetPercussionProgramNum(Task* ev);
		static void _AppendMidiFile(Task* ev);
		static void _LoadMidi(Task* ev);
		static void _PlayMidi(Task* ev);
		static void _StopMidi(Task* ev);
		static void _RemoveMidi(Task* ev);
		static void _MidiGoto(Task* ev);
		static void _SendText(Task* ev);
		static void _DisableMidiTrack(Task* ev);
		static void _EnableMidiTrack(Task* ev);
		static void _DisableMidiTrackChannel(Task* ev);
		static void _EnableMidiTrackChannel(Task* ev);
		static void _SetDeviceChannelMidiControllerValue(Task* ev);
		static void _EnableInstrument(Task* ev);
		static void _RemoveInstrument(Task* ev);
		static void _DelInstrument(Task* ev);
		static void _OnInstrument(Task* ev);
		static void _OffInstrument(Task* ev);
		static void _TakeVirInstrumentList(Task* ev);
		static void _RecordMidi(Task* ev);
		static void _StopRecordMidi(Task* ev);
		static void _CreateRecordMidiFileObject(Task* ev);
		static void _SaveMidiFileToDisk(Task* ev);

	private:
		Ventrue* ventrue = nullptr;

		friend class Ventrue;
		friend class VirInstrument;
	};
}

#endif
