//
// Created by sivin on 1/1/23.
//
#include "utils/NSLog.h"
#include "demuxer/DemuxerService.h"

#include "datasource/curl/CurlDataSource.h"

#include "codec/ICodec.h"
#include "datasource/DataSourceFactory.h"

using namespace Sivin;

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    std::string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    auto dataSource = DataSourceFactory::create(url);
    dataSource->open(0);
    SN_TRACE;
    std::unique_ptr<DemuxerService> demuxerService = std::make_unique<DemuxerService>(dataSource);
    int ret = demuxerService->initOpen(IDemuxer::DEMUXER_TYPE_BITSTREAM);
    if (ret < 0) {
        SN_LOGE("demuxerService initOpen failed;\n");
    }
    demuxerService->openStream(0);
    demuxerService->start();
    std::unique_ptr<ISNPacket> packet;
    do {
        ret = demuxerService->readPacket(packet, 0);
        SN_LOGI("return packet size = %d\n",ret);
    } while (ret > 0);

}