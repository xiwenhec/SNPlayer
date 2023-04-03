//
// Created by sivin on 12/6/22.
//

#ifndef SN_FRAMEWORK_DEMUXERSERVICE_H
#define SN_FRAMEWORK_DEMUXERSERVICE_H

#include "data_source/IDataSource.h"
#include "demuxer/IDemuxer.h"
#include <cstdint>
#include <memory>

namespace Sivin {

  class DemuxerService {

  public:
    explicit DemuxerService(std::unique_ptr<IDataSource> &dataSource);

    int createDemuxer(IDemuxer::DemuxerType demuxerType);

    int initOpen(IDemuxer::DemuxerType type);

    int start();

    int openStream(int index);

    SNRet readPacket(std::unique_ptr<SNPacket> &packet, int index);

    int64_t seek(int64_t us, int flags, int index);

  public:
    int getMediaInfo(std::unique_ptr<SNMeidaInfo> &mediaInfo);

    int getNbStreams() const;

    int getStreamInfo(std::unique_ptr<SNStreamInfo> &streamInfo, int index);

    const std::unique_ptr<IDemuxer> &getDemuxer() {
      return mDemuxer;
    }

  private:
    static int read_callback(void *userArgs, uint8_t *buffer, int size);

    static int64_t seek_callback(void *userArgs, int64_t offset, int whence);

  private:
    std::unique_ptr<IDemuxer> mDemuxer{nullptr};
    const std::unique_ptr<IDataSource> &mDataSource;

    bool mNoFile{false};

    std::shared_ptr<void> mReadArg{nullptr};

    IDemuxer::ReadCallback mReadCb{nullptr};
  };

}// namespace Sivin

#endif//SN_FRAMEWORK_DEMUXERSERVICE_H
