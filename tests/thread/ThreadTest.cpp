//
// Created by Sivin on 2022-11-26.
//

#define LOG_TAG "ThreadTest"

#include <thread>
#include <iostream>
#include "utils/NSLog.h"
#include <utils/SNThread.h>

using namespace Sivin;

int main() {

    int i = 0;
    auto *thread = new SNThread([&i]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SN_LOGD("i = %d", i++);
        return 0;
    }, "testThread");

    thread->setBeginCallback([](){
        SN_LOGD("thread start..");
    });

    thread->setEndCallback([]{
        SN_LOGD("thread end..");
    });

    thread->start();
    std::this_thread::sleep_for(std::chrono::seconds(7));
    thread->pause();
    std::this_thread::sleep_for(std::chrono::seconds(7));
    thread->stop();
    std::this_thread::sleep_for(std::chrono::seconds(7));
    std::cout << "test end " <<std::endl;
    return 0;
}