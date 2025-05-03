﻿
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
#include <FX\Distortion.h>
#include<FX\PcmRecorder.h>
#include<FX\SpectrumVisual.h>
#include<Synth/Editor/Editor.h>
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

VirInstrument* vinst;

//弹奏测试
void art(Tau* tau)
{
	VirInstrument* inst = tau->EnableVirInstrument(0, 0, 0, 0);
	//tau->SetVirInstrumentMidiControllerValue(inst, MidiControllerType::ModulationWheelMSB, 127);
	Sleep(1000);
	 tau->OnKey(70, 89, inst);

	Sleep(500);
	tau->OffKey(70, 89, inst);
	Sleep(12000);

	for (int i = 20; i < 90; i++) {
		tau->OnKey(i, 127, inst);
		Sleep(200);
		tau->OffKey(i, 127, inst);
		Sleep(200);
	}
	

	//	tau->OffKey(92, 127, inst);
		//tau->OffKey(57, 127, inst);
		//tau->OffKey(65, 127, inst);
		//Sleep(100);
		//tau->OnKey(60, 127, inst);
		//tau->OnKey(62, 127, inst);
		//tau->OnKey(65, 127, inst);
		//tau->OnKey(57, 127, inst);

		//Sleep(100);
		//tau->OffKey(60, 127, inst);
		//tau->OffKey(62, 127, inst);
		//tau->OffKey(65, 127, inst);
		//tau->OffKey(57, 127, inst);

		//Sleep(630 - 370);



		//tau->OnKey(65, 67, inst);
		//tau->OnKey(62, 67, inst);
		//tau->OnKey(57, 67, inst);
		//tau->OnKey(60, 67, inst);

		//Sleep(100);
		//tau->OffKey(65, 27, inst);
		//tau->OffKey(62, 27, inst);
		//tau->OffKey(57, 27, inst);
		//tau->OffKey(60, 27, inst);

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
	sf.Parse("SF2", sfPath + "GeneralUser GS MuseScore v1.442.sf2");
	//sf.Parse("SF2", sfPath + "MS Basic.sf2");
	//sf.Parse("SF2", sfPath + "testmod.sf2");
	// sf.Parse("SF3", sfPath + "MS Basic.sf3");
	//sf.Parse("SF2", sfPath + "gnusmas_gm_soundfont_2.01.sf2");
	//sf.Parse("SF2", sfPath + "FluidR3_GM.sf2");
	//sf.Parse("SF2", sfPath + "CrisisGeneralMidi301.sf2");
	//sf.Parse("SF2", sfPath + "Sonatina_Symphonic_Orchestra.sf2");
	//sf.Parse("SF2", sfPath + "Farfisa Grand Piano V4.sf2");
	//sf.Parse("SF2", sfPath + "GMGSx.sf2");
	//sf.Parse("SF2", sfPath + "SGM-V2.01.sf2");
	//sf.Parse("SF2", sfPath + "tongfang.sf2");


	//物理钢琴需要在文件轨道中指定乐器播放才会生效
	//sf.EnablePhysicsPiano(0, 1, 0);


	//sf.Parse("TauFont", sfPath + "OreSoundFont\\GarageBandPianoH\\GarageBandPianoH.xml");
	//sf.Parse("SF2", sfPath + "霸王龙-63M.sf2");
	//sf.Parse("SF2", sfPath + "Farfisa Grand Piano V4.sf2");
	//sf.Parse("SF2", sfPath + "SGM-V2.01.sf2");


	//建立tau
	Tau* tau = new Tau();

	tau->SetFrameSampleCount(256);
	tau->SetSampleProcessRate(44100);
	tau->SetChildFrameSampleCount(64);
	tau->SetLimitZoneSounderCount(500);
	tau->SetLimitOnKeySpeed(600);
	tau->SetSoundFont(&sf);
	//tau->SetChannelOutputMode(ChannelOutputMode::Mono);
	tau->SetEnableMidiEventCountOptimize(true);
	//tau->SetEnableMidiEventParseLimitTime(true, 1);
	tau->SetMidiKeepSameTimeNoteOnCount(20);
	//tau->SetEnableCreateSpectrumsBars(false);
	//tau->SetSampleStreamCacheSec(5);
	tau->SetEnableMergeNotesOptimize(true);
	tau->SetUseZoneInnerChorusEffect(false);
	tau->SetEnableAllVirInstEffects(false);
	//tau->SetAudioEngine(Audio::EngineType::PortAudio);
	//tau->SetAudioEngine(Audio::EngineType::RtAudio);
	//tau->SetEnableCopySameChannelControlEvents(true);
	//tau->SetRenderQuality(RenderQuality::SuperHigh);
	//tau->SetUseMulThread(true);
	//tau->AppendReplaceInstrument(0, 0, 0, 0, 1, 0);

	//添加压缩器效果
	Compressor* compressor = new Compressor();
	//tau->AddEffect(compressor);

	//添加均衡器效果
//	Equalizer* eq = new Equalizer();

//	auto filter = eq->GetFilters();


	//auto a = eq->GetFilters();
//	tau->AddEffect(eq);

	Reverb* reverb = new Reverb();
	reverb->SetRoomSize(0.6f);
	reverb->SetWidth(0.5f);
	reverb->SetDamping(0.3f);
	reverb->SetEffectMix(0.2f);
	tau->AddEffect(reverb);
	//
		//Chorus* chorus = new Chorus();
		//chorus->SetModDepth(0.2);
		////chorus->SetModFrequency(1);
		//chorus->SetEffectMix(1);
		//tau->AddEffect(chorus);

	Autowah* wah = new Autowah();

	wah->LockData();
	wah->SetLevel(1);
	wah->SetDryWet(100);
	wah->SetWah(1);
	wah->UnLockData();

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
//	tau->AddEffect(distortion);

	//TauEffect* pcmRecorder = new PcmRecorder();
	//tau->AddEffect(pcmRecorder);
	//delete (TauEffect*)pcmRecorder;


	//SpectrumVisual* spectrumVisual = new SpectrumVisual();
	//tau->AddEffect(spectrumVisual);


	//打开音频
	tau->Open();

	//spectrumVisual->Init(40,
	//	44100, 2,
	//	60, 5000,
	//	160, 5120);


	vector<string> midiFiles;



	//添加需要播放的midi文件
	//midiFiles.push_back(midiPath + "tau2.5.9.mid");
	//	midiFiles.push_back(midiPath + "01-StartWithMiddleC.mid");
	//midiFiles.push_back(midiPath + "venture.mid");
	//midiFiles.push_back(midiPath + "大航海时代.mid");
	//midiFiles.push_back(midiPath + "世界第一等.mid");
	//midiFiles.push_back(midiPath + "(SG)世界第一等.mid");
	//midiFiles.push_back(midiPath + "地球仪完美版 圣斗士冥王篇.mid");
	//midiFiles.push_back(midiPath + "(SG)世界第一等.mid");
	//midiFiles.push_back(midiPath + "Free Loop.mid");
	//midiFiles.push_back(midiPath + "梦醒时分.mid");
	//midiFiles.push_back(midiPath + "逍遥叹.mid");
	//midiFiles.push_back(midiPath + "仙剑奇侠传 - 蝶恋(95版).mid");
	//midiFiles.push_back(midiPath + "Sunny.mid");
	//	midiFiles.push_back(midiPath + "learning\\MengXingShiFen.mid");
	//midiFiles.push_back(midiPath + "生命之杯GS.mid");
	midiFiles.push_back(midiPath + "追梦人.mid");

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
	midiFiles.push_back(midiPath + "learning\\Pachelbel_Canon_in_D.mid");
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
	midiFiles.push_back(midiPath + "黑乐谱Victory.mid");
	midiFiles.push_back(midiPath + "黑乐谱 ASGORE.mid");

	//for (int i = 0; i < 128; i++)
	//	tau->AppendReplaceInstrument(0, 0, i, 0, 0, 11);

	//for (int i = 0; i < 1000; i++)
	//{
		//tau->RemoveMidi();

	//art(tau);



	string p3 = midiPath + "(SG)世界第一等4.mid"; // "仙剑奇侠传 - 蝶恋(95版).mid";

	//string p3 = midiPath + "TestMEIX21.mid"; // "仙剑奇侠传 - 蝶恋(95版).mid";
	////string p = midiPath + "learning\\QianQianQueGe.mid";
	string p2 = midiPath + "圆周率+TSMB2之作+3.14百万音符.mid";
	string p4 = midiPath + "music\\Alouette.mid";
	string p = midiPath + "(SG)世界第一等.mid";

//	tau->Load(p3);
	//Editor* editor = tau->GetEditor();
	//editor->ExportMEI();


	//tau->SetEnableCreateSpectrumsBars(true);

	//tau->Load(p);

	//tau->EnterMuteMode();

	//tau->SetPlayType(MidiEventPlayType::LeftHand);
//	tau->SetTrackPlayType(1, MidiEventPlayType::LeftHand);


	//tau->SpectrumsVisualInit(40, 40, 100);
	//tau->Play();

	/*Sleep(300);
	tau->Goto(10);*/

	//int i = 0;
	//while (i++ < 20)
	//{
	//	Sleep(300);
	//	tau->Goto(10);
	//	Sleep(300);
	//	tau->Goto(2);
	//}

	//int i = -1;
	//while (1)
	//{
	//	Sleep(2000);
	//	i++;
	//	tau->Load(midiFiles[i]);
	//	tau->Play();
	//}




	//Sleep(10000);


	//double ampbars[320];
	//int freqbars[160];

	//spectrumVisual->LockData();
	//int count = spectrumVisual->GetAmpBars(ampbars, freqbars);
	//int count = spectrumVisual->CreateSmoothAmpBars(ampbars);

//	spectrumVisual->UnLockData();

	//int b;
	//b = 3;
	//double left[20000];
	//double right[20000];

	//int count = tau->GetSampleStreamFreqSpectrums(0, left, right);

	//count = 4;

	//tau->SetOpenAccompany(false);


	/*for (int i = 0; i < 20; i++) {
		Sleep(100);
		tau->Goto(6);
		Sleep(100);
		tau->Goto(16);
	}*/


	//pcmRecorder->StartRecordPCM();

	//Sleep(30000);

	//string path = "G:\\圆周率+TSMB2之作+3.14百万音符.mp3";
	//pcmRecorder->SaveRecordPCMToMp3(path);

	/*while (1)
	{
		Sleep(5);

		if (tau->GetPlayState() == EditorState::PAUSE ||
			tau->GetPlayState() == EditorState::STOP ||
			tau->GetPlayState() == EditorState::ENDPAUSE)
		{
			string path = "G:\\圆周率+TSMB2之作+3.14百万音符.mp3";
			tau->SaveRecordPCMToMp3(path);
			break;
		}
	}*/







	//for (int i = 0; i < 20; i++) {

	//	tau->Goto(3);
	//	tau->Play();

	//	Sleep(7000);
	//	tau->Goto(8);
	//	tau->Play();
	//	Sleep(3000);
	//}

	//Sleep(4000);

	//tau->Remove();

	////Sleep(1000);

	//printf("\n载入:Au Clair de la Lune\n");
	//tau->Load(p4);
	//tau->Play();
	//Sleep(1000);

	//tau->Stop();
	//Sleep(2100);
	//tau->Play();
	//Sleep(1100);
	//tau->Stop();


	//for (int i = 0; i < 20; i++) {

	//	printf("\nStop!!\n");
	//	tau->Goto(0);
	//	printf("\nplay!!!\n");
	//	tau->Play();
	//	Sleep(4000);
	//}


	//for (int i = 0; i < 1; i++) {

	//	printf("\n载入:Au Clair de la Lune\n");
	//	tau->Load(p4);
	//	Sleep(300);
	//	tau->Play();
	//	Sleep(5000);

	//	printf("\n载入:Au Clair de la Lune2\n");
	//	tau->Load(p4);
	//	tau->Play();
	//	Sleep(5000);
	//}


	//for (int i = 0; i < 20; i++)
	//{
	//	printf("\n载入:Au Clair de la Lune\n");
	//	//tau->Remove();
	//	tau->Load(p4);

	//	//Sleep(13000);

	//	tau->Play();

	//	Sleep(400);
	//	printf("\n载入:Au Clair de la Lune2\n");
	//	tau->Load(p4);
	//	tau->Play();
	//	Sleep(400);
	//}

	//tau->Remove();



	//tau->SetPlayType(MidiEventPlayType::LeftHand);
	//tau->SetTrackPlayType(1, MidiEventPlayType::LeftHand);


	//tau->EnterWaitPlayMode();

	//tau->Play();

	//Sleep(3000);

	//tau->EditorOnKeySignal(60);
	//Sleep(200);
	////tau->EditorOffKeySignal(60);

	//Sleep(2000);
	//tau->EditorOnKeySignal(60);
	//Sleep(200);
	//tau->EditorOffKeySignal(60);


	//	Sleep(100);
		//tau->Goto(0);


		////->Goto(4);


		//tau->Load(p3);
		//tau->Play();

		//Sleep(2000);
		//tau->Load(p);
		//tau->Play();


		//	tau->SetTrackPlayType(1, MidiEventPlayType::LeftHand);

		//	tau->CreateSimpleModeTrack();

			//tau->EnterWaitPlayMode();



			//tau->EnterMuteMode();

		//	tau->Play();

			//tau->Goto(16);

			/*Sleep(1000);
			tau->EditorOnKeySignal(55);

			Sleep(1000);
			tau->EditorOnKeySignal(64);

			Sleep(1000);
			tau->EditorOnKeySignal(55);

			Sleep(1000);
			tau->EditorOnKeySignal(57);

			Sleep(1000);
			tau->EditorOnKeySignal(60);

			Sleep(5000);
			tau->Goto(16);*/


			//	tau->EditorOnKeySignal(67);

				//tau->SetSimpleModePlayWhiteKeyCount(3);
				//tau->CreateSimpleModeTrack();
				//tau->Play();

				/*while (true) {
					for (int i = 60; i < 90; i++) {
						tau->OnKey(i, 120, 1);
						Sleep(200);
						tau->OffKey(i, 120, 1);
					}
				}*/

				//art(tau);

	int i = -1;
	while (1)
	{
		Sleep(100);
		if (tau->GetPlayState() == EditorState::PAUSE ||
			tau->GetPlayState() == EditorState::STOP)
		{
			i++;
			if (i >= midiFiles.size())
				break;

			tau->Load(midiFiles[i]);
		//	tau->DisableChannel(6);

		//	tau->DisableAllTrack();
		//	tau->EnableChannel(6);
			//tau->DisableTrack(5);
			//tau->EnableTrack(2);

			//tau->DisableAllTrack();
			//tau->EnableChannel(3);
			//tau->DisableTrack(5);
			//tau->EnableTrack(1);
			//tau->SetMidiVirInstrument(2, 0, 1, 0); //设置轨道2启用物理钢琴乐器
			tau->Play();
		}
	}



	//
	char input;
	std::cout << "\n播放中 ... 按 <enter> 退出.\n";
	std::cin.get(input);

	delete tau;
	//delete tau2;

	//Sleep(50000000);

	return 0;
}
