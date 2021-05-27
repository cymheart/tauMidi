
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
	VirInstrument* inst = cmd->EnableVirInstrument(0, 0, 0, 0);

	cmd->OnKey(60, 127, inst);

	//Sleep(2000);
	//cmd->RemoveVirInstrument(inst);


	/*for (int i = 36; i <= 100; i++) {
		cmd->OnKey(i, 127, inst);
		Sleep(45);
		cmd->OffKey(i, 127, inst);

	}*/
	//cmd->OffKey(64, 127, inst);
	//VirInstrument* inst = cmd->EnableVirInstrument(0, 0, 0, 63);
	//for (int j = 0; j < 13; j++)
	//{


	//	////在指定乐器上弹奏
	//	for (int i = 36; i <= 80; i++)
	//	{
	//		cmd->OnKey(i, 127, inst);
	//		Sleep(3355);
	//		cmd->OffKey(i, 127, inst);
	//		Sleep(125);
	//	}
	//}
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
	VentrueCmd* cmd = ventrue->GetCmd();

	ventrue->SetFrameSampleCount(512);
	ventrue->SetSampleProcessRate(44100);
	ventrue->SetChildFrameSampleCount(64);
	ventrue->SetLimitRegionSounderCount(600);
	ventrue->SetTrackChannelMergeMode(AutoMerge);
	//ventrue->SetRenderQuality(RenderQuality::SuperHigh);
