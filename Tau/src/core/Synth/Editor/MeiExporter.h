#ifndef _MeiExporter_h_
#define _MeiExporter_h_

#include "Synth/TauTypes.h"

namespace tau
{
#define NOTE_1 1  //全音符
#define NOTE_2  2  //2分音符
#define NOTE_4  4  //4分音符
#define NOTE_8  8  //8分音符
#define NOTE_16  16  //16分音符
#define NOTE_32  32  //32分音符
#define NOTE_64  64  //64分音符
#define NOTE_128  128  //128分音符
#define NOTE_256  256  //256分音符
//
#define NOTE_1_D 101  //全音符附点
#define NOTE_2_D 201  //2分音符附点
#define NOTE_4_D 401  //4分音符附点
#define NOTE_8_D 801  //8分音符附点
#define NOTE_16_D 1601  //16分音符附点
#define NOTE_32_D 3201  //32分音符附点
#define NOTE_64_D 6401  //64分音符附点
#define NOTE_128_D 12801  //128分音符附点
#define NOTE_256_D 25601  //256分音符附点
//
#define NOTE_1_DD 1011  //全音符附2点
#define NOTE_2_DD 2011  //2分音符附2点
#define NOTE_4_DD 4011  //4分音符附2点
#define NOTE_8_DD 8011  //8分音符附2点
#define NOTE_16_DD 16011  //16分音符附2点
#define NOTE_32_DD 32011  //32分音符附2点
#define NOTE_64_DD 64011  //64分音符附2点
#define NOTE_128_DD 128011  //128分音符附2点
#define NOTE_256_DD 256011  //256分音符附2点

	class VirElem;

	//音符估值信息
	struct NoteValueInfo {
		float t;
		int noteTimeIdx;
	};

	//元素片段
	struct ElemFrag {
		int idx;
		uint32_t startTick;
		uint32_t endTick;
		VirElem* elem;
	};

	//元素分桶
	struct ElemBucket {
		bool isEnable = true;
		float val = 1;
		int ticks = 0;	
		int startFrag = 0;
		uint32_t bucketStartTick = 0;
		vector<ElemFrag> frags;
	};

	enum VirElemType {
		UNKNOWN,
		ALONE_NOTE, //单音符
		REST,       //休止符
		CHORD,      //和声
		TUPLET,     //连音
		TUPLET_FRAG,//连音分片
		TUPLET_BUCKET, //连音分桶
		TIE,        //延音
	};

	class VirElem {
	public:
		virtual ~VirElem(){};
		virtual VirElemType Type() {
			return UNKNOWN;
		}

		virtual void SetTickRange(int32_t _startTick, int32_t _endTick) {
			startTick = _startTick;
			endTick = _endTick;
		}

		virtual void SetStartTick(int32_t _startTick) {
			startTick = _startTick;
		}

		virtual void SetEndTick(int32_t _endTick) {
			endTick = _endTick;
		}

	public:
		int32_t startTick = 0;
		int32_t endTick = 0;
	};

	/// <summary>
/// 单音符
/// </summary>
	class AloneNote : public VirElem {
	public:
		VirElemType Type() {
			return ALONE_NOTE;
		}

		void SetNote(NoteOnEvent* note);
		NoteOnEvent* note = nullptr;
	};

	/// <summary>
/// 休止符
/// </summary>
	class Rest : public VirElem {
	public:
		VirElemType Type() {
			return REST;
		}
	};

	/// <summary>
	/// 和声
	/// </summary>
	class Chord :public VirElem {
	public:
		Chord(){}
		Chord(const Chord& org);
		~Chord();

		VirElemType Type() {
			return CHORD;
		}

		void SetTickRange(int32_t _startTick, int32_t _endTick) {
			startTick = _startTick;
			endTick = _endTick;
			for (int i = 0; i < aloneNotes.size(); i++)
				aloneNotes[i]->SetTickRange(startTick, endTick);
		}

		void SetStartTick(int32_t _startTick) {
			startTick = _startTick;
			for (int i = 0; i < aloneNotes.size(); i++)
				aloneNotes[i]->SetStartTick(startTick);
		}

