//
// Created by Sivin on 2022-11-26.
//

#define LOG_TAG "ThreadTest"

#include <thread>
#include <iostream>
#include "utils/SNLog.h"


int main() {

    NS_LOGD("hello world\n");
    NS_LOGE("hello world\n");
    NS_LOGI("hello world\n");
    NS_TRACE;

    std::cout << "test end " << std::this_thread::get_id() << std::endl;

    return 0;
}