//	ventrue->SetUseMulThread(true);
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
	ventrue->ParseSoundFont("SF2", sfPath + "GeneralUser GS MuseScore v1.442.sf2");
	//ventrue->ParseSoundFont("SF2", sfPath + "gnusmas_gm_soundfont_2.01.sf2");
	//ventrue->ParseSoundFont("VentrueFont", sfPath + "OreSoundFont\\GarageBandPianoH\\GarageBandPianoH.xml");

	//添加需要播放的midi文件
	//cmd->AppendMidiFile(midiPath + "venture.mid");
	cmd->AppendMidiFile(midiPath + "吻别.mid");
	//cmd->AppendMidiFile(midiPath + "dream.mid"); 
	//cmd->AppendMidiFile(midiPath + "鱼姐 remix.mid");
	//cmd->AppendMidiFile(midiPath + "决心鱼.mid");
	//cmd->AppendMidiFile(midiPath + "Overwrite remix.mid");
	//cmd->AppendMidiFile(midiPath + "dreamsp.mid");
	//cmd->AppendMidiFile(midiPath + "喜剧之王.mid");
	//cmd->AppendMidiFile(midiPath + "Here-We-Are-Again-喜剧之王插曲.mid");
	//cmd->AppendMidiFile(midiPath + "圆周率+TSMB2之作+3.14百万音符.mid");  //（百万音符来袭）
	//cmd->AppendMidiFile(midiPath + "狂妄之人2.mid"); //（音符非常多）
	//cmd->AppendMidiFile(midiPath + "狂妄之人.mid");
	//cmd->AppendMidiFile(midiPath + "狂妄errow.mid");//xm制
	//cmd->AppendMidiFile(midiPath + "Undertale AU-Ink！Sans-MEGALOVANIA.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Sound of Silence-Simon & Garfunkel.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\QianQianQueGe.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\ShiNian.mid");
	//cmd->AppendMidiFile(midiPath + "(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Brainiac Maniac 2.mid");
	//cmd->AppendMidiFile(midiPath + "song100\\054.mid");
	//cmd->AppendMidiFile(midiPath + "song100\\052.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\剑之心(Heart_Of_Sword).mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Promise.mid");
	//cmd->AppendMidiFile(midiPath + "笑傲江湖.mid");
	//cmd->AppendMidiFile(midiPath + "笑傲江湖3.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\Canon.mid");
	//cmd->AppendMidiFile(midiPath + "横山菁児-英雄たちの夜明け.mid");
	//cmd->AppendMidiFile(midiPath + "英雄的黎明钢琴版.mid");
	//cmd->AppendMidiFile(midiPath + "英雄的黎明(人声UTAU).mid");
	//cmd->AppendMidiFile(midiPath + "Battle Against a True Hero.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\A Time For Us(Romeo and Juliet Theme)-Nino Rota.mid");
	//cmd->AppendMidiFile(midiPath + "learning\\AiQing.mid");
	//cmd->AppendMidiFile(midiPath + "百鸟朝凤.mid");
	//cmd->AppendMidiFile(midiPath + "czh.mid");

	//for (int i = 0; i < 128; i++)
	//	cmd->AppendReplaceInstrument(0, 0, i, 8, 0, 4);

	cmd->LoadMidi(0);
	cmd->PlayMidi(0);

	//cmd->DisableAllMidiTrack(0);
	//cmd->EnableMidiTrack(0, 17);

	/*Sleep(6000);

	for (int i = 0; i < 30; i++) {
		cmd->MidiGoto(0, 7 + i);
		Sleep(100);
	}*/


	//cmd->MidiGoto(0, 102);
	//Sleep(6000);
	//cmd->MidiGoto(0, 187);


	//Sleep(1000);

	//for (int i = 0; i < 1000; i++)
	//{
	//	cmd->RemoveMidi(0);
	//	//播放第1个midi文件
	//	cmd->LoadMidi(0);
	//	cmd->PlayMidi(0);
	//	Sleep(100);
	//}


	//cmd->MidiGoto(0, 30);

	//Sleep(7000);
	//cmd->SuspendMidi(0);
	//cmd->PlayMidi(0);

	//cmd->LoadMidi(0);
	//cmd->PlayMidi(0);

	//cmd->LoadMidi(1);

	//cmd->SetPercussionProgramNum(0, 26);

	//播放第2个midi文件
	//cmd->PlayMidi(1);

	//cmd->DisableMidiTrackChannel(0, 0, 9);

	//cmd->DisableMidiTrackAllChannels(0, 0);
	//cmd->EnableMidiTrackChannel(0, 0, 9);

	//cmd->DisableAllMidiTrack(0);
	//cmd->EnableMidiTrack(0, 1);

	//cmd->DisableMidiTrack(0, 1);
	//cmd->MidiGotoSec(0, 16);


	//去掉注释,弹奏测试
	//art(cmd);
	/*for (int i = 0; i < 5; i++)
	{
		cmd->StopMidi(0);
		cmd->MidiGoto(0, 30);
		cmd->PlayMidi(0);
		Sleep(200);

	}*/

	//art(cmd);

	//
	//Sleep(2000);
	//cmd->MidiGoto(0, 30);
	//cmd->AppendReplaceInstrument(0, 0, 0, 0, 0, 52);
	//cmd->PlayMidi(0);
	//cmd->PlayMidi(1);
	//Sleep(15000);
	//cmd->StopMidi(0);

	////
	////Sleep(2000);
	//cmd->MidiGoto(0, 30);
	//cmd->AppendReplaceInstrument(0, 0, 0, 0, 0, 30);
	//cmd->PlayMidi(0);
	//Sleep(15000);
	//cmd->StopMidi(0);


	////Sleep(2000);
	//cmd->MidiGoto(0, 30);
	//cmd->AppendReplaceInstrument(0, 0, 0, 0, 0, 40);
	//cmd->PlayMidi(0);
	//Sleep(15000);
	//cmd->StopMidi(0);

	/*cmd->PlayMidi(0);
	Sleep(5000);
	cmd->MidiGoto(0, 16);
	Sleep(5000);
	cmd->MidiGoto(0, 60);
	Sleep(5000);
	cmd->MidiGoto(0, 0);*/
	//cmd->LoadMidi(1);
	//cmd->PlayMidi(0);
	//Sleep(5000);
	//cmd->MidiGoto(0, 26);
	//Sleep(5000);
	//cmd->RemoveMidi(0);
	/*vector<VirInstrument*>* insts = cmd->TakeVirInstrumentList();
	for (int i = 0; i < 6; i++)
		cmd->RemoveVirInstrument((*insts)[i]);
	DEL(insts);*/



	//
	Sleep(50000000);
	//delete ventrue;


	return 0;
}
