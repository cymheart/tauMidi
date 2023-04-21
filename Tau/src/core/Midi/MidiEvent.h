#ifndef _MidiEvent_h_
#define _MidiEvent_h_

#include"MidiTypes.h"

namespace tau
{
	/// <summary>
	/// midi事件基类
	/// by cymheart, 2020--2021.
	/// </summary>   
	class MidiEvent
	{
	public:
		MidiEventType type = MidiEventType::Unknown;
		MidiEventPlayType playType = MidiEventPlayType::Background;

		/// <summary>
		/// 起始tick
		/// </summary>
		uint32_t startTick = 0;

		//起始时间点(单位:秒)
		float startSec = 0;

		//结束时间点(单位:秒)
		float endSec = 0;

		/// <summary>
		/// 事件相关通道
		/// </summary>
		int channel = -1;

		// 事件相关轨道
		int track = 0;
	};


	/// <summary>
	/// 按下音符事件
	/// </summary>
	class NoteOnEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 结束tick
		/// </summary>
		uint32_t endTick = 0;

		/// <summary>
		/// 音符
		/// </summary>
		int note = 0;

		/// <summary>
		/// 力度
		/// </summary>
		int velocity = 0;

		/// <summary>
		/// 对应的NoteOffEvent
		/// </summary>
		NoteOffEvent* noteOffEvent = nullptr;

		NoteOnEvent** childNoteOnEvents = nullptr;
		int childNoteOnEventCount = 0;

		NoteOnEvent()
		{
			type = MidiEventType::NoteOn;
		}

