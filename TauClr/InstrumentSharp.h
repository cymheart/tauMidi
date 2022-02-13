#pragma once

#include<vcclr.h>
#include"Synth/VirInstrument.h"
#include"Types.h"

using namespace System;
using namespace tau;
using namespace std;

namespace TauClr
{

	public ref class InstrumentSharp
	{

	public:
		InstrumentSharp();
		~InstrumentSharp();

		void SetVirInstrument(VirInstrument* vinst)
		{
			inst = vinst;
		}

		VirInstrument* GetVirInstrument()
		{
			return inst;
		}

	private:
		VirInstrument* inst;
	};
}