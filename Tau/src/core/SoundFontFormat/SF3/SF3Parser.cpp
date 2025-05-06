#include"SF3Parser.h"
#include"sf3/sfont.h"

namespace tau
{

	void SF3Parser::Parse(string filePath)
	{
		DEL(sf2);
		sf2 = CreateSF2(filePath);

		if (sf2 == nullptr || sf2->size == 0) {
			DEL(sf2);
			cout << filePath << "�ļ�����ʧ��!" << endl;
			return;
		}

		ParseCore();
		DEL(sf2);
	}


	SF2* SF3Parser::CreateSF2(string& filePath)
	{
		SfTools::SoundFont sf3(filePath);
		sf3.read();
		short* smpls = sf3.createUncompressSmpl();

		//infos
		InfoListChunk* infos = new InfoListChunk;
		//
		VersionSubChunk* version = new VersionSubChunk();
		version->Version.Major = sf3.version.major;
		version->Version.Minor = sf3.version.minor;
		version->SetName("ifil");
		version->SetSize(4);
		infos->AddSubChunk(version);
		//
		if (sf3.engine != nullptr) {
			HeaderSubChunk* engine = new HeaderSubChunk();
			engine->SetName("isng");
			engine->SetField(sf3.engine);
			infos->AddSubChunk(engine);
		}
		//
		if (sf3.name != nullptr) {
			HeaderSubChunk* name = new HeaderSubChunk();
			name->SetName("INAM");
			name->SetField(sf3.name);
			infos->AddSubChunk(name);
		}
		//
		if (sf3.date != nullptr) {
			HeaderSubChunk* date = new HeaderSubChunk();
			date->SetName("ICRD");
			date->SetField(sf3.date);
			infos->AddSubChunk(date);
		}
		//
		if (sf3.creator != nullptr) {
			HeaderSubChunk* creator = new HeaderSubChunk();
			creator->SetName("IENG");
			creator->SetField(sf3.creator);
			infos->AddSubChunk(creator);
		}
		//
		if (sf3.product != nullptr) {
			HeaderSubChunk* product = new HeaderSubChunk();
			product->SetName("IPRD");
			product->SetField(sf3.product);
			infos->AddSubChunk(product);
		}
		//
		if (sf3.copyright != nullptr) {
			HeaderSubChunk* copyright = new HeaderSubChunk();
			copyright->SetName("ICOP");
			copyright->SetField(sf3.copyright);
			infos->AddSubChunk(copyright);
		}
		//
		if (sf3.comment != nullptr) {
			HeaderSubChunk* comment = new HeaderSubChunk();
			comment->SetName("ICMT");
			comment->SetField(sf3.comment);
			infos->AddSubChunk(comment);
		}
		//
		if (sf3.tools != nullptr) {
			HeaderSubChunk* tools = new HeaderSubChunk();
			tools->SetName("ISFT");
			tools->SetField(sf3.tools);
			infos->AddSubChunk(tools);
		}

		//
		if (sf3.irom != nullptr) {
			HeaderSubChunk* irom = new HeaderSubChunk();
			irom->SetName("irom");
			irom->SetField(sf3.irom);
			infos->AddSubChunk(irom);
			//
			VersionSubChunk* iver = new VersionSubChunk();
			iver->Version.Major = sf3.iver.major;
			iver->Version.Minor = sf3.iver.minor;
			iver->SetSize(4);
			iver->SetName("iver");
			infos->AddSubChunk(iver);
		}

		//samples
		SdtaListChunk* samples = new SdtaListChunk();
		SMPLSubChunk* smplSub = new SMPLSubChunk();
		smplSub->samples = smpls;
		smplSub->SetName("smpl");
		smplSub->SetSize(sf3.sampleLen);
		samples->smplSubChunk = smplSub;



		//preset&inst datas
		PdtaListChunk* hydraChunk = new PdtaListChunk();

		//
		SHDRSubChunk* smplHdr = new SHDRSubChunk();
		hydraChunk->shdrSubChunk = smplHdr;
		auto snode = sf3.samples.GetHeadNode();
		for (; snode != nullptr; snode = snode->next) {
			auto s = snode->elem;
			auto smpl = new SF2SampleHeader();
			//
			int size = strlen(s->name);
			char* dest = (char*)(smpl->sampleName);
#ifdef _WIN32
			strcpy_s(dest, size, s->name);
#else
			strncpy(dest, s->name, size - 1);
			dest[size - 1] = '\0';
#endif

			smpl->Start = s->start;
			smpl->End = s->end;
			smpl->LoopStart = s->loopstart;
			smpl->LoopEnd = s->loopend;
			smpl->SampleRate = s->samplerate;
			smpl->OriginalKey = s->origpitch;
			smpl->PitchCorrection = s->pitchadj;
			smpl->SampleLink = s->sampleLink;
			smpl->SampleType = (SF2SampleLink)s->sampletype;
			smplHdr->samples.push_back(smpl);
		}

	
		//
		PHDRSubChunk* presets = new PHDRSubChunk();
		hydraChunk->phdrSubChunk = presets;
		//

        int pBagIdx = 0;
        auto node = sf3.presets.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			auto p = node->elem;
			auto sf2Preset = new SF2PresetHeader();

			//
			int size = strlen(p->name);
			char* dest = (char*)(sf2Preset->presetName);
#ifdef _WIN32
			strcpy_s(dest, size, p->name);
#else
			strncpy(dest, p->name, size - 1);
			dest[size - 1] = '\0';
#endif

			//
			sf2Preset->Bank = p->bank;
			sf2Preset->Preset = p->preset;
			sf2Preset->library = p->library;
			sf2Preset->morphology = p->morphology;
			sf2Preset->genre = p->genre;
			sf2Preset->PresetBagIndex = pBagIdx;
			presets->presets.push_back(sf2Preset);
			//
			pBagIdx += p->zones.Size();
		}
   

