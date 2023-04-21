#include"TauFont.h"
#include"Synth/TauTypes.h"
#include"Synth/Sample.h"
#include"Synth/Instrument.h"
#include"Synth/Preset.h"
#include"Synth/Region.h"
#include"SoundFormat/Wav/WavReader.h"
#include <Synth\UnitTransform.h>
#include"Synth/SoundFont.h"

namespace tau
{
	TauFont::~TauFont()
	{
		DEL(xmlDoc);
	}

	/// <summary>      
	/// 解析乐器配置文件到synther 
	/// </summary>
	void TauFont::Parse(string filePath)
	{
		path = filePath;
		size_t end = filePath.find_last_of('\\');
		if (end == filePath.length() - 1)
			end -= 1;
		pathRoot = filePath.substr(0, end + 1);

		if (xmlDoc == nullptr)
			xmlDoc = new tinyxml2::XMLDocument();

		xmlDoc->LoadFile(filePath.c_str());


		ParseUnitType();
		ParseSampleList();
		ParseInstrumentList();
		ParsePresetList();

		xmlDoc->Clear();
	}

	/// <summary>
	/// 解析单位类型
	/// </summary>
	void TauFont::ParseUnitType()
	{

		XMLElement* xmlSynthers = xmlDoc->FirstChildElement("Synther"); //取得节点名为Sample的XmlNode集合
		if (xmlSynthers == nullptr)
			return;

		const XMLAttribute* attr = xmlSynthers->FindAttribute("unitType");
		if (attr == nullptr)
			return;

		string unitTypeStr(attr->Value());
		if (unitTypeStr.compare("SoundFont") == 0)
		{
			unitType = SyntherXmlUnitType::UnitType_SoundFont;
		}
		else if (unitTypeStr.compare("Polyphone") == 0)
		{
			unitType = SyntherXmlUnitType::UnitType_Polyphone;
		}
		else if (unitTypeStr.compare("Synther") == 0)
		{
			unitType = SyntherXmlUnitType::UnitType_Synther;
		}
	}


