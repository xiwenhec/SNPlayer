//
// Created by sivin on 11/28/22.
//
#define LOG_TAG "CurlDataSource"

#include <utils/SNLog.h>
#include "CurlDataSource.h"
#include "utils/SNTimer.h"

namespace Sivin {
    CurlDataSource::CurlDataSource(const std::string &url) : IDataSource(url) {
        mConnectManager = std::make_shared<CurlConnectionManager>();

    }

    CurlDataSource::~CurlDataSource() {

    }


    int CurlDataSource::open(int flags) {
        //TODO:Sivin 多线程处理
        mOpenTimeMs = SNTimer::getSteadyTimeMs();
        bool isRtmp = mUrl.compare(0, 7, "rtmp://") == 0;
        mUri = (isRtmp ? (mUrl + " live=1") : mUrl);
        mConnection = initConnection();
        mConnection->setInterrupt(mInterrupt);
        mConnection->setResume(mRangeStart != INT64_MIN ? mRangeStart : 0);
        mConnection->startConnect();
        //TODO:Sivin 连接失败错误处理
        mOpenTimeMs = SNTimer::getSteadyTimeMs() - mOpenTimeMs;
        return 0;
    }

    int CurlDataSource::read(void *outBuffer, size_t size) {


        return 0;
    }


    int64_t CurlDataSource::seek(int64_t offset, int whence) {


        return 0;
    }

    void CurlDataSource::close() {


    }

    std::shared_ptr<CurlConnection> CurlDataSource::initConnection() {
        auto connection = std::make_shared<CurlConnection>(
                std::shared_ptr<SourceConfig>(&mConfig),
                mConnectManager,
                nullptr);
        connection->setSource(mUri, mHeaderList);
        connection->setPost(false, nullptr, 0);
        return std::move(connection);
    }


}