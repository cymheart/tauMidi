#include"SF2.h"
namespace tau
{
	SF2::SF2(string filePath)
	{
		std::ifstream t;
		char* buffer = nullptr;
		size_t length = 0;
		try
		{
			t.open(filePath, ios::in | ios::binary);
			t.seekg(0, std::ios::end);
			length = t.tellg();
			t.seekg(0, std::ios::beg);
			buffer = new char[length];
			t.read(buffer, length);
			t.close();
		}
		catch (exception)
		{
			t.close();
			cout << filePath << "文件打开出错!" << endl;
			return;
		}


		ByteStream br(buffer, length);
		delete[] buffer;

		char chars[5];
		br.read((byte*)chars, 0, 4);
		chars[4] = 0;

		if (strcmp(chars, "RIFF") != 0)
			return;

		size = br.read<uint32_t>();
		br.read((byte*)chars, 0, 4);
		if (strcmp(chars, "sfbk") != 0)
			return;

		infoChunk = new InfoListChunk(br);
		soundChunk = new SdtaListChunk(br);
		hydraChunk = new PdtaListChunk(br);

	}

	SF2::~SF2()
	{
		DEL(infoChunk);
		DEL(soundChunk);
		DEL(hydraChunk);
	}
}
