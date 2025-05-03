#ifndef _MeiExporter_h_
#define _MeiExporter_h_

#include "Synth/TauTypes.h"

namespace tau
{
#define NOTE_1 1  //ȫ����
#define NOTE_2  2  //2������
#define NOTE_4  4  //4������
#define NOTE_8  8  //8������
#define NOTE_16  16  //16������
#define NOTE_32  32  //32������
#define NOTE_64  64  //64������
#define NOTE_128  128  //128������
#define NOTE_256  256  //256������
//
#define NOTE_1_D 101  //ȫ��������
#define NOTE_2_D 201  //2����������
#define NOTE_4_D 401  //4����������
#define NOTE_8_D 801  //8����������
#define NOTE_16_D 1601  //16����������
#define NOTE_32_D 3201  //32����������
#define NOTE_64_D 6401  //64����������
#define NOTE_128_D 12801  //128����������
#define NOTE_256_D 25601  //256����������
//
#define NOTE_1_DD 1011  //ȫ������2��
#define NOTE_2_DD 2011  //2��������2��
#define NOTE_4_DD 4011  //4��������2��
#define NOTE_8_DD 8011  //8��������2��
#define NOTE_16_DD 16011  //16��������2��
#define NOTE_32_DD 32011  //32��������2��
#define NOTE_64_DD 64011  //64��������2��
#define NOTE_128_DD 128011  //128��������2��
#define NOTE_256_DD 256011  //256��������2��

	class VirElem;

	//������ֵ��Ϣ
	struct NoteValueInfo {
		float t;
		int noteTimeIdx;
	};

	//Ԫ��Ƭ��
	struct ElemFrag {
		int idx;
		uint32_t startTick;
		uint32_t endTick;
		VirElem* elem;
	};

	//Ԫ�ط�Ͱ
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
		ALONE_NOTE, //������
		REST,       //��ֹ��
		CHORD,      //����
		TUPLET,     //����
		TUPLET_FRAG,//������Ƭ
		TUPLET_BUCKET, //������Ͱ
		TIE,        //����
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
/// ������
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
/// ��ֹ��
/// </summary>
	class Rest : public VirElem {
	public:
		VirElemType Type() {
			return REST;
		}
	};

	/// <summary>
	/// ����
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
	/// ����
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
    /// ������Ƭ
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
    /// ������Ͱ
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
	/// ����
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
	/// mei����
	/// by cymheart, 2025.
	/// </summary> 
	class MeiExporter
	{
	public:
		MeiExporter(Editor* editor);
		~MeiExporter();

		void Execute();

	private:

		/// �Ѽ�С��notes
		void CollectMeasureNotes(Track* track, float measureStartSec, float measureEndSec, vector<NoteOnEvent*>& notes);
		//����
		void Parse(int measureIdx, vector<NoteOnEvent*>& notes);

		//��������֮��ľ����С�����ɵļ�϶,ʹ������һ���̶��Ͽ��Ի������
		void AlginNotes(vector<AloneNote*>& aloneNotes,
			int32_t measureStartTick, int32_t measureEndTick, int perBeatTicks);

		//��������
		vector<VirElem*> CreateTuplets(vector<VirElem*>& elems);
		void ModifyTupletElemTick(VirElem* tupletElem, vector<VirElem*>& elems);

		//����������
		vector<Tuplet*> CreateTuplets(vector<ElemFrag>& frags);
		//����������Ͱ
		TupletBucket* CreateTupletBucket(vector<ElemFrag>& frags);
		//��ȡ������Ͱ��
		vector<vector<ElemBucket>> GetTupletBucketGroup(vector<ElemFrag>& frags, bool isStrictMode = true);

		//�Ե�ǰС����ÿ������ʱֵ��ʽ������׼��������ʱֵ
		//����С�������ķ�����Ϊһ�ĵķ�Χ���и�ʽ��
		void FormatMeasureNoteToStdType(vector<VirElem*>& elems, int32_t measureStartTick);

		//ΪԪ������Ƭ��
		vector<ElemFrag> CreateFrags(vector<VirElem*>& elems);
		//��ȡ�������͵�˳����
		int32_t GetNoteTypeOrderIdx(int32_t noteType);
		//��ȡ����ʱֵ��Ӧ�ı��
		int GetNoteTimeIdx(int noteTicks);
		//��ȡ������ֵ��Ϣ
		NoteValueInfo GetNoteValueInfo(int noteTicks, bool isEnableMinNote = true);
		//����Ԫ��
		VirElem* CopyElem(VirElem* orgElem);

	private:
		Editor* editor = nullptr;
		vector<Track*>* tracks = nullptr;

		//ÿ�Ļ�����������
		//4������ perBeatBaseNote = 4
		//8������ perBeatBaseNote = 8
		int perBeatBaseNote = 4;
		int perBeatBaseNoteOrder = 2;

		//����ʱֵ
		double* noteTimes = nullptr;	
		//����ʱֵ���鳤��
		int32_t noteTimeLen = 0;

		//��������,����ʱֵ��Ӧ��������
		int32_t* noteTypes = nullptr;

		//������������
		int32_t* noteTypeOrders = nullptr;
		int32_t noteTypeOrderLen = 0;

		int32_t qNoteTicks = 0; //�ķ�����tick����
		int32_t perBeatForNNote = 0; //һ�Ķ�Ӧ��������


	};
}

#endif
