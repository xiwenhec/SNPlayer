//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_DEMUXERSERVICE_H
#define DATASOURCETEST_DEMUXERSERVICE_H

#include "demuxer/IDemuxer.h"
#include "datasource/IDataSource.h"

namespace Sivin {

    class DemuxerService {

    public:
        explicit DemuxerService(std::shared_ptr<IDataSource> dataSource);

        int initOpen(IDemuxer::DemuxerType type);

        int start();

        int openStream(int index);

        int readPacket(std::unique_ptr<ISNPacket> &packet, int index = -1);

    private:

        static int read_callback(void *userArgs, uint8_t *buffer, int size);

        static int64_t seek_callback(void *userArgs, int64_t offset, int whence);

        int createDemuxer(IDemuxer::DemuxerType demuxerType);

    private:
        std::unique_ptr<IDemuxer> mDemuxer{nullptr};
        std::shared_ptr<IDataSource> mDataSource{nullptr};

        bool mNoFile{false};

        std::shared_ptr<void> mReadArg{nullptr};
        IDemuxer::ReadCallback mReadCb{nullptr};
    };

}

#endif //DATASOURCETEST_DEMUXERSERVICE_H
