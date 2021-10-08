
#include <iostream>
#include<string>
#include <stdio.h>
#include"windows.h"
#include<Synth/Tau.h>
#include<Synth/SoundFont.h>
#include"Synth/VirInstrument.h"
#include <FX\Autowah.h>
#include <FX\Reverb.h>
#include <FX\Equalizer.h>
#include <FX\Compressor.h>
#include <FX\Tremolo.h>
#include <FX\Overdrive.h>
#include <FX\Distortion.h>
using namespace tau;


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
void art(Tau* tau)
{

	VirInstrument* inst = tau->EnableVirInstrument(0, 0, 0, 0);
	//tau->SetVirInstrumentMidiControllerValue(inst, MidiControllerType::ModulationWheelMSB, 127);
	tau->OnKey(60, 127, inst);
	Sleep(100);
	tau->OffKey(60, 127, inst);
	tau->OnKey(60, 127, inst);
	//tau->SetVirInstrumentPolyPressure(inst, 60, 120);

	//tau->OffKey(60, 127, inst, 2000);
	//tau->CancelDownKey(60, 127, inst);

	/*for (int i = 0; i < 10; i++)
	{
		Sleep(100);
		tau->SetVirInstrumentPolyPressure(inst, 60, 5);
		Sleep(100);
		tau->SetVirInstrumentPolyPressure(inst, 60, 115);
		Sleep(100);
		tau->SetVirInstrumentPolyPressure(inst, 60, 5);
		Sleep(100);
		tau->SetVirInstrumentPolyPressure(inst, 60, 125);
	}*/

	//tau->OnKey(60, 127, inst);
	//Sleep(1000);
	//tau->OnKey(64, 127, inst);
	//tau->OffKey(59, 127, inst);
	//	Sleep(1000);
	//	//tau->OffKey(61, 127, inst);
	//	//tau->OnKey(49, 127, inst);
	//	//Sleep(30);
	//	tau->OffKey(59, 127, inst);
	//
	//	//tau->OnKey(42, 127, inst);
	//	//Sleep(3000);
	//	//tau->OffKey(41, 127, inst);
	//
	////	tau->OnKey(43, 127, inst);
	////	Sleep(1000);
	//	//tau->OffKey(42, 127, inst);
	//
	//
	//	/*for (int i = 40; i < 60; i++)
	//	{
	//		tau->OnKey(i, 127, inst);
	//		Sleep(1000);
	//		tau->OffKey(i, 127, inst);
	//	}*/

}



