#include"RingBuffer.h"

namespace ventrue
{
	RingBuffer::RingBuffer(int32_t _bufSize)
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

    void RingBuffer::ReadToDst(void* dst, int32_t len)
    {
        int32_t readEndPos = readPos + len;
        if (readEndPos < bufSize)
        {
            memcpy(dst, buf + readPos, len);
            readPos = readEndPos;
        }
        else
        {
            int frontCount = bufSize - readPos;
            int backCount = readEndPos - bufSize;
            memcpy(dst, buf + readPos, frontCount);
            if(backCount > 0)
                memcpy((uint8_t*)dst + frontCount, buf, backCount);
            readPos = backCount;
        }
    }
}