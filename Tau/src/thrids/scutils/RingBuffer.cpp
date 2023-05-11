#include"RingBuffer.h"

namespace scutils
{
	RingBuffer::RingBuffer(int64_t _bufSize)
	{
		buf = new uint8_t[_bufSize];
		bufSize = _bufSize;

		memset(buf, 0, bufSize);
		readPos = 0;
		writePos = 0;
	}

	RingBuffer::~RingBuffer()
	{
		DEL(buf);
		bufSize = 0;
		readPos = 0;
		writePos = 0;
	}

	void RingBuffer::Write(void* value, int64_t size)
	{
		int64_t writeEndPos = writePos + size;
		if (writeEndPos < bufSize)
		{
			if (value)
				memcpy(buf + writePos, value, size);
			writePos += size;
		}
		else
		{
			uint8_t* valPtr = (uint8_t*)(value);
			int64_t frontCount = bufSize - writePos;
			int64_t backCount = writeEndPos - bufSize;
			if (value) {
				memcpy(buf + writePos, valPtr, frontCount);
				if (backCount > 0)
					memcpy(buf, valPtr + frontCount, backCount);
			}
			writePos = backCount;
		}
	}

	//读取指定长度字节到dst中
	void RingBuffer::ReadToDst(void* dst, int64_t needReadSize)
	{
		if (writePos == readPos)
			return;

		int64_t readEndPos = readPos + needReadSize;
		if (readEndPos < bufSize)
		{
			if (dst)
				memcpy(dst, buf + readPos, needReadSize);
			readPos = readEndPos;
		}
		else
		{
			int64_t frontCount = bufSize - readPos;
			int64_t backCount = readEndPos - bufSize;

			if (dst) {
				memcpy(dst, buf + readPos, frontCount);
				if (backCount > 0)
					memcpy((uint8_t*)dst + frontCount, buf, backCount);
			}

			readPos = backCount;
		}
	}

}
