#include "timer.h"

#include <iostream>
#include <chrono>

double hires_timer::freq_GHz;

void hires_timer::calibrate()
{
    freq_GHz = 2.4;

    auto start = std::chrono::system_clock::now();
    auto end = start + std::chrono::duration<double>(5);
    uint64_t tsc0 = __rdtsc();
    double d = 1.0;
    static volatile double sum = 0.0;
    int cnt = 0;
    while (std::chrono::system_clock::now() < end) {
        for (int i = 0; i < 1000000; i++) {
            sum += 1 / d;
            d += 1;
        }
        ++cnt;
    }
    end = std::chrono::system_clock::now();
    uint64_t tsc1 = __rdtsc();
    std::chrono::duration<double> dur = end - start;
    freq_GHz = (tsc1 - tsc0) / dur.count() * 1.0E-9;
    std::cout << "RDTSC calibrated: freq = " << freq_GHz << " GHz\n";
}

