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

        int createDemuxer(IDemuxer::DemuxerType demuxerType);

    private:
        std::unique_ptr<IDemuxer> mDemuxer{nullptr};
        std::shared_ptr<IDataSource> mDataSource{nullptr};

        uint8_t *mProbBuffer{nullptr};
        int mProbBufferSize{0};

        bool mNoFile{false};

        std::shared_ptr<void> mReadArg{nullptr};
        IDemuxer::ReadCallback mReadCb{nullptr};
    };

}

#endif //DATASOURCETEST_DEMUXERSERVICE_H
