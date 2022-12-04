//
// Created by sivin on 12/3/22.
//

#include "SNTimer.h"
#include <chrono>
#include <thread>

int64_t SNTimer::getSteadyTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

int64_t SNTimer::getSteadyTimeUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

int64_t SNTimer::getSystemTimeMs() {

    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

int64_t SNTimer::getSystemTimeUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

void SNTimer::sleepMs(int ms) {
    std::chrono::milliseconds duration(ms);
    std::this_thread::sleep_for(duration);
}

void SNTimer::sleepUs(int us) {
    std::chrono::microseconds duration(us);
    std::this_thread::sleep_for(duration);
}
