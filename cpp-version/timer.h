#ifndef _TIMER_H
#define _TIMER_H

#include <cstdint>
#include <cmath>

#ifdef __linux__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

class hires_timer
{
    const unsigned FACTOR = 1024;
    const unsigned interval;
    uint64_t start_time;

    static double freq_GHz;

public:
    static void calibrate();

    hires_timer(int interval_ns) : interval((unsigned)round(freq_GHz * interval_ns * FACTOR))
    {
    }

    inline void start()
    {
        start_time = __rdtsc();
    }

    inline void iteration(uint64_t i)
    {
        sleep_until(interval * i / FACTOR + start_time);
    }

private:
    inline void sleep_until(uint64_t until)
    {
        while (__rdtsc() < until);
    }
};

#endif
