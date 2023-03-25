//
// Created by sivin on 11/28/22.
//
#define LOG_TAG "DataSourceTest"

#include <iostream>
#include <string>
#include <thread>
#include "curl/curl.h"
#include "data_source/DataSourceFactory.h"
#include "utils/SNLog.h"
#include <memory>
#include "utils/SNTimer.h"

using namespace Sivin;
int size = 0;
bool hasSeek = false;

void readThreadFunc(IDataSource *dataSource) {
    while (true) {
        char buffer[1024 * 32];
        int ret = dataSource->read(buffer, sizeof(buffer));
        if (ret <= 0) break;
        size += ret;
        if (size > 80000 && !hasSeek) {
            hasSeek = true;
            SN_LOGD("size = %ld", size);
            dataSource->seek(80527700, SEEK_SET);
            size = 80527700;
        } else {
            SN_LOGD("size = %ld", size);
        }
        SNTimer::sleepMs(10);
    }
}


void testRead(IDataSource *dataSource) {
    std::thread readThread{readThreadFunc, std::ref(dataSource)};
    readThread.join();
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    std::string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    auto dataSource = DataSourceFactory::create(url);
    dataSource->open(0);
    testRead(dataSource);
    SNTimer::sleepMs(1000 * 1000);
    SN_LOGD("exit success ...");
    return 0;
}