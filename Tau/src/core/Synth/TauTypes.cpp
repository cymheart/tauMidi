#include "TauTypes.h"

namespace tau
{

	int keyTypes88Std[] =
	{
		KeyWhite, KeyBlack, KeyWhite, //A0, bB0, B0
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C1 - B1
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C2 - B2
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C3 - B3
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C4 - B4
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C5 - B5
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C6 - B6
			KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, KeyBlack, KeyWhite, //C7 - B7
			KeyWhite, //C8
	};

	//获取note类型
	int GetNoteType(int note)
	{
		return keyTypes88Std[note - A0];
	}
	//获取下一个黑色类型note
	int GetNextBlackNote(int note)
	{
		for (int i = note - A0 + 1; i < 88; i++)
			if (keyTypes88Std[i] == KeyBlack)
				return i + A0;
		return -1;
	}

	//获取下一个白色类型note
	int GetNextWhiteNote(int note)
	{
		for (int i = note - A0 + 1; i < 88; i++)
			if (keyTypes88Std[i] == KeyWhite)
				return i + A0;
		return -1;
	}

}