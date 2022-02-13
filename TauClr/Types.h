#pragma once

#include <msclr\marshal_cppstd.h>
using namespace msclr::interop;

namespace TauClr
{
	// Editor×´Ì¬
	public enum class EditorState
	{
		//Í£Ö¹
		STOP,
		//²¥·Å
		PLAY,
		//ÔÝÍ£
		PAUSE
	};
}