#include"MidiMarker.h"

namespace tau
{
	string MidiMarker::majorName[] = {
	"C","G","D","A","E","B","#F","#C","F","bB","bE","bA","bD","bG","bC"
	};

	string MidiMarker::majorFullName[] = {
		"C","G","D","A","E","B","F sharp","C sharp","F","B flat","E flat","A flat","D flat","G flat","C flat"
	};

	string MidiMarker::minorName[] = {
		"A","E","B","#F","#C","#G","#D","#A","D","G","C","F","bB","bE","bA"
	};

	string MidiMarker::minorFullName[] = {
		"A","E","B","F sharp","C sharp","G sharp","D sharp","A sharp","D","G","C","F","B flat","E flat","A flat"
	};

	MidiMarker::MidiMarker()
	{

	}

	MidiMarker::~MidiMarker()
	{

	}

	void MidiMarker::Copy(MidiMarker& cpyMidiMarker)
	{
		track = cpyMidiMarker.track;
		isEnableMarkerText = cpyMidiMarker.isEnableMarkerText;
		isEnableTempo = cpyMidiMarker.isEnableTempo;
		startTick = cpyMidiMarker.startTick;
		startSec = cpyMidiMarker.startSec;
		microTempo = cpyMidiMarker.microTempo;
		tickForQuarterNote = cpyMidiMarker.tickForQuarterNote;
		isEnableTimeSignature = cpyMidiMarker.isEnableTimeSignature;
		numerator = cpyMidiMarker.numerator;
		denominator = cpyMidiMarker.denominator;
		isEnableKeySignature = cpyMidiMarker.isEnableKeySignature;
		sf = cpyMidiMarker.sf;
		mi = cpyMidiMarker.mi;
		tempo.Copy(cpyMidiMarker.tempo);
	}

	//计算速度
	void MidiMarker::ComputeTempo()
	{
		tempo.SetTempo(microTempo, tickForQuarterNote, startTick, startSec);
	}
}
