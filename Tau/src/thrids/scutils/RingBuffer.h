#ifndef _RingBuffer_h_
#define _RingBuffer_h_

#include "Utils.h"

namespace scutils
{
	class RingBuffer
	{
	public:
		RingBuffer(int64_t _bufSize);
		~RingBuffer();

		template<typename T>
		inline void Write(T value)
		{
			int64_t writeEndPos = writePos + sizeof(T);
			if (writeEndPos < bufSize)
			{
				*((T*)(buf + writePos)) = value;
				writePos += sizeof(T);
			}
			else {
				uint8_t* valPtr = (uint8_t*)(&value);
				int64_t frontCount = bufSize - writePos;
				int64_t backCount = writeEndPos - bufSize;
				memcpy(buf + writePos, valPtr, frontCount);
				if (backCount > 0)
					memcpy(buf, valPtr + frontCount, backCount);
				writePos = backCount;
			}
		}

		template<typename T>
		T Read()
		{
			T value;
			int64_t readEndPos = readPos + sizeof(T);

			if (readEndPos < bufSize)
			{
				value = *((T*)(buf + readPos));
				readPos = readEndPos;
			}
			else
			{
				int64_t frontCount = bufSize - readPos;
				int64_t backCount = readEndPos - bufSize;
				memcpy(tmpBuf, buf + readPos, frontCount);
				if (backCount > 0)
					memcpy(tmpBuf + frontCount, buf, backCount);
				value = *((T*)tmpBuf);
				readPos = backCount;
			}

			return value;
		}

		//读取到目标buffer中
		void ReadToDst(void* dst, int64_t len);

		//获取剩余尺寸
		inline int64_t GetSurplusSize()
		{
			int64_t size;
			if (writePos >= readPos)
				size = writePos - readPos;
			else
				size = bufSize - readPos + writePos;
			return size;
		}

		inline void Clear()
		{
			readPos = 0;
			writePos = 0;
		}

	private:
		volatile int64_t readPos = 0;
		volatile int64_t writePos = 0;
		uint8_t* buf = nullptr;
		int64_t bufSize = 0;

		uint8_t tmpBuf[1024] = { 0 };
	};
}

#endif
