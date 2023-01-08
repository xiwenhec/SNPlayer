//
// Created by sivin on 1/1/23.
//

#define LOG_TAG "DemuxerTest"

#include "utils/NSLog.h"
#include "demuxer/DemuxerService.h"
#include "datasource/curl/CurlDataSource.h"
#include "datasource/DataSourceFactory.h"
#include "utils/SNTimer.h"

using namespace Sivin;

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    std::string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    auto dataSource = DataSourceFactory::create(url);
    int ret = dataSource->open(0);

    if (ret < 0) {
        SN_LOGE("Couldn't open stream: uri = %s;", dataSource->getUri().c_str());
        return 0;
    }

    std::unique_ptr<DemuxerService> demuxerService = std::make_unique<DemuxerService>(dataSource);
    ret = demuxerService->initOpen(IDemuxer::DEMUXER_TYPE_BITSTREAM);
    if (ret < 0) {
        SN_LOGE("demuxer service create failed");
        return 0;
    }
    demuxerService->start();
    std::unique_ptr<ISNPacket> packet;
    demuxerService->openStream(0);
    do {
        ret = demuxerService->readPacket(packet, 0);
        if (ret > 0) {
            SN_LOGI("get packet: pts = %d", packet->getInfo().pts);
        } else if (ret == 0) {
            SNTimer::sleepUs(10);
        } else if (ret == -1) {
            SN_LOGI("stream finish to end");
        }
    } while (ret >= 0);

    SN_LOGI("test end...");
}