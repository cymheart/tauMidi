#ifndef _RingBuffer_h_
#define _RingBuffer_h_

#include "Utils.h"

namespace ventrue
{
	class RingBuffer
	{
	public:
		RingBuffer(int32_t _bufSize);
		~RingBuffer();

		template<typename T>
		inline void Write(T value)
		{
			int32_t writeEndPos = writePos + sizeof(T);
			if (writeEndPos < bufSize)
			{
				*((T*)(buf + writePos)) = value;
				writePos += sizeof(T);
			}
			else {
				uint8_t* valPtr = (uint8_t*)(&value);
				int frontCount = bufSize - writePos;
				int backCount = writeEndPos - bufSize;
				memcpy(buf + writePos, valPtr, frontCount);
				if (backCount > 0)
					memcpy(buf, valPtr + frontCount, backCount);
				writePos = backCount;
			}
		}

		template<typename T>
		inline T Read()
		{
			T value;
			int32_t readEndPos = readPos + sizeof(T);
			if (readEndPos < bufSize)
			{
				value = *((T*)(buf + readPos));
				readPos = readEndPos;
			}
			else
			{
				int frontCount = bufSize - readPos;
				int backCount = readEndPos - bufSize;
				memcpy(tmpBuf, buf + readPos, frontCount);
				if (backCount > 0)
					memcpy(tmpBuf + frontCount, buf, backCount);
				value = *((T)tmpBuf);
				readPos = backCount;
			}

			return value;
		}

		//读取到目标buffer中
		void ReadToDst(void* dst, int32_t len);

		//获取剩余尺寸
		inline int32_t GetSurplusSize()
		{
			int size;
			if (writePos >= readPos)
				size = writePos - readPos;
			else
				size = bufSize - readPos + writePos;
			return size;
		}

	private:
		volatile int32_t readPos = 0;
		volatile int32_t writePos = 0;
		uint8_t* buf = nullptr;
		int32_t bufSize = 0;

		uint8_t tmpBuf[1024] = { 0 };
	};
}

#endif
