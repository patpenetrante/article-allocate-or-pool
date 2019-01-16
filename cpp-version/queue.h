#ifndef _QUEUE_H
#define _QUEUE_H

#include <atomic>

#ifdef __linux__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

#define CACHE_LINE_SIZE 64

inline void yield()
{
    _mm_pause();
}

template<typename E> class Queue
{
public:
    virtual void write(const E& elem) = 0;
    virtual void read(E& elem) = 0;

    Queue()
    {
    }
};

template<typename E> class DualArrayAsyncQueue : public Queue<E>
{
    const size_t size;
    E * const buf;
    std::uintptr_t diff;

    alignas (CACHE_LINE_SIZE)
        size_t read_ptr;
    const E * read_buf;

    alignas (CACHE_LINE_SIZE)
        size_t write_ptr;
    E * write_buf;

    alignas (CACHE_LINE_SIZE)
        std::atomic<size_t> read_limit;

public:
    DualArrayAsyncQueue(size_t size) : size(size), buf(new E[size * 2]),
        read_ptr(0),
        write_ptr(0),
        read_limit(0)
    {
        read_buf = buf;
        write_buf = buf;
        diff = reinterpret_cast<uintptr_t> (buf) ^ reinterpret_cast<uintptr_t> (buf + size);
    }

    ~DualArrayAsyncQueue()
    {
        delete[] buf;
    }

    void write(const E& elem)
    {
        E * wb = write_buf;
        size_t w = write_ptr;
        if (w < size) {
            wb[w++] = elem;
        }
        if (read_limit.load(std::memory_order_acquire) == 0) {
            read_limit.store(w, std::memory_order_release);
            write_buf = reinterpret_cast<E*>(reinterpret_cast<uintptr_t>(wb) ^ diff);
            w = 0;
        }
        write_ptr = w;
    }

    void read(E& elem)
    {
        size_t lim;

        while ((lim = read_limit.load(std::memory_order_acquire)) == 0) {
            yield();
        }
        size_t r = read_ptr;
        const E* t = read_buf;
        elem = t[r++];
        if (r == lim) {
            read_limit.store(0, std::memory_order_release);
            read_buf = reinterpret_cast<const E*>(reinterpret_cast<uintptr_t>(t) ^ diff);
            r = 0;
        }
        read_ptr = r;
    }
};

#endif
