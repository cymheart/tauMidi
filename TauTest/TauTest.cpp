
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

	VirInstrument* inst = tau->EnableVirInstrument(0, 0, 0, 88);
	//tau->SetVirInstrumentMidiControllerValue(inst, MidiControllerType::ModulationWheelMSB, 127);

	tau->OnKey(62, 127, inst);
	tau->OnKey(60, 127, inst);
	tau->OnKey(57, 127, inst);
	tau->OnKey(65, 127, inst);
	Sleep(370 - 110);

	tau->OffKey(62, 127, inst);
	tau->OffKey(60, 127, inst);
	tau->OffKey(57, 127, inst);
	tau->OffKey(65, 127, inst);

	tau->OnKey(60, 127, inst);
	tau->OnKey(62, 127, inst);
	tau->OnKey(65, 127, inst);
	tau->OnKey(57, 127, inst);

	Sleep(100);
	tau->OffKey(60, 127, inst);
	tau->OffKey(62, 127, inst);
	tau->OffKey(65, 127, inst);
	tau->OffKey(57, 127, inst);

	Sleep(630 - 370);



	tau->OnKey(65, 67, inst);
	tau->OnKey(62, 67, inst);
	tau->OnKey(57, 67, inst);
	tau->OnKey(60, 67, inst);

	Sleep(100);
	tau->OffKey(65, 27, inst);
	tau->OffKey(62, 27, inst);
	tau->OffKey(57, 27, inst);
	tau->OffKey(60, 27, inst);

	/*for (int j = 0; j < 15; j++)
	{
		for (int i = 55; i < 65; i++)
		{
			tau->OnKey(i, 127, inst);
			tau->OnKey(i + 2, 127, inst);
			tau->OnKey(i + 4, 127, inst);
			tau->OnKey(i + 8, 127, inst);
			Sleep(200);
			tau->OffKey(i, 127, inst);
			tau->OffKey(i + 2, 127, inst);
			tau->OffKey(i + 4, 127, inst);
			tau->OffKey(i + 8, 127, inst);
		}
	}*/

	//tau->OnKey(73, 127, inst);
	//Sleep(2000);
	//tau->OffKey(73, 127, inst);
	//tau->OnKey(60, 127, inst);
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
	tau->SetEnableMidiEventCountOptimize(true);
	tau->SetMidiKeepSameTimeNoteOnCount(20);
	tau->SetOnlyUseOneMainSynther(true);
	//tau->SetUseRegionInnerChorusEffect(false);
	//tau->SetEnableAllVirInstEffects(false);
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
	reverb->SetRoomSize(0.6f);
	reverb->SetWidth(0.5f);
	reverb->SetDamping(0.5f);
	reverb->SetEffectMix(0.6f);
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

	vector<string> midiFiles;



	//添加需要播放的midi文件
	//midiFiles.push_back(midiPath + "tau2.5.9.mid");
	midiFiles.push_back(midiPath + "venture.mid");
	midiFiles.push_back(midiPath + "吻别.mid");
	midiFiles.push_back(midiPath + "dream.mid");
	midiFiles.push_back(midiPath + "鱼姐 remix.mid");
	midiFiles.push_back(midiPath + "决心鱼.mid");
	midiFiles.push_back(midiPath + "Overwrite remix.mid");
	midiFiles.push_back(midiPath + "dreamsp.mid");
	midiFiles.push_back(midiPath + "喜剧之王.mid");
	midiFiles.push_back(midiPath + "Here-We-Are-Again-喜剧之王插曲.mid");
	//midiFiles.push_back(midiPath + "圆周率+TSMB2之作+3.14百万音符.mid");  //（百万音符来袭）
	//midiFiles.push_back(midiPath + "狂妄之人2.mid"); //（音符非常多）
	//midiFiles.push_back(midiPath + "狂妄之人.mid");
	//midiFiles.push_back(midiPath + "狂妄errow.mid");//xm制
	//midiFiles.push_back(midiPath + "混音11.mid");//xm制
	midiFiles.push_back(midiPath + "Undertale AU-Ink！Sans-MEGALOVANIA.mid");
	midiFiles.push_back(midiPath + "learning\\Sound of Silence-Simon & Garfunkel.mid");
	midiFiles.push_back(midiPath + "learning\\QianQianQueGe.mid");
	midiFiles.push_back(midiPath + "learning\\ShiNian.mid");
	midiFiles.push_back(midiPath + "(ACG)芬兰梦境乐团-The Dawn《魔兽世界》亡灵序曲.mid");
	midiFiles.push_back(midiPath + "learning\\Brainiac Maniac 2.mid");
	midiFiles.push_back(midiPath + "song100\\013.mid");
	midiFiles.push_back(midiPath + "song100\\052.mid");
	midiFiles.push_back(midiPath + "learning\\剑之心(Heart_Of_Sword).mid");
	midiFiles.push_back(midiPath + "learning\\Promise.mid");
	midiFiles.push_back(midiPath + "笑傲江湖.mid");
	midiFiles.push_back(midiPath + "笑傲江湖3.mid");
	midiFiles.push_back(midiPath + "learning\\Canon.mid");
	midiFiles.push_back(midiPath + "横山菁児-英雄たちの夜明け.mid");
	midiFiles.push_back(midiPath + "英雄的黎明钢琴版.mid");
	midiFiles.push_back(midiPath + "英雄的黎明(人声UTAU).mid");
	midiFiles.push_back(midiPath + "Battle Against a True Hero.mid");
	midiFiles.push_back(midiPath + "learning\\亡灵序曲.mid");
	midiFiles.push_back(midiPath + "灌篮高手主题曲(完整版).mid");
	midiFiles.push_back(midiPath + "百鸟朝凤.mid");
	//midiFiles.push_back(midiPath + "czh.mid");
	midiFiles.push_back(midiPath + "进击的巨人OP【变调摇滚版】.mid");
	midiFiles.push_back(midiPath + "进击的巨人op.mid");
	midiFiles.push_back(midiPath + "The Reluctant Heroes 进击的巨人.mid");
	midiFiles.push_back(midiPath + "红莲的弓矢-钢琴版.mid");
	midiFiles.push_back(midiPath + "红莲的弓矢 - 进击的巨人 OP1.mid");
	midiFiles.push_back(midiPath + "紅蓮の弓矢.mid");
	midiFiles.push_back(midiPath + "圣斗士星矢-天马.mid");
	midiFiles.push_back(midiPath + "ゲーム名賞_聖闘士星矢-地球儀.mid");
	midiFiles.push_back(midiPath + "爱江山更爱美人-李丽芬.mid");
	midiFiles.push_back(midiPath + "地球仪完美版 圣斗士冥王篇.mid");
	midiFiles.push_back(midiPath + "故乡的原风景 钢琴版.mid");
	//midiFiles.push_back(midiPath + "Fujiwara no Mokou's Theme act.5.0 32 million ICEwiimaker.mid");
	midiFiles.push_back(midiPath + "故乡的原风景(陶笛).mid");

	midiFiles.push_back(midiPath + "Fight.mid");
	midiFiles.push_back(midiPath + "突然的自我.mid");
	midiFiles.push_back(midiPath + "突然的自我3.mid");
	midiFiles.push_back(midiPath + "笑红尘-陈淑桦.mid");
	midiFiles.push_back(midiPath + "共同度过－张国荣.mid");
	midiFiles.push_back(midiPath + "一生所爱(西游记).mid");
	midiFiles.push_back(midiPath + "一生所爱(钢琴).mid");
	midiFiles.push_back(midiPath + "大话西游主题曲.mid");
	//midiFiles.push_back(midiPath + "黑乐谱Victory.mid");
	//midiFiles.push_back(midiPath + "黑乐谱 ASGORE.mid");

	//for (int i = 0; i < 128; i++)
	//	tau->AppendReplaceInstrument(0, 0, i, 0, 0, 11);

	//for (int i = 0; i < 1000; i++)
	//{
		//tau->RemoveMidi();

	string p2 = midiPath + "tau2.5.9.mid";
	string p = midiPath + "圆周率+TSMB2之作+3.14百万音符.mid";
	string p3 = midiPath + "dream.mid";

	//for (int i = 0; i < 100; i++)
	//{
		//string p = midiPath + "紅蓮の弓矢.mid";

	//tau->Read(p2, false);
	////Sleep(20);

	////tau->Read(p2);


	//while (tau->IsReading()) {
	//	Sleep(5);
	//}

	//tau->Load(p2);
	//tau->Load(p);