		//
		BAGSubChunk* pbags = new BAGSubChunk();
		hydraChunk->pbagSubChunk = pbags;
		int genIdx = 0, modIdx = 0;
		node = sf3.presets.GetHeadNode();
		for (; node != nullptr; node = node->next) {
			auto p = node->elem;
			auto zNode = p->zones.GetHeadNode();
			for (; zNode != nullptr; zNode = zNode->next) {
				auto z = zNode->elem;
				SF2Bag* bag = new SF2Bag();
				bag->GeneratorIndex = genIdx;
				bag->ModulatorIndex = modIdx;
				pbags->bags.push_back(bag);
				//
				genIdx += z->generators.Size();
				modIdx += z->modulators.Size();
			}
		}

		SF2Bag* bag = new SF2Bag();
		bag->GeneratorIndex = genIdx;
		bag->ModulatorIndex = modIdx;
		pbags->bags.push_back(bag);


		//
		MODSubChunk* mods = new MODSubChunk();
		GENSubChunk* gens = new GENSubChunk();
		hydraChunk->pmodSubChunk = mods;
		hydraChunk->pgenSubChunk = gens;
		auto zNode = sf3.pZones.GetHeadNode();
		for (; zNode != nullptr; zNode = zNode->next) {
			auto z = zNode->elem;
			auto mNode = z->modulators.GetHeadNode();
			for (; mNode != nullptr; mNode = mNode->next) {
				auto m = mNode->elem;

				SF2ModulatorList* mod = new  SF2ModulatorList();
				mod->ModulatorSource = m->src;
				mod->ModulatorDestination = (SF2Generator)m->dst;
				mod->ModulatorAmount = m->amount;
				mod->ModulatorAmountSource = m->amtSrc;
				mod->ModulatorTransform = (SF2Transform)m->transform;
				mods->modulators.push_back(mod);	
			}

			//
			auto gNode = z->generators.GetHeadNode();
			for (; gNode != nullptr; gNode = gNode->next) {
				auto g = gNode->elem;
				SF2GeneratorList* gen = new SF2GeneratorList();
				gen->Generator = (SF2Generator)g->gen;
				gen->GeneratorAmount.ranges.byHi = g->amount.hi;
				gen->GeneratorAmount.ranges.byLo = g->amount.lo;
				gen->GeneratorAmount.Amount = g->amount.sword;
				gen->GeneratorAmount.UAmount = g->amount.uword;
				gens->generators.push_back(gen);
			}
		}
		//
		//
		//
		INSTSubChunk* insts = new INSTSubChunk();
		hydraChunk->instSubChunk = insts;
		int ibagIdx = 0;
		auto inode = sf3.instruments.GetHeadNode();
		for (; inode != nullptr; inode = inode->next) {
			auto inst = inode->elem;
			auto sf2Inst = new SF2Instrument();
			//
			int size = strlen(inst->name);
			char* dest = (char*)(sf2Inst->instrumentName);
#ifdef _WIN32
			strcpy_s(dest, size, inst->name);
#else
			strncpy(dest, inst->name, size - 1);
			dest[size - 1] = '\0';
#endif

			//
			sf2Inst->InstrumentBagIndex = ibagIdx;
			insts->instruments.push_back(sf2Inst);
			//
			ibagIdx += inst->zones.Size();
		}

