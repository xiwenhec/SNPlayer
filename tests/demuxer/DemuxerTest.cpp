//
// Created by sivin on 1/1/23.
//

#include "data_source/IDataSource.h"
#include <memory>
#define LOG_TAG "DemuxerTest"

#include "data_source/DataSourceFactory.h"
#include "data_source/curl/CurlDataSource.h"
#include "demuxer/DemuxerService.h"
#include "utils/SNLog.h"
#include "utils/SNTimer.h"

using namespace Sivin;

int main() {
  curl_global_init(CURL_GLOBAL_ALL);
  std::string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
  auto dataSource = std::shared_ptr<IDataSource>(DataSourceFactory::create(url));
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
  std::unique_ptr<SNPacket> packet;
  demuxerService->openStream(0);

  int64_t start = SNTimer::getSteadyTimeMs();

  do {
    ret = demuxerService->readPacket(packet, 0);
    if (ret > 0) {
      SN_LOGI("get packet: pts = %d", packet->getInfo().pts);
      if (packet->getInfo().pts / 1000000 > 100) {
      }
    } else if (ret == 0) {
      SNTimer::sleepUs(5);
    } else if (ret == -1) {
      SN_LOGI("stream finish to end");
    }
  } while (ret >= 0);

  start = SNTimer::getSteadyTimeMs() - start;

  SN_LOGI("test end...:time = %ldms", start);
}