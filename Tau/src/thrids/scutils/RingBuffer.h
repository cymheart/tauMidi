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
		void Write(T value)
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
		void ReadToDst(void* dst, int64_t needReadSize);

		//拾取指定长度字节到dst中
		void PeekToDst(void* dst, int64_t needReadSize)
		{
			int64_t orgReadPos = readPos;
			ReadToDst(dst, needReadSize);
			readPos = orgReadPos;
		}

		//往后移动len读取位置
		void OffsetReadPos(int64_t len)
		{
			ReadToDst(nullptr, len);
		}

		//往后移动len写入位置
		void OffsetWritePos(int64_t len)
		{
			Write(nullptr, len);
		}


		//获取读取位置
		int64_t GetReadPos()
		{
			return readPos;
		}

		//获取写入位置
		int64_t GetWritePos()
		{
			return writePos;
		}

		//设置写入位置
		void SetWritePos(int64_t pos)
		{
			writePos = 0;
			OffsetWritePos(pos);
		}

		//设置读取位置
		void SetReadPos(int64_t pos)
		{
			readPos = 0;
			OffsetReadPos(pos);
		}

		//获取buf的总尺寸
		int64_t GetBufSize()
		{
			return bufSize;
		}

		//获取原始buf
		uint8_t* GetBuf() {
			return buf;
		}

		//获取剩余需要读取的尺寸
		int64_t GetNeedReadSize()
		{
			int64_t size;
			if (writePos >= readPos)
				size = writePos - readPos;
			else
				size = bufSize - readPos + writePos;
			return size;
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
