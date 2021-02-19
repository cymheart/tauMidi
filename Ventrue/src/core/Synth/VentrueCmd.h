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
	public:
		VentrueCmd(Ventrue* ventrue);

		void ProcessTask(TaskCallBack taskCallBack, void* data, int delay = 0);

		// 按下按键
		void OnKey(int key, float velocity, VirInstrument* virInst);
		// 释放按键 
		void OffKey(int key, float velocity, VirInstrument* virInst);

		//增加效果器
		void AddEffect(VentrueEffect* effect);

		//发送文本
		void SendText(string text);

		//增加并播放midi文件
		void PlayMidiFile(string midifile);

		// 播放指定编号的内部Midi文件
		void PlayMidi(int midiFileIdx);

		void PlayMidi(MidiFile* midiFile);

		//添加midi文件
		void AppendMidiFile(string midifile);

		// 禁止播放指定编号Midi文件的轨道
		void DisableMidiTrack(int midiFileIdx, int trackIdx);
		// 禁止播放Midi的所有轨道
		void DisableAllMidiTrack(int midiFileIdx);

		// 启用播放指定编号Midi文件的轨道
		void EnableMidiTrack(int midiFileIdx, int trackIdx);
		// 启用播放Midi的所有轨道
		void EnableAllMidiTrack(int midiFileIdx);

		// 指定midi文件播放的起始时间点
		void MidiGotoSec(int midiFileIdx, float sec);

		// 设置设备通道Midi控制器值
		void SetDeviceChannelMidiControllerValue(int deviceChannelNum, MidiControllerType midiController, int value);

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
		static void _OnKey(Task* ev);
		static void _OffKey(Task* ev);
		static void _AddEffect(Task* ev);
		static void _PlayMidi(Task* ev);
		static void _AppendMidi(Task* ev);
		static void _SendText(Task* ev);
		static void _DisableMidiTrack(Task* ev);
		static void _EnableMidiTrack(Task* ev);
		static void _MidiGotoSec(Task* ev);
		static void _SetDeviceChannelMidiControllerValue(Task* ev);
		static void _EnableInstrument(Task* ev);
		static void _RecordMidi(Task* ev);
		static void _StopRecordMidi(Task* ev);
		static void _CreateRecordMidiFileObject(Task* ev);
		static void _SaveMidiFileToDisk(Task* ev);

	private:
		Ventrue* ventrue = nullptr;

	};
}

#endif
