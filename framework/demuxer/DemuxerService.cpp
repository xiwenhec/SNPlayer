//
// Created by sivin on 12/6/22.
//

#include "demuxer/IDemuxer.h"
#include <cstdint>
#include <memory>
#define LOG_TAG "DemuxerService"

#include "DemuxerService.h"
#include "demuxer/DemuxerFactory.h"
#include "utils/SNLog.h"

#define MAX_PROBE_SIZE 1024

#define CHECK_DEMUXER        \
  do {                       \
    if (mDemuxer == nullptr) \
      return -1;             \
  } while (false);
#define CHECK_DEMUXER_V      \
  do {                       \
    if (mDemuxer == nullptr) \
      return;                \
  } while (false);

namespace Sivin {

  DemuxerService::DemuxerService(std::shared_ptr<IDataSource> dataSource)
      : mDataSource(std::move(dataSource)) {
  }

  int DemuxerService::initOpen(IDemuxer::DemuxerType type) {
    SN_TRACE;
    if (mDemuxer == nullptr) {
      createDemuxer(type);
    }

    if (mDemuxer == nullptr) {
      SN_LOGE("create demuxer error");
      return -1;
    }

    if (mDataSource != nullptr) {
      mDemuxer->setDataCallback(read_callback, seek_callback, this);
    }
    int ret = mDemuxer->open();
    if (ret < 0) {
      SN_LOGE("mDemuxer open failed");
      return ret;
    }
    return 0;
  }


  int DemuxerService::createDemuxer(IDemuxer::DemuxerType demuxerType) {
    if (mDemuxer == nullptr) {
      std::string url;
      if (mDataSource) {
        url = mDataSource->getUri();
      }
      mDemuxer = std::shared_ptr<IDemuxer>(DemuxerFactory::createDemuxer(url));
      if (!mDemuxer) {
        SN_LOGE("create demuxer error");
        return -1;
      }
    }
    return 0;
  }


  int DemuxerService::read_callback(void *userArgs, uint8_t *buffer, int size) {
    auto demuxerService = (DemuxerService *) userArgs;
    return demuxerService->mDataSource->read(buffer, size);
  }

  int64_t DemuxerService::seek_callback(void *userArgs, int64_t offset, int whence) {
    auto demuxerService = (DemuxerService *) userArgs;
    return demuxerService->mDataSource->seek(offset, whence);
  }

  int DemuxerService::start() {
    SN_TRACE;
    CHECK_DEMUXER;
    mDemuxer->start();
    return 0;
  }

  int DemuxerService::openStream(int index) {
    SN_TRACE;
    CHECK_DEMUXER;
    return mDemuxer->openStream(index);
  }


  int DemuxerService::readPacket(std::unique_ptr<ISNPacket> &packet, int index) {
    CHECK_DEMUXER;
    return mDemuxer->readPacket(packet, index);
  }

  int64_t DemuxerService::seek(int64_t us, int flags, int index) {
    //TODO:
    return 0;
  }

}// namespace Sivin