	/// <summary>  
	/// 解析样本列表    
	/// </summary>      
	void TauFont::ParseSampleList()
	{

		XMLElement* xmlSampleList = xmlDoc->FirstChildElement("Synther")->FirstChildElement("SampleList"); //取得节点SampleList
		if (xmlSampleList == nullptr)
			return;

		string path = "";
		const XMLAttribute* pathAttr = xmlSampleList->FindAttribute("path");
		if (pathAttr != nullptr)
			path.assign(pathAttr->Value());

		XMLElement* xmlSample = xmlSampleList->FirstChildElement("Sample");

		//
		WavReader wavReader;

		struct Info { string link; Sample* sample; Info(string l, Sample* s) { link = l; sample = s; } };
		vector<Info> sampleLinkList;
		string name;
		string file;

		for (; xmlSample; xmlSample = xmlSample->NextSiblingElement("Sample"))
		{
			auto nameAttr = xmlSample->FindAttribute("name");
			if (nameAttr != nullptr) { name.assign(nameAttr->Value()); }

			if (nameAttr == nullptr || name.length() == 0)
				continue;

			auto fileAttr = xmlSample->FindAttribute("file");
			if (fileAttr == nullptr)
			{
				file.assign(name);
			}
			else
			{
				file.assign(fileAttr->Value());
				if (file.length() == 0)
					file.assign(name);
			}


			wavReader.ReadWavFile(pathRoot + path + file + ".wav");
			short* pcm = wavReader.GetLeftChannelData();
			if (pcm == nullptr)
				continue;

			Sample* sample = sf->AddSample(name, pcm, wavReader.GetDataSize());
			XMLElement* childElem = xmlSample->FirstChildElement(); //取得子节点集合

			for (; childElem; childElem = childElem->NextSiblingElement())
			{
				if (childElem->GetText() == nullptr)
					continue;

				const char* name = childElem->Name();

				if (strcmp(name, "Start") == 0)
				{
					childElem->QueryIntText(&(sample->startIdx));
				}
				else if (strcmp(name, "End") == 0)
				{
					childElem->QueryIntText(&(sample->endIdx));
				}
				else if (strcmp(name, "LoopStart") == 0)
				{
					childElem->QueryIntText(&(sample->startloopIdx));
				}
				else if (strcmp(name, "LoopEnd") == 0)
				{
					childElem->QueryIntText(&(sample->endloopIdx));
				}
				else if (strcmp(name, "Rate") == 0)
				{
					childElem->QueryFloatText(&(sample->sampleRate));
				}
				else if (strcmp(name, "OriginalPitch") == 0)
				{
					int orgPitch;
					childElem->QueryIntText(&orgPitch);
					sample->SetOriginalPitch((float)orgPitch);
				}
				else if (strcmp(name, "CentPitchCorrection") == 0)
				{
					float centPitchCorrection;
					childElem->QueryFloatText(&centPitchCorrection);
					sample->SetCentPitchCorrection(centPitchCorrection);
				}
				else if (strcmp(name, "Velocity") == 0)
				{
					float velocity;
					childElem->QueryFloatText(&velocity);
					sample->SetVelocity(velocity);
				}
				else if (strcmp(name, "LinkType") == 0)
				{
					SampleType linkType = SampleType::MonoSample;
					const char* linkTypeText = childElem->GetText();
					if (strcmp(linkTypeText, "MonoSample") == 0)
					{
						linkType = SampleType::MonoSample;
					}
					else if (strcmp(linkTypeText, "RightSample") == 0)
					{
						linkType = SampleType::RightSample;
					}
					else if (strcmp(linkTypeText, "LeftSample") == 0)
					{
						linkType = SampleType::LeftSample;
					}
					else if (strcmp(linkTypeText, "LinkedSample") == 0)
					{
						linkType = SampleType::LinkedSample;
					}

					sample->SetSampleType(linkType);
				}
				else if (strcmp(name, "Link") == 0)
				{
					if (childElem->GetText() == nullptr)
						break;

					string link(childElem->GetText());
					sampleLinkList.push_back(Info(link, sample));
				}
			}


			if (sample->endIdx == 0)
				sample->endIdx = (int)sample->size - 1;

			if (sample->endloopIdx == 0)
				sample->endloopIdx = sample->endIdx;

		}

		//
		SampleList& sampleList = *(sf->GetSampleList());

		for (int i = 0; i < sampleLinkList.size(); i++)
		{
			for (int j = 0; j < sampleList.size(); j++)
			{
				if (sampleList[j]->name == sampleLinkList[i].link)
				{
					sampleLinkList[i].sample->SetSampleLink(sampleList[i]);
				}
			}
		}
	}

	/// <summary>
	/// 解析乐器列表
	/// </summary>
	void TauFont::ParseInstrumentList()
	{
		SampleList& sampleList = *(sf->GetSampleList());
		XMLElement* xmlInstrumentList = xmlDoc->FirstChildElement("Synther")->FirstChildElement("InstrumentList"); //取得InstrumentList
		if (xmlInstrumentList == nullptr)
			return;

		XMLElement* xmlInstrument = xmlInstrumentList->FirstChildElement("Instrument");

		for (; xmlInstrument; xmlInstrument = xmlInstrument->NextSiblingElement())
		{
			string name(xmlInstrument->FindAttribute("name")->Value());
			Instrument* inst = sf->AddInstrument(name);
			XMLElement* childRegionElem = xmlInstrument->FirstChildElement("Region"); //取得子节点集合

			for (; childRegionElem; childRegionElem = childRegionElem->NextSiblingElement("Region"))
			{
				Region* region = nullptr;
				auto sampleAttr = childRegionElem->FindAttribute("sample");
				if (sampleAttr == nullptr || sampleAttr->Value() == nullptr)
				{
					region = inst->GetGlobalRegion();
				}
				else
				{
					for (int i = 0; i < sampleList.size(); i++)
					{
						if (sampleList[i]->name.compare(sampleAttr->Value()) == 0)
						{
							region = sf->SampleBindToInstrument(sampleList[i], inst);
							break;
						}
					}
				}

				if (region == nullptr)
					continue;

				XMLElement* regionChildElem = childRegionElem->FirstChildElement();
				SetGenListByXml(*(region->GetGenList()), regionChildElem, RegionType::Instrument);
			}
		}
	}

