//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_DEMUXERSERVICE_H
#define DATASOURCETEST_DEMUXERSERVICE_H

#include "data_source/IDataSource.h"
#include "demuxer/IDemuxer.h"
#include <cstdint>
#include <memory>

namespace Sivin {

  class DemuxerService {

  public:
    explicit DemuxerService(std::shared_ptr<IDataSource> dataSource);

    int createDemuxer(IDemuxer::DemuxerType demuxerType);

    int initOpen(IDemuxer::DemuxerType type);

    int start();

    int openStream(int index);

    int readPacket(std::unique_ptr<ISNPacket> &packet, int index = -1);

    int64_t seek(int64_t us, int flags, int index);

  private:
    static int read_callback(void *userArgs, uint8_t *buffer, int size);

    static int64_t seek_callback(void *userArgs, int64_t offset, int whence);

  private:
    std::shared_ptr<IDemuxer> mDemuxer{nullptr};

    std::shared_ptr<IDataSource> mDataSource{nullptr};

    bool mNoFile{false};

    std::shared_ptr<void> mReadArg{nullptr};

    IDemuxer::ReadCallback mReadCb{nullptr};
  };

}// namespace Sivin

#endif//DATASOURCETEST_DEMUXERSERVICE_H
