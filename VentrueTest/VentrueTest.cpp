
#include <iostream>
#include<string>
#include <stdio.h>
#include"windows.h"
#include<Synth/Ventrue.h>
#include<Synth/VentrueCmd.h>
#include"Synth/VirInstrument.h"
#include <Effect\EffectCmd\EffectReverbCmd.h>
#include <Effect\EffectCmd\EffectEqualizerCmd.h>
#include <Effect\EffectCmd\EffectCompressorCmd.h>
//#include"Effect/EffectReverb.h"

using namespace ventrue;

#ifdef  main
#undef main
#endif 

string GetProgramDir()
{
	wchar_t exeFullPath[MAX_PATH]; // Full path 
	string strPath = "";

	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	char CharString[MAX_PATH];
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, CharString, MAX_PATH, exeFullPath, _TRUNCATE);

	strPath = (string)CharString;    // Get full path of the file 

	int pos = strPath.find_last_of('\\', strPath.length());

	//string path = GetProgramDir();
	pos = strPath.find_last_of("\\", pos - 1);
	pos = strPath.find_last_of("\\", pos - 1);
	strPath = strPath.substr(0, pos);

	return strPath;  // Return the directory without the file name 
}

//弹奏测试
void art(VentrueCmd* cmd)
{
	//选择乐器
	//VirInstrument* inst = cmd->EnableVirInstrument(0, 0, 0, 49);

	/*cmd->SetDeviceChannelMidiControllerValue(0, MidiControllerType::ModulationWheelMSB, 127);
	cmd->OnKey(60, 127, inst);
	Sleep(6300);
	cmd->OffKey(60, 127, inst);*/

	//VirInstrument* inst = cmd->EnableVirInstrument(0, 0, 0, 5);
	//int key[] = { 59,64,49,59,64,59,76,64,59,59,64,59,64, -1 };
	//for (int i = 0; key[i] != -1; i++)
	//{
	//	cmd->OnKey(i, 127, inst);
	//	Sleep(355);
	//	//cmd->OffKey(i, 127, inst);
	//}
	//Sleep(1555);
	//VirInstrument* inst = cmd->EnableVirInstrument(0, 0, 0, 49);

	//for (int i = 0; i < 10; i++) {
	//	cmd->OnKey(43, 127, inst);
	//	Sleep(1555);
	//	cmd->OffKey(43, 127, inst);

	//}
	//cmd->OffKey(64, 127, inst);
	VirInstrument* inst = cmd->EnableVirInstrument(0, 0, 0, 0);
	for (int j = 0; j < 13; j++)
	{


		////在指定乐器上弹奏
		for (int i = 33; i <= 90; i++)
		{
			cmd->OnKey(i, 127, inst);
			Sleep(1455);
			cmd->OffKey(i, 127, inst);
			Sleep(125);
		}
	}
}



int main(int argc, char* argv[])
{
	//资源根路径
	string rootPath = GetProgramDir();
	//midi文件路径
	string midiPath = rootPath + "\\VentrueTest\\midi\\";
	//soundfont音源路径
	string sfPath = rootPath + "\\VentrueTest\\SoundFont\\";


	//建立ventrue
	Ventrue* ventrue = new Ventrue();
	VentrueCmd* cmd = new VentrueCmd(ventrue);


	ventrue->SetFrameSampleCount(256);
	ventrue->SetSampleProcessRate(44100);
	ventrue->SetChildFrameSampleCount(64);
	ventrue->SetLimitRegionSounderCount(600);
	ventrue->SetTrackChannelMergeMode(AutoMerge);
	//ventrue->SetRenderQuality(RenderQuality::SuperHigh);
	//ventrue->SetUseMulThread(true);
	//ventrue->SetChannelOutputMode(ChannelOutputMode::Mono);
	//ventrue->SetUsePortamento(true);
	//ventrue->SetPortaTime(0.3f);
	//ventrue->SetAlwaysUsePortamento(true);
	//ventrue->SetUseLegato(true);
	//ventrue->SetUseMonoMode(true);

	//添加压缩器效果
	//EffectCompressor* compressor = new EffectCompressor();
	//EffectCompressorCmd* compressorCmd = new EffectCompressorCmd(ventrue, compressor);
	//cmd->AddEffect(compressor);

	//添加均衡器效果
	//EffectEqualizer* eq = new EffectEqualizer();
	//auto a = eq->GetFilters();


	//打开音频
	ventrue->OpenAudio();

	//设置音源
	//ventrue->ParseSoundFont("SF2", sfPath + "GeneralUser GS MuseScore v1.442.sf2");
	ventrue->ParseSoundFont("SF2", sfPath + "gnusmas_gm_soundfont_2.01.sf2");
	//ventrue->ParseSoundFont("VentrueFont", sfPath + "OreSoundFont\\GarageBandPianoH\\GarageBandPianoH.xml");

	//添加需要播放的midi文件
	//cmd->AppendMidiFile(midiPath + "venture.mid");
	//cmd->AppendMidiFile(midiPath + "吻别.mid");
	//cmd->AppendMidiFile(midiPath + "dream.mid");
	//cmd->AppendMidiFile(midiPath + "圆周率+TSMB2之作+3.14百万音符.mid");  //（百万音符来袭）
	//cmd->AppendMidiFile(midiPath + "狂妄之人2.mid"); //（音符非常多）
	//cmd->AppendMidiFile(midiPath + "狂妄之人.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Sound of Silence-Simon & Garfunkel.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\QianQianQueGe.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\ShiNian.mid");
	//cmd->AppendMidiFile(midiPath + "(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Brainiac Maniac 2.mid");
	//cmd->AppendMidiFile(midiPath + "song100\\041.mid");
	cmd->AppendMidiFile(midiPath + "song100\\060.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\剑之心(Heart_Of_Sword).mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Promise.mid");
	//cmd->AppendMidiFile(midiPath + "笑傲江湖.mid");
	//cmd->AppendMidiFile(midiPath + "笑傲江湖3.mid");

	//cmd->AppendMidiFile(midiPath + "test.mid");

	//播放第1个midi文件
	cmd->PlayMidi(0);
	//播放第2个midi文件
	//cmd->PlayMidi(1);

	//cmd->DisableMidiTrackChannel(0, 0, 3);

	//cmd->DisableMidiTrackAllChannels(0, 0);
	//cmd->EnableMidiTrackChannel(0, 0, 0);

	//cmd->DisableAllMidiTrack(0);
	//cmd->EnableMidiTrack(0, 4);

	//cmd->DisableMidiTrack(0, 4);
	//cmd->MidiGotoSec(0, 45);


	//去掉注释,弹奏测试
	//art(cmd);

	//
	Sleep(50000000);
	//delete ventrue;


	return 0;
}