//	tau->Load(p2);

	//while (!tau->IsLoadCompleted())
	//	Sleep(5);

//	tau->Play();

	//for (int i = 0; i < 10; i++)
	//{
	//	printf("idx=%d\n", i);
	//	tau->Load(p2, false);

	//	Sleep(1000);
	//}

	//while (!tau->IsLoadCompleted()) {
	//	Sleep(5);
	//}

	//tau->Play();

	//tau->Play();



	//Sleep(40);
//	}


	//int i = 15;
//
	//string p2 = "G:\\TauProjects\\TauTest\\midi\\song100\\086.mid";

	//	for (int i = 0; i < 4; i++) {
	//tau->Load(p2);
	//tau->Play();
	//		Sleep(6000);
		//}
		//Sleep(3000);
		//tau->Close();
		//tau->Open();
		//tau->Load(p2);
		//tau->Play();
		//Sleep(3000);
		//tau->Close();
		//Sleep(1000);
		//tau->Open();
		//tau->Load(p);
		//tau->Play();
		//tau->DisableAllTrack();
		//tau->EnableTrack(3);
		//tau->Goto(60 + 37);

		//for (int i = 0; i < 20; i++)
		//{
		//	tau->Goto(434);
		//	tau->Play();
		//	Sleep(2000);
		//}

		//art(tau);

		//tau->DisableTrack(7);
		//Sleep(116000);

	/*vector<string> midiFiles2;
	midiFiles2.push_back(midiPath + "大话西游主题曲.mid");
	int i = -1;
	while (1)
	{
		Sleep(5);
		if (tau->GetEditorState() == EditorState::PAUSE ||
			tau->GetEditorState() == EditorState::STOP)
		{
			i++;
			if (i >= midiFiles2.size())
				break;

			tau->Load(midiFiles2[i]);
			tau->Play();

		}
	}*/


	int i = -1;
	while (1)
	{
		Sleep(5);
		if (tau->GetEditorState() == EditorState::PAUSE ||
			tau->GetEditorState() == EditorState::STOP)
		{
			i++;
			if (i >= midiFiles.size())
				break;

			tau->Load(midiFiles[i]);
			tau->Play();

		}
	}



	//int i = 0;
	//while (1)
	//{
	//	Sleep(5);
	//	if (tau->GetEditorState() == EditorState::PAUSE ||
	//		tau->GetEditorState() == EditorState::STOP)
	//	{
	//		i++;
	//		if (i >= 101)
	//			break;
	//		string a;
	//		if (i < 10)
	//			a = "00" + std::to_string(i);
	//		else if (i < 100)
	//			a = "0" + std::to_string(i);
	//		else
	//			a = std::to_string(i);
	//		string p = midiPath + "song100\\" + a + ".mid";
	//		tau->Load(p);
	//		tau->Play();
	//	}
	//}



	//tau->Load(path);
	//tau->Play();

	//tau->SetTrackPlayType(1, MidiEventPlayType::Background);

	//tau->EnterWaitPlayMode();
	//Sleep(6000);

