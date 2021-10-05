#include"MidiMarker.h"

namespace tau
{
	MidiMarker::MidiMarker()
	{

	}

	MidiMarker::~MidiMarker()
	{

	}

	void MidiMarker::Copy(MidiMarker& cpyMidiMarker)
	{
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
