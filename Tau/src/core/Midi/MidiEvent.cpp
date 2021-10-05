#include"MidiEvent.h"
namespace tau
{
	SysexEvent::~SysexEvent()
	{
		free(data);
	}

	void SysexEvent::CreateData(byte* d, size_t len)
	{
		if (len <= 0)
			return;

		try
		{
			data = (byte*)malloc(len);
			if (data != nullptr)
				memcpy(data, d, len);
			this->size = len;
		}
		catch (exception)
		{

		}
	}

	UnknownEvent::~UnknownEvent()
	{
		free(data);
	}

	void UnknownEvent::CreateData(byte* d, size_t len)
	{
		if (len <= 0)
			return;

		try
		{
			data = (byte*)malloc(len);
			if (data != nullptr)
				memcpy(data, d, len);
			this->size = len;
		}
		catch (exception)
		{

		}
	}
}