//	tau->Stop();

	//Sleep(2000);

//	tau->Play();

	/*tau->EnterWaitPlayMode();

	Sleep(1000);

	tau->EditorOnKeySignal(71);

	Sleep(1000);

	tau->EditorOffKeySignal(71);
	tau->EditorOnKeySignal(76);
	Sleep(1000);


	tau->EditorOffKeySignal(76);*/
	//tau->NewTracks(1);
	//tau->SelectInstFragment(1, 0, 0);
	//tau->MoveSelectedInstFragment(2, 0, 5);

	//tau->Goto(35);


	/*
	tau->Wait();
	Sleep(3000);
	tau->Continue();
	Sleep(2000);
	tau->Wait();
	Sleep(3000);
	tau->Continue();*/

	//tau->Goto(80);

//	tau->DisableAllTrack();
	//tau->EnableTrack(1);
	//tau->EditorOnKeySignal(71);
	//tau->EditorOnKeySignal(73);

	//Sleep(1000);

	//tau->EditorOffKeySignal(71);
	//Sleep(1000);
	////tau->EditorOnKeySignal(73);
	////tau->EditorOffKeySignal(71);
	//tau->EditorOffKeySignal(73);

	////tau->EnterStepPlayMode();
	//tau->Runto(2);

	//Sleep(1000);
	//tau->EditorOnKeySignal(73);
	//tau->EditorOnKeySignal(78);

	//Sleep(1000);
	//tau->EditorOffKeySignal(73);
	//Sleep(1000);
	//tau->EditorOnKeySignal(78);
	//tau->EditorOffKeySignal(78);

	//步进播放模式
	/*Sleep(1000);
	tau->EnterStepPlayMode();
	float t = 6;
	for (int i = 0; i < 50; i++)
	{
		Sleep(200);
		t += 0.2;

		tau->Runto(t);
	}

	tau->Runto(0);
	t = 0;
	for (int i = 0; i < 50; i++)
	{
		Sleep(200);
		t += 0.2;

		tau->Runto(t);
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

	p = midiPath + "笑红尘-陈淑桦.mid";
	tau2->Load(p);
	tau2->Play();*/



	//
	char input;
	std::cout << "\n播放中 ... 按 <enter> 退出.\n";
	std::cin.get(input);

	delete tau;
	//delete tau2;

	//Sleep(50000000);

	return 0;
}