int main(int argc, char* argv[])
{
	//资源根路径
	string rootPath = GetProgramDir();
	//midi文件路径
	string midiPath = rootPath + "\\TauTest\\midi\\";
	//soundfont音源路径
	string sfPath = rootPath + "\\TauTest\\SoundFont\\";






	SoundFont sf;
	//设置音源
	//sf.Parse("SF2", sfPath + "GeneralUser GS MuseScore v1.442.sf2");
	sf.Parse("SF2", sfPath + "gnusmas_gm_soundfont_2.01.sf2");
	//sf.Parse("SF2", sfPath + "CrisisGeneralMidi301.sf2");
	//sf.Parse("SF2", sfPath + "GMGSx.sf2");
	//sf.Parse("TauFont", sfPath + "OreSoundFont\\GarageBandPianoH\\GarageBandPianoH.xml");
	//sf.Parse("SF2", sfPath + "霸王龙-63M.sf2");
	//sf.Parse("SF2", sfPath + "Farfisa Grand Piano V4.sf2");
	//sf.Parse("SF2", sfPath + "SGM-V2.01.sf2");


	//建立tau
	Tau* tau = new Tau();

	tau->SetFrameSampleCount(512);
	tau->SetSampleProcessRate(44100);
	tau->SetChildFrameSampleCount(64);
	tau->SetLimitRegionSounderCount(500);
	tau->SetLimitOnKeySpeed(600);
	tau->SetPerSyntherLimitTrackCount(20);
	tau->SetLimitSyntherCount(12);
	tau->SetSoundFont(&sf);
	tau->SetOnlyUseOneMainSynther(true);
	tau->SetUseRegionInnerChorusEffect(false);
	tau->SetEnableAllVirInstEffects(false);
	//tau->SetAudioEngine(Audio::EngineType::PortAudio);
	//tau->SetAudioEngine(Audio::EngineType::RtAudio);
	//tau->SetTrackChannelMergeMode(AutoMerge);
	//tau->SetRenderQuality(RenderQuality::SuperHigh);
	//tau->SetUseMulThread(true);

	//添加压缩器效果
//	Compressor* compressor = new Compressor();
//	tau->AddEffect(compressor);

	//添加均衡器效果
//	EffectEqualizer* eq = new EffectEqualizer();
	//auto a = eq->GetFilters();
//	tau->AddEffect(eq);

	Reverb* reverb = new Reverb();
	reverb->SetRoomSize(0.8f);
	reverb->SetWidth(0.5f);
	reverb->SetDamping(0.5f);
	reverb->SetEffectMix(0.3f);
	//tau->AddEffect(reverb);

	//Chorus* chorus = new Chorus();
	//chorus->SetModDepth(0.2);
	////chorus->SetModFrequency(1);
	//chorus->SetEffectMix(1);
	//tau->AddEffect(chorus);

	Autowah* wah = new Autowah();
	wah->SetLevel(1);
	wah->SetDryWet(100);
	wah->SetWah(1);
	//tau->AddEffect(wah);

	//Tremolo* tremolo = new Tremolo();
	//tremolo->SetDepth(0.8);
	//tremolo->SetFreq(1);
	////tau->AddEffect(tremolo);

	Distortion* distortion = new Distortion();
	distortion->SetDrive(1);
	distortion->SetType(DistortionType::Heavy);
	//distortion->SetThreshold(1);
	distortion->EnablePostFilter();
	distortion->SetFilterAlpha(0);
	distortion->SetRectif(0.5);
	distortion->SetMix(1);
	//distortion->SetSoftWidth(0.2);
	//tau->AddEffect(distortion);


	//打开音频
	tau->Open();



	//添加需要播放的midi文件
	//tau->AppendMidiFile(midiPath + "tau2.5.9.mid");
	//tau->AppendMidiFile(midiPath + "venture.mid");
	//tau->AppendMidiFile(midiPath + "吻别.mid");
	//tau->AppendMidiFile(midiPath + "dream.mid");
	//tau->AppendMidiFile(midiPath + "鱼姐 remix.mid");
	//tau->AppendMidiFile(midiPath + "决心鱼.mid");
	//tau->AppendMidiFile(midiPath + "Overwrite remix.mid");
	//tau->AppendMidiFile(midiPath + "dreamsp.mid");
	//tau->AppendMidiFile(midiPath + "喜剧之王.mid");
	//tau->AppendMidiFile(midiPath + "Here-We-Are-Again-喜剧之王插曲.mid");
	//tau->AppendMidiFile(midiPath + "圆周率+TSMB2之作+3.14百万音符.mid");  //（百万音符来袭）
	//tau->AppendMidiFile(midiPath + "狂妄之人2.mid"); //（音符非常多）
	//tau->AppendMidiFile(midiPath + "狂妄之人.mid");
	//tau->AppendMidiFile(midiPath + "狂妄errow.mid");//xm制
	//tau->AppendMidiFile(midiPath + "混音11.mid");//xm制
	//tau->AppendMidiFile(midiPath + "Undertale AU-Ink！Sans-MEGALOVANIA.mid");
	//tau->AppendMidiFile(midiPath + "learning\\Sound of Silence-Simon & Garfunkel.mid");
	//tau->AppendMidiFile(midiPath + "learning\\QianQianQueGe.mid");
	//tau->AppendMidiFile(midiPath + "learning\\ShiNian.mid");
	//tau->AppendMidiFile(midiPath + "(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");
	//tau->AppendMidiFile(midiPath + "learning\\Brainiac Maniac 2.mid");
	//tau->AppendMidiFile(midiPath + "song100\\013.mid");
	//tau->AppendMidiFile(midiPath + "song100\\052.mid");
	//tau->AppendMidiFile(midiPath + "learning\\剑之心(Heart_Of_Sword).mid");
	//tau->AppendMidiFile(midiPath + "learning\\Promise.mid");
	//tau->AppendMidiFile(midiPath + "笑傲江湖.mid");
	//tau->AppendMidiFile(midiPath + "笑傲江湖3.mid");
	//tau->AppendMidiFile(midiPath + "learning\\Canon.mid");
	//tau->AppendMidiFile(midiPath + "横山菁児-英雄たちの夜明け.mid");
	//tau->AppendMidiFile(midiPath + "英雄的黎明钢琴版.mid");
	//tau->AppendMidiFile(midiPath + "英雄的黎明(人声UTAU).mid");
	//tau->AppendMidiFile(midiPath + "Battle Against a True Hero.mid");
	//tau->AppendMidiFile(midiPath + "learning\\亡灵序曲.mid");
	//tau->AppendMidiFile(midiPath + "灌篮高手主题曲(完整版).mid");
	//tau->AppendMidiFile(midiPath + "百鸟朝凤.mid");
	//tau->AppendMidiFile(midiPath + "czh.mid");
	//tau->AppendMidiFile(midiPath + "进击的巨人OP【变调摇滚版】.mid");
	//tau->AppendMidiFile(midiPath + "进击的巨人op.mid");
	//tau->AppendMidiFile(midiPath + "The Reluctant Heroes 进击的巨人.mid");
	//tau->AppendMidiFile(midiPath + "红莲的弓矢-钢琴版.mid");
	//tau->AppendMidiFile(midiPath + "红莲的弓矢 - 进击的巨人 OP1.mid");
	//tau->AppendMidiFile(midiPath + "紅蓮の弓矢.mid");
	//tau->AppendMidiFile(midiPath + "圣斗士星矢-天马.mid");
	//tau->AppendMidiFile(midiPath + "ゲーム名賞_聖闘士星矢-地球儀.mid");
	//tau->AppendMidiFile(midiPath + "爱江山更爱美人-李丽芬.mid");
	//tau->AppendMidiFile(midiPath + "地球仪完美版 圣斗士冥王篇.mid");
	//tau->AppendMidiFile(midiPath + "故乡的原风景 钢琴版.mid");
	//tau->AppendMidiFile(midiPath + "Fujiwara no Mokou's Theme act.5.0 32 million ICEwiimaker.mid");
	//tau->AppendMidiFile(midiPath + "故乡的原风景(陶笛).mid");
	//tau->AppendMidiFile(midiPath + "BA.DECIMATIONMODE.mid.OR.mid");
	//tau->AppendMidiFile(midiPath + "Fight.mid");

	//tau->AppendMidiFile(midiPath + "笑红尘-陈淑桦.mid");

	//for (int i = 0; i < 128; i++)
	//	tau->AppendReplaceInstrument(0, 0, i, 0, 0, 11);

	//for (int i = 0; i < 1000; i++)
	//{
		//tau->RemoveMidi();

	string path = midiPath + "venture.mid"; // midiPath + "BA.DECIMATIONMODE.mid.OR.mid"; // midiPath + "venture.mid";   //midiPath + "tau2.5.9.mid"; // midiPath + "笑傲江湖.mid";
	tau->Load(path);
	tau->Play();

	//步进播放模式
	/*Sleep(6000);

	tau->EnterStepPlayMode();
	float t = 6;
	for (int i = 0; i < 50; i++)
	{
		Sleep(200);
		t += 0.2;

		tau->Moveto(t);
	}

	tau->Moveto(0);
	t = 0;
	for (int i = 0; i < 50; i++)
	{
		Sleep(200);
		t += 0.2;

		tau->Moveto(t);
	}

	tau->LeaveStepPlayMode();*/

	//Sleep(1000);
	//tau->Moveto(8);
	//Sleep(1000);
	//tau->Moveto(12);


	//tau->SetSpeed(0.3);

	//	tau->DisableChannel(3);
		//Sleep(2000);
		//tau->EnableTrack(3);

		//tau->SetSpeed(0.5);


	//tau->SetSpeed(2);
	//Sleep(3000);
	//tau->SetSpeed(0.5);
	//Sleep(6000);
	//tau->SetSpeed(1);
	//tau->Pause();
	//Sleep(2000);
	//tau->SetSpeed(0.3);
	//tau->Play();
	//Sleep(4000);
	//tau->Goto(0);
	//tau->SetSpeed(1);
	//Sleep(4000);
	//tau->Goto(20);
	//tau->SetSpeed(2);

	//tau->DisableAllMidiTrack();
	//tau->PlayMidi();
	//tau->MidiGoto(290);
	//tau->DisableAllMidiTrack();
	//tau->DisableMidiTrack(1);
	//tau->DisableMidiTrack(6);
	//tau->EnableMidiTrack(11);
	/*Sleep(3000);



	tau->Close();

	Sleep(3000);

	tau->Open();*/

	//}

	//Sleep(6000);

	//delete tau;

	//tau->RemoveMidi();
	//tau->LoadMidi(0);
	//tau->SetMidiPlaySpeed(3);
	//tau->PlayMidi();
	//tau->SetMidiPlaySpeed(0.3);

	//tau->MidiGoto(90);


	//去掉注释,弹奏测试
	//art(tau);


	/*Tau* tau2 = new Tau();
	tau2->SetSoundFont(&sf);
	tau2->Open();

	tau2->AppendMidiFile(midiPath + "笑红尘-陈淑桦.mid");
	tau2->LoadMidi(0);
	tau2->PlayMidi();*/

	//
	char input;
	std::cout << "\n播放中 ... 按 <enter> 退出.\n";
	std::cin.get(input);

	delete tau;
	//delete tau2;

	//Sleep(50000000);

	return 0;
}