		//
		BAGSubChunk* ibags = new BAGSubChunk();
		hydraChunk->ibagSubChunk = ibags;
		genIdx = 0; modIdx = 0;
		inode = sf3.instruments.GetHeadNode();
		for (; inode != nullptr; inode = inode->next) {
			auto inst = inode->elem;
			auto zNode = inst->zones.GetHeadNode();
			for (; zNode != nullptr; zNode = zNode->next) {
				auto z = zNode->elem;
				SF2Bag* bag = new SF2Bag();
				bag->GeneratorIndex = genIdx;
				bag->ModulatorIndex = modIdx;
				ibags->bags.push_back(bag);
				//
				genIdx += z->generators.Size();
				modIdx += z->modulators.Size();
			}
		}

		bag = new SF2Bag();
		bag->GeneratorIndex = genIdx;
		bag->ModulatorIndex = modIdx;
		ibags->bags.push_back(bag);

		//
		MODSubChunk* imods = new MODSubChunk();
		GENSubChunk* igens = new GENSubChunk();
		hydraChunk->imodSubChunk = imods;
		hydraChunk->igenSubChunk = igens;
		zNode = sf3.iZones.GetHeadNode();
		for (; zNode != nullptr; zNode = zNode->next) {
			auto z = zNode->elem;
			auto mNode = z->modulators.GetHeadNode();
			for (; mNode != nullptr; mNode = mNode->next) {
				auto m = mNode->elem;
				SF2ModulatorList* mod = new  SF2ModulatorList();
				mod->ModulatorSource = m->src;
				mod->ModulatorDestination = (SF2Generator)m->dst;
				mod->ModulatorAmount = m->amount;
				mod->ModulatorAmountSource = m->amtSrc;
				mod->ModulatorTransform = (SF2Transform)m->transform;
				imods->modulators.push_back(mod);
			}

			//
			auto gNode = z->generators.GetHeadNode();
			for (; gNode != nullptr; gNode = gNode->next) {
				auto g = gNode->elem;
				SF2GeneratorList* gen = new SF2GeneratorList();
				gen->Generator = (SF2Generator)g->gen;
				gen->GeneratorAmount.ranges.byHi = g->amount.hi;
				gen->GeneratorAmount.ranges.byLo = g->amount.lo;
				gen->GeneratorAmount.Amount = g->amount.sword;
				gen->GeneratorAmount.UAmount = g->amount.uword;
				igens->generators.push_back(gen);
			}
		}
	
		//
		SF2* sf2 = new SF2;
		sf2->size = sf3.fileLen;
		sf2->infoChunk = infos;
		sf2->soundChunk = samples;
		sf2->hydraChunk = hydraChunk;

		return sf2;
	}

}