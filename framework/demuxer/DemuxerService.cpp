//
// Created by sivin on 12/6/22.
//

#include "DemuxerService.h"
#include "demuxer/DemuxerPrototype.h"
#include "utils/SNLog.h"
#define  MAX_PROBE_SIZE 1024


namespace Sivin {

    DemuxerService::DemuxerService(std::shared_ptr<IDataSource> dataSource)
            : mDataSource(std::move(dataSource)) {
    }

    int DemuxerService::createDemuxer(IDemuxer::DemuxerType demuxerType) {

        if (mDemuxer == nullptr) {
            std::string url;
            if (mDataSource) {
                url = mDataSource->getUri();
            }
            mDemuxer = DemuxerPrototype::createDemuxer(url);
            if (!mDemuxer) {
                SN_LOGE("create demuxer error\n");
                return -1;
            }
        }


        return 0;
    }
}