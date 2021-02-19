#ifndef _ByteStream_h_
#define _ByteStream_h_

#include"Utils.h"
#include <vector>

typedef unsigned char byte;

namespace scutils
{

    class ByteStream 
    {
    public:
        ByteStream() { };
        ByteStream(const void* const ptr, const size_t size);
        

        // Write value to stream
        template<typename T>
        void                    write(const T& value);
        void                    write(const void* const ptr, const size_t size);

        size_t read(byte* buf, size_t index, size_t len);

        ///<summary>Read data from memory and convert it to type T, but don't advance cursor.</summary>
        size_t peek(byte* buf, size_t index, size_t len) const;

        // Read value from stream and advance cursor
        template<typename T>
        T                       read();

        // Read value from stream without advancing cursor
        template<typename T>
        T                       peek() const;

        template<typename T>
        ByteStream& operator<<(T& value);

        template<typename T>
        ByteStream& operator>>(T& output);

        // Beginning of memory
        const byte* begin() const { if (buffer.size() == 0) return nullptr; return &buffer[0]; };
        size_t          size() const { return buffer.size(); };

        void                    clear();

        // Reset cursor to position 0
        void                    resetRead();
        bool                    readEndOfStream() const { return cursor >= buffer.size(); }

        size_t                  getReadCursor() const { return cursor; }
        void                    setReadCursor(size_t pos);


        void                    resetWrite();
        size_t                  getWriteCursor() const { return writeCursor; }
        void                    setWriteCursor(size_t pos);

    private:
        std::vector<byte>       buffer;
        size_t                  cursor = 0;
        size_t                  writeCursor = 0;
    };

    inline ByteStream::ByteStream(const void* const ptr, const size_t size) {
        write(ptr, size);
    }

    ///<summary>Write value onto stream.</summary>
    template<typename T> inline void ByteStream::write(const T& value) { write(&value, sizeof(T)); }
    ///<summary>Write generic memory onto stream.</summary>
    inline void ByteStream::write(const void* const ptr, const size_t dataSize) {

        if (size() < writeCursor + dataSize) {
            size_t sz = (writeCursor + dataSize) * 2;
            buffer.resize(sz);
        }

        if (ptr != nullptr) {
            memcpy(buffer.data() + writeCursor, ptr, dataSize);
            writeCursor += dataSize;
        }
    }

    inline void ByteStream::setWriteCursor(size_t pos) {
        if (pos > size())
            throw std::exception();
        writeCursor = pos;
    }

    inline void ByteStream::resetWrite() { setWriteCursor(0); }

    ///<summary>Read data from memory and convert it to type T, then advance cursor sizeof(T) bytes.</summary>
    template<typename T> inline T ByteStream::read() {
        // Get value at cursor and advance
        T value = peek<T>();
        cursor += sizeof(T);

        return value;
    }

    ///<summary>Read data from memory and convert it to type T, but don't advance cursor.</summary>
    template<typename T> inline T ByteStream::peek() const {
        // Check if buffer is too small
        if (cursor + sizeof(T) > size())
            throw std::exception();

        T       value;

        // Copy memory into value from the buffer
        memcpy(&value, &buffer[cursor], sizeof(T));

        return value;
    }

    ///<summary>Write data onto stream.</summary>
    template<typename T> inline ByteStream& ByteStream::operator<<(T& value) {
        write(value);
        return *this;
    }

    ///<summary>Read data from stream, and copy it into <code>output</code>, then advance cursor sizeof(T) bytes.</summary>
    template<typename T> inline ByteStream& ByteStream::operator>>(T& output) {
        // Read value
        T value = read<T>();

        // Copy memory
        memcpy(&output, &value, sizeof(T));

        return *this;
    }

    ///<summary>Clear all data.</summary>
    inline void ByteStream::clear() {
        buffer.clear();
        resetRead();
        resetWrite();
    }

    ///<summary>Reset cursor position to beginning of stream.</summary>
    inline void ByteStream::resetRead() { setReadCursor(0); }

    ///<summary>Set cursor to position in stream.</summary>
    inline void ByteStream::setReadCursor(size_t pos) {
        if (pos > size())
            throw std::exception();

        cursor = pos;
    }
}

#endif