		void SetEndTick(int32_t _endTick) {
			endTick = _endTick;
			for (int i = 0; i < aloneNotes.size(); i++)
				aloneNotes[i]->SetEndTick(endTick);
		}

	public:
		vector<AloneNote*> aloneNotes;
	};

	/// <summary>
	/// 连音
	/// </summary>
	class Tuplet :public VirElem {

	public:
		~Tuplet();

		VirElemType Type() {
			return TUPLET;
		}

	public:
		vector<VirElem*> elems;
	};

	/// <summary>
    /// 连音分片
    /// </summary>
	class TupletFrag :public VirElem {

	public:
		~TupletFrag();

		VirElemType Type() {
			return TUPLET_FRAG;
		}

	public:
		VirElem* elem;
	};

	/// <summary>
    /// 连音分桶
    /// </summary>
	class TupletBucket :public VirElem {

	public:
		~TupletBucket();

		VirElemType Type() {
			return TUPLET_BUCKET;
		}

	public:
		vector<VirElem*> elems;
	};



	/// <summary>
	/// 延音
	/// </summary>
	class Tie :public VirElem {

	public:
		VirElemType Type() {
			return TIE;
		}

		void GetAllAloneNotes(vector<AloneNote*>& aloneNotes);

	private:
		void GetChildAloneNotes(vector<AloneNote*>&, Tie*);

	public:
		vector<VirElem*> elems;
	};





	/// <summary>
	/// mei导出
	/// by cymheart, 2025.
	/// </summary> 
	class MeiExporter
	{
	public:
		MeiExporter(Editor* editor);
		~MeiExporter();

		void Execute();

	private:

		/// 搜集小节notes
		void CollectMeasureNotes(Track* track, float measureStartSec, float measureEndSec, vector<NoteOnEvent*>& notes);
		//解析
		void Parse(int measureIdx, vector<NoteOnEvent*>& notes);

		//消除音符之间的距离的小误差造成的间隙,使得音符一定程度上可以互相对齐
		void AlginNotes(vector<AloneNote*>& aloneNotes,
			int32_t measureStartTick, int32_t measureEndTick, int perBeatTicks);

		//生成连音
		vector<VirElem*> CreateTuplets(vector<VirElem*>& elems);
		void ModifyTupletElemTick(VirElem* tupletElem, vector<VirElem*>& elems);

		//生成连音组
		vector<Tuplet*> CreateTuplets(vector<ElemFrag>& frags);
		//生成连音分桶
		TupletBucket* CreateTupletBucket(vector<ElemFrag>& frags);
		//获取连音分桶组
		vector<vector<ElemBucket>> GetTupletBucketGroup(vector<ElemFrag>& frags, bool isStrictMode = true);

		//对当前小节中每个音符时值格式化到标准几分音符时值
		//将按小节中以四分音符为一拍的范围进行格式化
		void FormatMeasureNoteToStdType(vector<VirElem*>& elems, int32_t measureStartTick);

		//为元素生成片段
		vector<ElemFrag> CreateFrags(vector<VirElem*>& elems);
		//获取音符类型的顺序编号
		int32_t GetNoteTypeOrderIdx(int32_t noteType);
		//获取音符时值对应的编号
		int GetNoteTimeIdx(int noteTicks);
		//获取音符价值信息
		NoteValueInfo GetNoteValueInfo(int noteTicks, bool isEnableMinNote = true);
		//复制元素
		VirElem* CopyElem(VirElem* orgElem);

	private:
		Editor* editor = nullptr;
		vector<Track*>* tracks = nullptr;

		//每拍基础音符类型
		//4分音符 perBeatBaseNote = 4
		//8分音符 perBeatBaseNote = 8
		int perBeatBaseNote = 4;
		int perBeatBaseNoteOrder = 2;

		//音符时值
		double* noteTimes = nullptr;	
		//音符时值数组长度
		int32_t noteTimeLen = 0;

		//音符类型,音符时值对应音符名称
		int32_t* noteTypes = nullptr;

		//音符名称排序
		int32_t* noteTypeOrders = nullptr;
		int32_t noteTypeOrderLen = 0;

		int32_t qNoteTicks = 0; //四分音符tick数量
		int32_t perBeatForNNote = 0; //一拍对应几分音符


	};
}

#endif