	/// <summary>
	/// 解析预设列表
	/// </summary>
	void TauFont::ParsePresetList()
	{
		InstrumentList& instList = *(sf->GetInstrumentList());

		XMLElement* xmlPresetList = xmlDoc->FirstChildElement("Synther")->FirstChildElement("PresetList"); //取得PresetList
		if (xmlPresetList == nullptr)
			return;

		XMLElement* xmlPreset = xmlPresetList->FirstChildElement("Preset");

		for (; xmlPreset; xmlPreset = xmlPreset->NextSiblingElement())
		{
			string name(xmlPreset->FindAttribute("name")->Value());

			int bankMSB = 0;
			int bankLSB = 0;
			int instNum = 0;
			xmlPreset->FindAttribute("bankMSB")->QueryIntValue(&bankMSB);
			xmlPreset->FindAttribute("bankLSB")->QueryIntValue(&bankLSB);
			xmlPreset->FindAttribute("InstNum")->QueryIntValue(&instNum);
			Preset* preset = sf->AddPreset(name, bankMSB, bankLSB, instNum);

			XMLElement* childRegionElem = xmlPreset->FirstChildElement("Region"); //取得子节点集合

			for (; childRegionElem; childRegionElem = childRegionElem->NextSiblingElement("Region"))
			{
				Region* region = nullptr;
				auto instAttr = childRegionElem->FindAttribute("instrument");
				if (instAttr == nullptr || instAttr->Value() == nullptr)
				{
					region = preset->GetGlobalRegion();
				}
				else
				{
					for (int i = 0; i < instList.size(); i++)
					{
						if (instList[i]->name.compare(instAttr->Value()) == 0)
						{
							region = sf->InstrumentBindToPreset(instList[i], preset);
							break;
						}
					}
				}

				if (region == nullptr)
					continue;

				XMLElement* regionChildElem = childRegionElem->FirstChildElement();
				SetGenListByXml(*(region->GetGenList()), regionChildElem, RegionType::Preset);
			}
		}
	}

	bool TauFont::IsEqual(const char* s1, const char* s2)
	{
		return (strcmp(s1, s2) == 0);
	}

