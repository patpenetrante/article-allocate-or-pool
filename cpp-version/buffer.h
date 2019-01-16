#ifndef _BUFFER_H
#define _BUFFER_H

#include <cstdint>
#include <assert.h>
#include <cstring>

class ReadBuffer
{
    const unsigned char * ptr;
    size_t index;
    size_t size;

public:
    ReadBuffer(const unsigned char * ptr, size_t size) : ptr(ptr), index(0), size(size)
    {
    }

    ReadBuffer& operator= (const ReadBuffer & other)
    {
        ptr = other.ptr;
        index = other.index;
        size = other.size;
        return *this;
    }

    uint32_t get_32()
    {
        assert(index + 4 <= size);
        uint32_t result;
        memcpy(&result, ptr + index, 4);
        index += 4;
        return result;
    }

    uint16_t get_16()
    {
        assert(index + 2 <= size);
        uint16_t result;
        memcpy(&result, ptr + index, 2);
        index += 2;
        return result;
    }
};

class WriteBuffer
{
    unsigned char * const ptr;
    size_t index;
    const size_t capacity;

public:
    WriteBuffer(unsigned char * ptr, size_t capacity) : ptr(ptr), index(0), capacity(capacity)
    {
    }

    void put32(int32_t x)
    {
        assert(index + 4 <= capacity);
        memcpy(ptr + index, &x, 4);
        index += 4;
    }

    void put16(int16_t x)
    {
        assert(index + 2 <= capacity);
        memcpy(ptr + index, &x, 2);
        index += 2;
    }

    size_t length()
    {
        return index;
    }
};

#endif
