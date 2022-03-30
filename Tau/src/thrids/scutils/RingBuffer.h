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
				Write((uint8_t*)(&value), sizeof(T));
			}
		}

		void Write(void* value, int64_t size);

		template<typename T>
		T Read()
		{

			T value;
			if (writePos == readPos)
				return value;

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

		//读取指定长度字节到dst中
		void ReadToDst(void* dst, int64_t len);

		//拾取指定长度字节到dst中
		inline void PeekToDst(void* dst, int64_t len)
		{
			int64_t orgReadPos = readPos;
			ReadToDst(dst, len);
			readPos = orgReadPos;
		}

		inline void OffsetReadPos(int64_t len)
		{
			ReadToDst(nullptr, len);
		}

		inline int64_t GetReadPos()
		{
			return readPos;
		}

		//获取剩余需要读取的尺寸
		inline int64_t GetNeedReadSize()
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

		uint8_t* buf = nullptr;
		int64_t readPos = 0;
		int64_t writePos = 0;
		int64_t bufSize = 0;

		uint8_t tmpBuf[1024] = { 0 };
	};
}

#endif
