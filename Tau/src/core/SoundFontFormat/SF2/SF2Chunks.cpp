#include"SF2Chunks.h"

namespace tau
{
	SF2Chunk::SF2Chunk(ByteStream& br)
	{
		br.read(chunkName, 0, 4);
		size = br.read<uint32_t>();
	}

	void SF2Chunk::SetName(const char* name)
	{
		memcpy(chunkName, name, 4);
	}


	SF2ListChunk::SF2ListChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		br.read(listChunkName, 0, 4);
	}

	VersionSubChunk::VersionSubChunk(ByteStream& br)
		: SF2Chunk(br)
	{
		Version.Major = br.read<uint16_t>();
		Version.Minor = br.read<uint16_t>();
	}

	HeaderSubChunk::HeaderSubChunk(ByteStream& br, int maxSize)
		:SF2Chunk(br)
	{
		this->maxSize = maxSize;
		field = (char*)malloc(size);
		br.read((byte*)field, 0, size);
	}

	HeaderSubChunk::~HeaderSubChunk()
	{
		free(field);
	}

	SMPLSubChunk::SMPLSubChunk(ByteStream& br)
		: SF2Chunk(br)
	{
		samples = (short*)malloc(size);
		br.read((byte*)samples, 0, size);
	}

	SMPLSubChunk::~SMPLSubChunk()
	{
		free(samples);
	}


	SM24SubChunk::SM24SubChunk(ByteStream& br)
		: SF2Chunk(br)
	{
		samples = (byte*)malloc(size);
		br.read(samples, 0, size);
	}

	SM24SubChunk::~SM24SubChunk()
	{
		free(samples);
	}

	PHDRSubChunk::PHDRSubChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		for (int i = 0; i < (int)(size / SF2PresetHeader::Size); i++)
			presets.push_back(new SF2PresetHeader(br));
	}

	PHDRSubChunk::~PHDRSubChunk()
	{
		for (int i = 0; i < presets.size(); i++)
			DEL(presets[i]);
		presets.clear();
	}

	INSTSubChunk::INSTSubChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		for (int i = 0; i < (int)(size / SF2Instrument::Size); i++)
			instruments.push_back(new SF2Instrument(br));
	}

	INSTSubChunk::~INSTSubChunk()
	{
		for (int i = 0; i < instruments.size(); i++)
			DEL(instruments[i]);
		instruments.clear();
	}

	BAGSubChunk::BAGSubChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		for (int i = 0; i < (int)(size / SF2Bag::Size); i++)
			bags.push_back(new SF2Bag(br));
	}

	BAGSubChunk::~BAGSubChunk()
	{
		for (int i = 0; i < bags.size(); i++)
			DEL(bags[i]);
		bags.clear();
	}

	MODSubChunk::MODSubChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		for (int i = 0; i < (int)(size / SF2ModulatorList::Size); i++)
			modulators.push_back(new SF2ModulatorList(br));
	}

	MODSubChunk::~MODSubChunk()
	{
		for (int i = 0; i < modulators.size(); i++)
			DEL(modulators[i]);
		modulators.clear();
	}


	GENSubChunk::GENSubChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		for (int i = 0; i < (int)(size / SF2GeneratorList::Size); i++)
			generators.push_back(new SF2GeneratorList(br));
	}

	GENSubChunk::~GENSubChunk()
	{
		for (int i = 0; i < generators.size(); i++)
			DEL(generators[i]);
		generators.clear();
	}

	SHDRSubChunk::SHDRSubChunk(ByteStream& br)
		:SF2Chunk(br)
	{
		for (int i = 0; i < (int)(size / SF2SampleHeader::Size); i++)
			samples.push_back(new SF2SampleHeader(br));
	}

	SHDRSubChunk::~SHDRSubChunk()
	{
		for (int i = 0; i < samples.size(); i++)
			DEL(samples[i]);
		samples.clear();
	}

	InfoListChunk::InfoListChunk(ByteStream& br)
		:SF2ListChunk(br)
	{
		char name[5];
		name[4] = 0;

		size_t startOffset = br.getReadCursor();
		while (br.getReadCursor() < startOffset + size - 4) // The 4 represents the INFO that was already read
		{
			br.peek((byte*)name, 0, 4);

			if (strcmp(name, "ICMT") == 0)
			{
				subChunks.push_back(new HeaderSubChunk(br, commentMaxSize));
			}
			else if (strcmp(name, "ifil") == 0 || strcmp(name, "iver") == 0)
			{
				subChunks.push_back(new VersionSubChunk(br));
			}

			else if (strcmp(name, "isng") == 0 || strcmp(name, "INAM") == 0 ||
				strcmp(name, "ICRD") == 0 || strcmp(name, "IENG") == 0 ||
				strcmp(name, "IPRD") == 0 || strcmp(name, "ICOP") == 0 ||
				strcmp(name, "ISFT") == 0 || strcmp(name, "irom") == 0)
			{
				subChunks.push_back(new HeaderSubChunk(br));
			}
		}
	}

	InfoListChunk::~InfoListChunk()
	{
		for (int i = 0; i < subChunks.size(); i++)
			DEL(subChunks[i]);
		subChunks.clear();
	}


	void InfoListChunk::AddSubChunk(SF2Chunk* subChunk)
	{
		subChunks.push_back(subChunk);
	}


	/// <summary>
	/// 获取备注
	/// </summary>
	/// <returns></returns>
	string InfoListChunk::GetComment()
	{
		string chunkName;
		HeaderSubChunk* chunk;
		for (int i = 0; i < subChunks.size(); i++)
		{
			chunkName.assign((const char*)subChunks[i]->chunkName, 4);
			if (chunkName.compare("ICMT") == 0)
			{
				chunk = (HeaderSubChunk*)subChunks[i];
				chunkName.assign(chunk->field);
				return chunkName;
			}
		}

		return "";
	}

	SdtaListChunk::SdtaListChunk(ByteStream& br)
		:SF2ListChunk(br)
	{
		char name[5];
		name[4] = 0;

		br.peek((byte*)name, 0, 4);
		if (strcmp(name, "smpl") == 0)
			smplSubChunk = new SMPLSubChunk(br);

		br.peek((byte*)name, 0, 4);
		if (strcmp(name, "sm24") == 0)
			sm24SubChunk = new SM24SubChunk(br);
	}

	SdtaListChunk::~SdtaListChunk()
	{
		DEL(smplSubChunk);
		DEL(sm24SubChunk);
	}

	PdtaListChunk::PdtaListChunk(ByteStream& br)
		:SF2ListChunk(br)
	{
		phdrSubChunk = new PHDRSubChunk(br);
		pbagSubChunk = new BAGSubChunk(br);
		pmodSubChunk = new MODSubChunk(br);
		pgenSubChunk = new GENSubChunk(br);
		instSubChunk = new INSTSubChunk(br);
		ibagSubChunk = new BAGSubChunk(br);
		imodSubChunk = new MODSubChunk(br);
		igenSubChunk = new GENSubChunk(br);
		shdrSubChunk = new SHDRSubChunk(br);
	}

	PdtaListChunk::~PdtaListChunk()
	{
		DEL(phdrSubChunk);
		DEL(pbagSubChunk);
		DEL(pmodSubChunk);
		DEL(pgenSubChunk);
		DEL(instSubChunk);
		DEL(ibagSubChunk);
		DEL(imodSubChunk);
		DEL(igenSubChunk);
		DEL(shdrSubChunk);
	}
}
