#ifndef _SF2Chunks_h_
#define _SF2Chunks_h_

#include"SF2Types.h"
#include"SF2Structs.h"
namespace tau
{
	class SF2Chunk
	{
	public:
		SF2Chunk() {};
		SF2Chunk(ByteStream& br);
		virtual ~SF2Chunk() {};
		void SetName(const char* name);
		void SetSize(uint32_t sz) {
			size = sz;
		}

	public:
		byte chunkName[4];
		uint32_t size = 0;
	};

	class SF2ListChunk :public SF2Chunk
	{
	public:
		SF2ListChunk() {};
		SF2ListChunk(ByteStream& br);
	public:
		byte listChunkName[4];
	};

	class VersionSubChunk : public SF2Chunk
	{

	public:
		VersionSubChunk() { SetName("ifil"); };
		VersionSubChunk(ByteStream& br);

	public:
		SF2VersionTag Version;

	};

	class HeaderSubChunk : public SF2Chunk
	{
	public:
		int maxSize = 0x100;
		char* field = nullptr;

	public:
		HeaderSubChunk() {};
		HeaderSubChunk(ByteStream& br, int maxSize = 0x100);
		~HeaderSubChunk();

		void SetField(char* str);

	};

	/// <summary>
	/// 16位采样数据子块
	/// </summary>
	class SMPLSubChunk : public SF2Chunk
	{
	public:
		SMPLSubChunk() {};
		SMPLSubChunk(ByteStream& br);
		~SMPLSubChunk();

	public:
		short* samples = nullptr;
	};

	/// <summary>
   /// 低8位采样数据子块
   /// </summary>
	class SM24SubChunk : public SF2Chunk
	{
	public:
		SM24SubChunk() {};
		SM24SubChunk(ByteStream& br);
		~SM24SubChunk();

	public:
		byte* samples = nullptr;
	};


	/// <summary>
	/// 预设列表子块
	/// </summary>
	class PHDRSubChunk : public SF2Chunk
	{
	public:
		PHDRSubChunk() {};
		PHDRSubChunk(ByteStream& br);
		~PHDRSubChunk();
	public:
		vector<SF2PresetHeader*> presets;
	};


	/// <summary>
	/// 乐器列表子块
	/// </summary>
	class INSTSubChunk : public SF2Chunk
	{

	public:
		INSTSubChunk() {};
		INSTSubChunk(ByteStream& br);
		~INSTSubChunk();

	public:
		vector<SF2Instrument*> instruments;
	};

	/// <summary>
	/// 区域列表子块
	/// </summary>
	class BAGSubChunk : public SF2Chunk
	{
	public:
		BAGSubChunk() {};
		BAGSubChunk(ByteStream& br);
		~BAGSubChunk();

	public:
		vector<SF2Bag*> bags;

	};

	/// <summary>
	/// 调制器列表子块
	/// </summary>
	class MODSubChunk : public SF2Chunk
	{
	public:
		MODSubChunk() {};
		MODSubChunk(ByteStream& br);
		~MODSubChunk();

	public:
		vector<SF2ModulatorList*> modulators;

	};


	/// <summary>
	/// 生成器列表子块
	/// </summary>
	class GENSubChunk : public SF2Chunk
	{
	public:
		GENSubChunk() {};
		GENSubChunk(ByteStream& br);
		~GENSubChunk();

	public:
		vector<SF2GeneratorList*> generators;

	};

	/// <summary>
	/// 样本列表子块
	/// </summary>
	class SHDRSubChunk : public SF2Chunk
	{
	public:
		SHDRSubChunk() {};
		SHDRSubChunk(ByteStream& br);
		~SHDRSubChunk();

	public:
		vector<SF2SampleHeader*> samples;

	};

	/// <summary>
	/// 信息列表块
	/// </summary>
	class InfoListChunk : public SF2ListChunk
	{
	public:
		InfoListChunk() {};
		InfoListChunk(ByteStream& br);
		~InfoListChunk();

		void AddSubChunk(SF2Chunk* subChunk);

		// 获取备注
		string GetComment();

	public:
		const int commentMaxSize = 0x10000;
		vector<SF2Chunk*> subChunks;
	};


	/// <summary>
  /// 样本列表块
  /// </summary>
	class SdtaListChunk : public SF2ListChunk
	{
	public:
		SdtaListChunk() {};
		SdtaListChunk(ByteStream& br);
		~SdtaListChunk();

	public:
		SMPLSubChunk* smplSubChunk = nullptr;
		SM24SubChunk* sm24SubChunk = nullptr;
	};


	class PdtaListChunk : public SF2ListChunk
	{
	public:
		PdtaListChunk() {};
		PdtaListChunk(ByteStream& br);
		~PdtaListChunk();

	public:
		PHDRSubChunk* phdrSubChunk = nullptr;
		BAGSubChunk* pbagSubChunk = nullptr;
		MODSubChunk* pmodSubChunk = nullptr;
		GENSubChunk* pgenSubChunk = nullptr;
		INSTSubChunk* instSubChunk = nullptr;
		BAGSubChunk* ibagSubChunk = nullptr;
		MODSubChunk* imodSubChunk = nullptr;
		GENSubChunk* igenSubChunk = nullptr;
		SHDRSubChunk* shdrSubChunk = nullptr;
	};
}

#endif