		~NoteOnEvent();
		void AppendChildNoteOnEvents(NoteOnEvent** noteOnEv, int count);
		void AppendChildNoteOnEvents(vector<NoteOnEvent*> notes);
	};


	/// <summary>
	/// 松开音符事件
	/// </summary>
	class NoteOffEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 音符
		/// </summary>
		int note = 0;

		/// <summary>
		/// 力度
		/// </summary>
		int velocity = 0;

		/// <summary>
		/// 对应的NoteOnEvent
		/// </summary>
		NoteOnEvent* noteOnEvent = nullptr;

		NoteOffEvent()
		{
			type = MidiEventType::NoteOff;
		}
	};

	/// <summary>
	/// 控制器事件
	/// </summary>
	class ControllerEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 控制器类型
		/// </summary>
		MidiControllerType ctrlType = MidiControllerType::BalanceMSB;

		/// <summary>
		/// 控制值
		/// </summary>
		int value = 0;

		ControllerEvent()
		{
			type = MidiEventType::Controller;
		}
	};

	/// <summary>
	/// 乐器设置事件
	/// </summary>
	class ProgramChangeEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 乐器值
		/// </summary>
		int value = 0;

		ProgramChangeEvent()
		{
			type = MidiEventType::ProgramChange;
		}
	};

	/// <summary>
	/// 按键力度设置事件
	/// </summary>
	class KeyPressureEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 音符
		/// </summary>
		int note = 0;

		/// <summary>
		/// 力度值
		/// </summary>
		int value = 0;

		KeyPressureEvent()
		{
			type = MidiEventType::KeyPressure;
		}
	};


	/// <summary>
	/// 通道力度设置事件
	/// </summary>
	class ChannelPressureEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 力度值
		/// </summary>
		int value = 0;

		ChannelPressureEvent()
		{
			type = MidiEventType::ChannelPressure;
		}
	};

	/// <summary>
	/// 滑音设置事件
	/// </summary>
	class PitchBendEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 滑音值
		/// </summary>
		int value = 0;

		PitchBendEvent()
		{
			type = MidiEventType::PitchBend;
		}
	};

	/// <summary>
	/// 系统码事件
	/// </summary>
	class SysexEvent : public MidiEvent
	{
	public:
		byte* data = nullptr;
		size_t size = 0;

		SysexEvent()
		{
			type = MidiEventType::Sysex;
		}

		SysexEvent(const SysexEvent& obj)
		{
			CreateData(obj.data, obj.size);
		}

		~SysexEvent();
		void CreateData(byte* d, size_t len);

	};

	/// <summary>
	/// 速度设置事件
	/// 一个四分音符的微秒数
	/// </summary>
	class TempoEvent : public MidiEvent
	{
	public:
		/// <summary>
		/// 一个四分音符的微秒数
		/// </summary>
		float microTempo = 0;

		TempoEvent()
		{
			type = MidiEventType::Tempo;
		}
	};

	/// <summary>
	/// 节拍设置事件
	/// 
	/// 拍子:
	/// 在音乐中，时间被分成均等的基本单位，每个单位叫做一个“拍子”或 称一拍。拍子的时值是以音符的时值来表示的，
	/// 一拍的时值可以是四分音符（即以四分音符为一拍），也可以是二分音符（以二分音符为一拍）或八分音符（以八分音符为一拍）。
	/// 拍子的时值是一个相对的时间概念，比如当乐曲的规定速度为每分钟 60 拍时，每拍占用的时间是一秒，半拍是二分之一 秒；
	/// 当规定速度为每分钟 120 拍时，每拍的时间是半秒，半拍就是四分之一 秒，依此类推。拍子的基本时值确定之后，各种时值的音符就与拍子联系在一起。
	/// 例如，当以四分音符为一拍时，一个全音符相当于四拍，一个二分音符相当于两拍， 八分音符相当于半拍，十六分音符相当于四分之一拍；
	/// 如果以八分音符做为 一拍，则全音符相当于八拍，二分音符是四拍，四分音符是两拍，十六分音符是半拍。
	/// 
	/// 拍号:
	/// 拍号是一个分数，写在乐曲开始的地方（第一行谱表的左端，跟在谱号 之后，如果有调号，则顺序为谱号→调号→拍号）。分母表示拍子的基本时值，也就是说以几分音符做为一拍，
	/// 分子表示每小节中有几拍。例如：2/4 表示以四分音符为一拍，每小节有两拍；3/4 表示以四分音符为一拍，每小 节有三拍；3/8 表示以八分音符为一拍，每小节有三拍。拍号的分母部分只有 2、4、8 三种，
	/// 也就是说，用二分音符、四分音符和八分音符作为拍子的 基本时值。 拍号之所以写成分数，是因为在五线谱中将全音符作为整数、即时值分 割的基础单位看待。全音符的标记是 C，设 C 等于 1，二分音符的时值自然是1/2，
	/// 当一小节中有两拍（两个二分音符）时，其时值为 1/2+1/2，得出分数 形式的拍号 2/2 ；当一小节中有三拍（三个二分音符）时，时值为 1/2+1/2+1/2，分数形式的拍号为 3/2。同样道理，四分音符的时值是 1/4，
	///  每小节有两拍（两个四分音符）时拍号写为 2/4，有三拍（三个四分音符号） 即写做 3/4，依此类推。
	/// 在谱表上写拍号时，用三线（即中间的那一条线） 代替表示分数的横线，不必另写。如果乐曲中途不改变拍子的话，只在开始 的地方写一次就可以了，若中途变化拍子，则需标记新的拍号。
	/// 
	/// 小节:
	/// 音乐总是由强拍和弱拍交替进行的，这种交替不能杂乱无章、任意安排， 而是按照一定的规律构成最小的节拍组织一小节，然后以此为基础循环往 复。
	/// 比如，当两个强拍之间只有一个弱拍时称作“二拍子”，2/4 节拍就是 这种类型；当两个强拍之间有二个弱拍时称作“三拍子”，象 3/4 和 3/8；
	///  两个强拍之间有三个弱拍称“四拍子”，常见的是 4/4。两个小节之间用“小节线”隔开。小节线是一条与谱表垂直的细线，上顶五线，下接一线，正好将谱表切断，
	/// 无论上、下加线离开谱表多么远，小 节线都不能超出谱表之外。作品的最后一个小节画两条小节线，并且右面的 一条较粗一些，表示一部作品或一个段落的结束。
	///  小节线后面的那一拍一定是强拍，并且，每个小节只有一个强拍，其余均为弱拍。
	/// 
	/// 
	/// 1/4拍：1 / 4拍是4分音符为一拍，每小节1拍。
	/// 2/4拍：2 / 4拍是4分音符为一拍，每小节2拍，可以有2个4分音符。强－弱。
	///	3/4拍：3 / 4拍是4分音符为一拍，每小节3拍，可以有3个4分音符。强－弱－弱。
	///	4/4拍：4 / 4拍是4分音符为一拍，每小节4拍，可以有4个4分音符。强－弱－次强－弱。
	///	3/8拍：3 / 8拍是8分音符为一拍，每小节可以为一大拍，但是实际上有3拍。可以有3个8分音符。强－弱－弱。
	///	6/8拍：6 / 8拍是8分音符为一拍，每个小节可以分为两大拍，但实际每小节6拍，可以有6个8分音符。强－弱－弱；次强－弱－弱。
	///	还有罕见的8 / 8、8 / 16拍，前者是以8分音符为一拍，后者则是以16分音符为一拍。
	/// </summary>
	class TimeSignatureEvent :public MidiEvent
	{
	public:
		/// <summary>
		/// 分子
		/// </summary>
		int numerator = 4;

		/// <summary>
		/// 分母
		/// denominatorResult = pow(2, denominator);
		/// </summary>
		int denominator = 2;

		/// <summary>
		/// 节拍器一次click的时钟数量
		/// 1个四分音符为24个时钟数量
		/// </summary>
		int metronomeCount = 24;

		/// <summary>
		/// 每个四分音符有几个32分音符
		/// </summary>
		int nCount32ndNotesPerQuarterNote = 8;

		TimeSignatureEvent()
		{
			type = MidiEventType::TimeSignature;
		}
	};

	/// <summary>
	/// 调号设置事件
	/// 大调小调的解释:相关连接：
	/// https://zhuanlan.zhihu.com/p/23125945?refer=pianofanie
	/// https://www.zhihu.com/question/28481469/answer/79850430
	/// https://www.zhihu.com/question/20230795/answer/20900710
	/// 
	/// 12345671（do re mi)是一个八度，八个音，七个音差。但每两个音之间的差距不是平均的。
	/// 两个do之间，不包括高音do，可以分成12份，每份是半音，两个半音是一个全音，这叫12平均律(等比平均）。
	/// 
	///	调式是7个主音之间一种结构关系，就是怎样在八度区间内分配12个半音，定义12345671。大调是这样，
	///	全全半全全全半，加起来12个半音，对应12345671，do Re 之间差一个全音，re mi全音，mi fa半音，
	///	以此类推后面都是全音直到xi do又是一个半音。我们一般从小唱的do Re mi就是这样。
	/// 
	/// 那么什么是C大调，D大调等。C D E F G A B C是五线谱上代表音高的音符。每一个可以对应一个声音频率，即音高。以中音C为例，调琴标准频率260赫兹左右(非准确值)，
	///	即琴弦或簧片每秒震动260次发出的声波。高八度的C就翻倍到520赫兹，八度就是频率加倍，在中间分12份就得到其他音阶。
	///	C大调就是以C，260赫兹，为do的12345671，全全半全全全半，对应CDEFGABC。这样就可以算出DEF等的频率。D大调就是从D，即C大调的2，开始的大调，音阶结构一样。
	///	比如同一首歌，男中音可以唱C大调，女高音可以用D大调，听起来调子一样，音高不同。
	/// 
	/// 小调就是另外一种结构，全半全全半全全，同样以C开头的就是C小调。
	///	在键盘上就是CDd#FGg#a#C。仍然是C到C，八度。如果还唱1234567，调子听起来却像大调的6712345（音高不同）。“#”，读sharp，计算机语言C#也来源于此。
	/// 
	///	可以看出黑白之间是半音，音阶是循环的，C到C之间分成了12份，黑白都是一份，CD差一个全音，EF中间没有黑键所以差一个半音。
	/// 
	///	大调是全全半全全全半，所以任意键开始，比如D大调，应该是DEf#GABc#D。
	/// 
	/// 小调是全半全全半全全，A小调，ABCDEFGA。（全是白键，所以流行？）从这里可以看出，只要记住C大调，A小调都是白键，也就明白了大调小调的结构。
	/// 
	/// 另外，所有音域连起来循环的话，
	///	大调是全全半全全全半全全半全全全半全全半全全全半，黑体字部分是小调，可以看出他们的分配是重叠的，只是有个起始偏差。
	///
	///	1234567，也就是唱名，并不包含有绝对音高的信息，说白了，它们是可以游移的。就好像唱同样的歌，有的人唱得高，有的人唱得低，
	/// 大家高度可以不同。1234567这样一个音阶也是一样，从任意高度起唱1（do），都是可以的。我唱和你唱大多时候会不一样。
	/// 
	/// 然而，音名则不同。CDEFGAB作为各音自己的名称，是与绝对音高挂勾的，不是随便定的，我说do，无法确定它是多高，但我说D，就可以确定它是多高。
	/// 音名与绝对音高相关。比如标准音A，其频率是440Hz，这是固定的，国际公认的。有一个固定频率，就有一个固定的音高。到什么地方，A也是这么高。
	/// 
	///	把以上两部分相叠加，就有了调。由于大家都唱唱名，1234567因为音高可以不同，就有无数组，那么如何来统一呢？假如我们规定，1234567中的1（do，即大调主音）唱C音这个高度，
	/// 后面的其它音按照大调的关系：全全半全全全半，依次往下排。这样就建立了一个以C音高度为1（do）的大调音阶，
	/// 于是就有了C大调。如果以D音为1（do）起，按照大调音阶的关系建立一个音阶，我们就称之为D大调。
	/// 
	///	简而言之，什么叫C大调？以C为do就是C大调，同理，以D为do就是D大调，即：一个调式音阶的主音在什么音上，
	/// 就是什么音的调。所以，不同的调之间的第一个区别就是各调音阶所在的音高不同，形象地说，张宇要唱孙楠的歌，肯定和孙楠不在一个调上。
	/// 
	///	也许你还会问，什么是D小调呢？上面说了，一个调式（此处先不展开说调式，你可以在此简单理解为大调小调等这些东西叫做调式，每种调式都有一个主音）的主音在X音上，
	/// 就称为X大（小）调。所以说，以“D小调”为例，我们可以分成两部分来看，“D”是主音的高度，“小调”是调式的标记。前面说过大调式的各音之音的关系，是全全半全全全半，
	/// （自然）小调式与之不同，是全半全全半全全，类似于67123456（la si do re mi fa sol la）的关系。这种情况下，6是主音。所以，D小调可以简单理解为：以主音D音为6（la）的调。
	/// </summary>
	class KeySignatureEvent : public MidiEvent
	{
	private:
		static string majorName[];
		static string majorFullName[];

		static string minorName[];
		static string minorFullName[];

	public:
		/// <summary>
		/// sf指明乐曲曲调中升号、降号的数目。
		/// 大调：每两个音之间需要构成全全半全全全半的音程关系，形成跨八度音域
		/// C大调：从C音开始构成大调关系，则为：C[全]D[全]E[半]F[全]G[全]A[全]B[半]C， 产生 0个升号,sf = 0
		/// G大调：从G音开始构成大调关系，则为：G[全]A[全]B[半]C[全]D[全]E[全]F#[半]G，产生 1个升号,sf = 1
		/// D大调: 从D音开始构成大调关系，则为：D[全]E[全]F#[半]G[全]A[全]B[全]C#[半]D， 产生 2个升号: sf = 2
		/// A大调: 在五线谱上标注了3个升号，那么sf=03
		/// E大调: sf=4
		/// B大调: sf=5
		/// #F大调: sf=6
		/// #C大调: sf=7
		/// F大调: 五线谱上写有1个降号，那么sf=-1
		/// bB大调: sf=-2
		/// bE大调: sf=-3
		/// bA大调: sf=-4
		/// bD大调: sf=-5
		/// bG大调: sf=-6
		/// bC大调: sf=-7
		/// 
		/// 小调：每两个音之间需要构成全半全全半全全的音程关系，形成跨八度音域
		/// a小调: 从a音开始构成小调关系，则为：a[全]b[半]c[全]d[全]e[半]f[全]g[全]a， 产生 0个升号,sf = 0
		/// e小调: 从e音开始构成小调关系，则为：e[全]f#[半]g[全]a[全]b[半]c[全]d[全]e， 产生 1个升号,sf = 1
		/// b小调: 从b音开始构成小调关系，则为：b[全]c#[半]d[全]e[全]f#[半]g[全]a[全]b， 产生 2个升号,sf = 2
		/// #f小调:在五线谱上标注了3个升号，那么sf=3
		/// #c小调: sf=4   #C[全]#D[半]E[全]#F[全]#G[半]A[全]B[全]#C
		/// #g小调: sf=5
		/// #d小调: sf=6
		/// #a小调: sf=7
		/// d小调: sf=-1   D，E，F，G，A，Bb，C，D
		/// g小调: sf=-2
		/// c小调: sf=-3   C[全]D[半]Eb[全]F[全]G[半]Ab[全]Bb[全]C
		/// f小调: sf=-4
		/// bb小调: sf=-5
		/// be小调: sf=-6
		/// ba小调: sf=-7
		/// 
		/// 
		/// sf = -7: 7 flats 7个降号
		/// sf = -1: 1 flat  1个降号
		/// sf = 0: key of C C调
		/// sf = 1: 1 sharp  1个升调
		/// sf = 7: 7 sharps 7个升调
		/// </summary>
		int sf = 0;

		/// <summary>
		/// mi指出曲调是大调还是小调。大调mi = 00，小调mi = 01。
		/// 大调小调
		/// 0:major (大调)
		/// 1:minor(小调)
		/// </summary>
		int mi = 0;

		KeySignatureEvent()
		{
			type = MidiEventType::KeySignature;
		}

		//获取调号序号
		int GetIdx() {
			return (sf < 0 ? 7 - sf : sf);
		}

		/// <summary>
		/// 获取调号名称
		/// </summary>
		/// <returns></returns>
		string GetName()
		{
			int n = GetIdx();
			return (mi == 0 ? majorName[n] : minorName[n]);
		}

		/// <summary>
		/// 获取调号全名
		/// </summary>
		/// <returns></returns>
		string GetFullName()
		{
			int n = GetIdx();
			return (mi == 0 ? majorFullName[n] : minorFullName[n]);
		}

	};

	/// <summary>
	/// 时间同步事件
	/// </summary>
	class SmpteEvent :public MidiEvent
	{
	public:
		/// <summary>
		/// 时
		/// </summary>
		int hr = 0;

		/// <summary>
		/// 分
		/// </summary>
		int mn = 0;

		/// <summary>
		/// 秒
		/// </summary>
		int sec = 0;

		/// <summary>
		/// 帧
		/// </summary>
		int fr = 0;

		/// <summary>
		/// 复帧
		/// </summary>
		int ff = 8;

		SmpteEvent()
		{
			type = MidiEventType::Smpte;
		}
	};


	/// <summary>
	/// 文本设置事件
	/// </summary>
	class TextEvent : public MidiEvent
	{
	public:
		MidiTextType textType = MidiTextType::GeneralText;
		string text;

		TextEvent()
		{
			type = MidiEventType::Text;
		}
	};


	/// <summary>
	/// 未定义事件
	/// </summary>
	class UnknownEvent : public MidiEvent
	{
	public:
		byte codeType = 0;
		byte* data = nullptr;
		size_t size = 0;

		UnknownEvent()
		{
			type = MidiEventType::Unknown;
		}

		UnknownEvent(const UnknownEvent& obj)
		{
			CreateData(obj.data, obj.size);
		}

		~UnknownEvent();
		void CreateData(byte* d, size_t len);

	};
}

#endif
