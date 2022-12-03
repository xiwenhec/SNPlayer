//
// Created by sivin on 12/3/22.
//

#ifndef DATASOURCETEST_SNTIMER_H
#define DATASOURCETEST_SNTIMER_H
#include <cstdint>

class SNTimer {

public:
    static int64_t getSteadyTimeMs();
    static int64_t getSteadyTimeUs();
    static int64_t getSystemTimeMs();
    static int64_t getSystemTimeUs();

    static void sleepMS(int ms);
    static void sleepUs(int us);

};


#endif //DATASOURCETEST_SNTIMER_H