	void TauFont::SetGenListByXml(GeneratorList& genList, tinyxml2::XMLElement* elem, RegionType regionType)
	{
		string innerText;
		for (; elem; elem = elem->NextSiblingElement())
		{
			if (elem->GetText() == nullptr)
				continue;

			innerText.assign(elem->GetText());
			if (innerText.length() == 0)
				continue;

			if (IsEqual(elem->Name(), "KeyRange") || IsEqual(elem->Name(), "VelRange"))
			{
				float low, high;
				try
				{
					vector<string> range = split(innerText, "-");
					if (range.size() == 1)
					{

						low = (float)atof(range[0].c_str());
						high = low;
					}
					else
					{
						if (range.size() != 2)
							continue;

						low = (float)atof(range[0].c_str());
						high = (float)atof(range[1].c_str());
					}
				}
				catch (exception)
				{
					continue;
				}

				if (IsEqual(elem->Name(), "KeyRange"))
					genList.SetAmountRange(GeneratorType::KeyRange, low, high);
				else
				{

					genList.SetAmountRange(GeneratorType::VelRange, low, high);
				}
			}
			else
			{
				float val;
				try { val = (float)atof(innerText.c_str()); }
				catch (exception) { continue; }

				const char* name = elem->Name();

				if (IsEqual(name, "StartAddrsOffset")) { genList.SetAmount(GeneratorType::StartAddrsOffset, val); }
				else if (IsEqual(name, "EndAddrsOffset")) { genList.SetAmount(GeneratorType::EndAddrsOffset, val); }
				else if (IsEqual(name, "StartloopAddrsOffset")) { genList.SetAmount(GeneratorType::StartloopAddrsOffset, val); }
				else if (IsEqual(name, "EndloopAddrsOffset")) { genList.SetAmount(GeneratorType::EndloopAddrsOffset, val); }
				else if (IsEqual(name, "StartAddrsCoarseOffset")) { genList.SetAmount(GeneratorType::StartAddrsCoarseOffset, val); }
				else if (IsEqual(name, "ModLfoToPitch")) { genList.SetAmount(GeneratorType::ModLfoToPitch, val); }
				else if (IsEqual(name, "VibLfoToPitch")) { genList.SetAmount(GeneratorType::VibLfoToPitch, val); }
				else if (IsEqual(name, "ModEnvToPitch"))
				{
					genList.SetAmount(GeneratorType::ModEnvToPitch, val);
				}
				else if (IsEqual(name, "InitialFilterFc"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:

						if (regionType == RegionType::Preset)
						{
							val = UnitTransform::TimecentsToSecsf(val);
						}
						else
						{
							val = UnitTransform::CentsToHertz(val);
						}
						break;
					}

					genList.SetAmount(GeneratorType::InitialFilterFc, val);
				}
				else if (IsEqual(name, "InitialFilterQ"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						val = UnitTransform::CentibelsToDecibels(val);
						break;
					}

					genList.SetAmount(GeneratorType::InitialFilterQ, val);
				}


				else if (IsEqual(name, "ModLfoToFilterFc")) { genList.SetAmount(GeneratorType::ModLfoToFilterFc, val); }
				else if (IsEqual(name, "ModEnvToFilterFc")) { genList.SetAmount(GeneratorType::ModEnvToFilterFc, val); }
				else if (IsEqual(name, "EndAddrsCoarseOffset")) { genList.SetAmount(GeneratorType::EndAddrsCoarseOffset, val); }
				else if (IsEqual(name, "ModLfoToVolume"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						if (val > 960) val = 960;
						else if (val < -960) val = 960;
						break;

					case SyntherXmlUnitType::UnitType_Polyphone:
						if (val > 96) val = 96;
						else if (val < -96) val = 96;
						break;
					}
					genList.SetAmount(GeneratorType::ModLfoToVolume, val);
				}
				else if (IsEqual(name, "ChorusEffectsSend"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 10.0f;
						break;
					}

					genList.SetAmount(GeneratorType::ChorusEffectsSend, val);
				}
				else if (IsEqual(name, "ReverbEffectsSend"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 10.0f;
						break;
					}

					genList.SetAmount(GeneratorType::ReverbEffectsSend, val);
				}
				else if (IsEqual(name, "Pan"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 10.0f;
						break;
					}

					genList.SetAmount(GeneratorType::Pan, val);
				}

				else if (IsEqual(name, "DelayModLFO")) { genList.SetAmount(GeneratorType::DelayModLFO, val); }
				else if (IsEqual(name, "FreqModLFO")) { genList.SetAmount(GeneratorType::FreqModLFO, val); }
				else if (IsEqual(name, "DelayVibLFO")) { genList.SetAmount(GeneratorType::DelayVibLFO, val); }
				else if (IsEqual(name, "FreqVibLFO")) { genList.SetAmount(GeneratorType::FreqVibLFO, val); }
				else if (IsEqual(name, "DelayModEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::DelayModEnv, val);
				}
				else if (IsEqual(name, "AttackModEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::AttackModEnv, val);
				}
				else if (IsEqual(name, "HoldModEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::HoldModEnv, val);
				}
				else if (IsEqual(name, "DecayModEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::DecayModEnv, val);
				}
				else if (IsEqual(name, "SustainModEnv"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 1000;
						break;

					case SyntherXmlUnitType::UnitType_Polyphone:
					case SyntherXmlUnitType::UnitType_Synther:
						val /= 100;
						break;
					}

					if (val < 0) { val = 0; }
					if (val > 1) { val = 1; }
					val = 1 - val;
					genList.SetAmount(GeneratorType::SustainModEnv, val);
				}
				else if (IsEqual(name, "ReleaseModEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::ReleaseModEnv, val);
				}
				else if (IsEqual(name, "KeynumToModEnvHold"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_Polyphone:
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 100.0f;
						break;

					}

					genList.SetAmount(GeneratorType::KeynumToModEnvHold, val);
				}
				else if (IsEqual(name, "KeynumToModEnvDecay"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_Polyphone:
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 100.0f;
						break;
				    default:
						break;
					}
					genList.SetAmount(GeneratorType::KeynumToModEnvDecay, val);
				}
				else if (IsEqual(name, "DelayVolEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::DelayVolEnv, val);
				}
				else if (IsEqual(name, "AttackVolEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::AttackVolEnv, val);
				}
				else if (IsEqual(name, "HoldVolEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::HoldVolEnv, val);
				}
				else if (IsEqual(name, "DecayVolEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::DecayVolEnv, val);
				}
				else if (IsEqual(name, "SustainVolEnv"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_SoundFont:
						val = UnitTransform::DecibelsToGain(-val / 10);
						break;

					case SyntherXmlUnitType::UnitType_Polyphone:
					case SyntherXmlUnitType::UnitType_Synther:
						val = UnitTransform::DecibelsToGain(-val);
						break;
					}

					if (val < 0) { val = 0; }
					if (val > 1) { val = 1; }

					genList.SetAmount(GeneratorType::SustainVolEnv, val);
				}
				else if (IsEqual(name, "ReleaseVolEnv"))
				{
					if (unitType == SyntherXmlUnitType::UnitType_SoundFont)
					{
						val = UnitTransform::TimecentsToSecsf(val);
					}
					genList.SetAmount(GeneratorType::ReleaseVolEnv, val);
				}
				else if (IsEqual(name, "KeynumToVolEnvHold"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_Polyphone:
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 100.0f;
						break;
					default:
						break;
					}

					genList.SetAmount(GeneratorType::KeynumToVolEnvHold, val);
				}
				else if (IsEqual(name, "KeynumToVolEnvDecay"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_Polyphone:
					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= 100.0f;
						break;
				    default:
						break;
					}

					genList.SetAmount(GeneratorType::KeynumToVolEnvDecay, val);
				}
				else if (IsEqual(name, "StartloopAddrsCoarseOffset")) { genList.SetAmount(GeneratorType::StartloopAddrsCoarseOffset, val); }
				else if (IsEqual(name, "Keynum")) { genList.SetAmount(GeneratorType::Keynum, val); }
				else if (IsEqual(name, "Velocity")) { genList.SetAmount(GeneratorType::Velocity, val); }
				else if (IsEqual(name, "InitialAttenuation"))
				{
					switch (unitType)
					{
					case SyntherXmlUnitType::UnitType_Synther:
						val = -val / 0.4f;
						break;

					case SyntherXmlUnitType::UnitType_Polyphone:
						val = -val;
						break;

					case SyntherXmlUnitType::UnitType_SoundFont:
						val /= -10.0f;
						break;
					}

					genList.SetAmount(GeneratorType::InitialAttenuation, val);

				}
				else if (IsEqual(name, "EndloopAddrsCoarseOffset")) { genList.SetAmount(GeneratorType::EndloopAddrsCoarseOffset, val); }
				else if (IsEqual(name, "CoarseTune")) { genList.SetAmount(GeneratorType::CoarseTune, val); }
				else if (IsEqual(name, "FineTune")) { genList.SetAmount(GeneratorType::FineTune, val); }
				else if (IsEqual(name, "SampleModes")) { genList.SetAmount(GeneratorType::SampleModes, val); }
				else if (IsEqual(name, "ScaleTuning")) { genList.SetAmount(GeneratorType::ScaleTuning, val); }
				else if (IsEqual(name, "ExclusiveClass")) { genList.SetAmount(GeneratorType::ExclusiveClass, val); }
				else if (IsEqual(name, "OverridingRootKey")) { genList.SetAmount(GeneratorType::OverridingRootKey, val); }
			}

		}
	}
}
