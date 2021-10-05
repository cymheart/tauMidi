#include"ByteStream.h"

namespace scutils
{
    size_t ByteStream::read(byte* buf, size_t index, size_t len) {
        // Get value at cursor and advance
        len = peek(buf, index, len);
        cursor += len;
        return len;
    }

    ///<summary>Read data from memory and convert it to type T, but don't advance cursor.</summary>
    size_t ByteStream::peek(byte* buf, size_t index, size_t len) const {
        // Check if buffer is too small
        if (cursor + len > size())
        {
            len = size() - cursor;
            if (len == 0)
                return 0;
        }

        // Copy memory into value from the buffer
        memcpy(buf + index, &buffer[cursor], len);

        return len;
    }
}