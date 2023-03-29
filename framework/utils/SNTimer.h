//
// Created by sivin on 12/3/22.
//

#ifndef SN_FRAMEWORK_SNTIMER_H
#define SN_FRAMEWORK_SNTIMER_H
#include <cstdint>

class SNTimer {

public:
    static int64_t getSteadyTimeMs();
    static int64_t getSteadyTimeUs();
    static int64_t getSystemTimeMs();
    static int64_t getSystemTimeUs();

    static void sleepMs(int ms);
    static void sleepUs(int us);

};


#endif //SN_FRAMEWORK_SNTIMER_H
