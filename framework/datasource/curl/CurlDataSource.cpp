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
        //TODO:Sivin 多线程open处理
        mOpenTimeMs = SNTimer::getSteadyTimeMs();
        bool isRtmp = mUrl.compare(0, 7, "rtmp://") == 0;
        mUri = (isRtmp ? (mUrl + " live=1") : mUrl);
        mConnection = initConnection();
        mConnection->setInterrupt(mInterrupt);
        mConnection->setResume(mRangeStart != INT64_MIN ? mRangeStart : 0);
        SN_LOGI("startConnect...\n");
        int ret = mConnection->startConnect();
        if (ret != 0) {
            mOpenTimeMs = 0;
            return ret;
        }
        //TODO:Sivin 连接失败错误处理
        mOpenTimeMs = SNTimer::getSteadyTimeMs() - mOpenTimeMs;
        return 0;
    }

    int64_t CurlDataSource::read(void *outBuffer, size_t size) {

        if (mRangeEnd != INT64_MIN || mFileSize > 0) {
            int64_t end = mFileSize;
            if (mRangeEnd > 0) {
                end = mRangeEnd;
            }
            end = std::min(mFileSize, end);
            if (end > 0) {
                size = std::min(size, (size_t) (end - mConnection->tell()));
                if (size <= 0) {
                    return 0;
                }
            }
        }
        int64_t ret = 0;
        /* only request 1 byte, for truncated reads (only if not eof) */
        if (mFileSize <= 0 || mConnection->tell() < mFileSize) {
            ret = mConnection->fillBuffer(1, mNeedReconnect);
            if (mNeedReconnect) {
//                closeConnections(false, true);
//                mConnection->setReconnect(true);
//                int64_t pos = mConnection->tell();
//                int64_t seekRet = seek(pos, SEEK_SET);
//                assert(seekRet == pos);
            }
            if (ret < 0) {
                SN_LOGE("CurlDataSource2::Read ret=%d", ret);
                return (int) ret;
            }
        }

        ret = mConnection->readBuffer(outBuffer, size);
        if (ret < 0) {
            SN_LOGE("CurlDataSource2::Read ret=%d", ret);
        }
        return ret;
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

    std::string CurlDataSource::getUri() {
        return mUri;
    }


}