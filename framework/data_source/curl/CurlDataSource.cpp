//
// Created by sivin on 11/28/22.
//
#include <cstdio>

#define LOG_TAG "CurlDataSource"

#include <utils/SNLog.h>
#include "CurlDataSource.h"
#include "utils/SNTimer.h"

namespace Sivin {
    CurlDataSource::CurlDataSource(const std::string &url) : IDataSource(url) {
        mConnectionManager = std::make_shared<CurlConnectionManager>();
    }

    CurlDataSource::~CurlDataSource() {

    }

    int CurlDataSource::open(int flags) {
        auto openTimeMs = SNTimer::getSteadyTimeMs();
        //TODO:Sivin RTMP协议处理
        bool isRtmp = mUrl.compare(0, 7, "rtmp://") == 0;
        mUri = (isRtmp ? (mUrl + " live=1") : mUrl);

        std::unique_lock<std::shared_mutex> lock{mSharedMutex};
        mConnection = initConnection();
        mConnection->setInterrupt(mInterrupt);
        mConnection->setResume(mRangeStart != -1 ? mRangeStart : 0);

        SN_LOGI("startConnect...");
        int ret = mConnection->startConnect();
        if (ret != 0) {
            return ret;
        }
        mFileSize = mConnection->getFileSize();
        openTimeMs = SNTimer::getSteadyTimeMs() - openTimeMs;
        SN_LOGI("Connect success. mFilesize = %ld, cost time = %ldms", mFileSize, openTimeMs);
        return 0;
    }


    int64_t CurlDataSource::read(void *outBuffer, int64_t size) {

        std::shared_lock<std::shared_mutex> lock{mSharedMutex};

        if (mRangeEnd > 0 || mFileSize > 0) {
            int64_t end = std::min(mRangeEnd, mFileSize);
            if (end > 0) {
                size = std::min(size, end - mConnection->tell());
                if (size <= 0) {
                    SN_LOGI("read to eof, pos = %ld, end = %ld", mConnection->tell(), end);
                    return 0; //文件读取结束
                }
            }
        }

        int64_t ret = 0;
        //检测是否可以读取数据
        if (mFileSize <= 0 || mConnection->tell() < mFileSize) {
            ret = mConnection->readCheck(mNeedReconnect);
            if (mNeedReconnect) {
                closeConnection(true);
                mConnection->setReconnect(true);
                int64_t pos = mConnection->tell();
                int64_t seekRet = seek(pos, SEEK_SET);
                assert(seekRet == pos);
            }
            if (ret < 0) {
                SN_LOGE("Could not read data. ret=%d", ret);
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
        std::shared_lock<std::shared_mutex> lock{mSharedMutex};
        if (!mConnection) {
            SN_LOGE("seek error connection = nullptr");
            return -1;
        }
        if (whence == SEEK_SIZE) {
            return mFileSize;
        } else if (((whence == SEEK_CUR && offset == 0) || (whence == SEEK_SET && offset == mConnection->tell()))
                   && !mConnection->needReconnect()) {
            return mConnection->tell();
        } else if (mFileSize <= 0 && whence == SEEK_END) {
            SN_LOGE("seek error,whence = SEEK_END, mFileSize = %ld", mFileSize);
            return -1;
        }
        if (whence == SEEK_CUR) {
            offset += mConnection->tell();
        } else if (whence == SEEK_END) {
            offset += mFileSize;
        } else if (whence != SEEK_SET) {
            return -1;
        }

        //这里及最终计算从0开始的相对位置，因此不应该小于0
        if (offset < 0) {
            return -1;
        }

        if (offset == mConnection->tell() && !mConnection->needReconnect()) {
            return offset;
        }

        //TODO:这里如果offset超过了end，会有什么问题码？
        if (mRangeEnd > 0 || mFileSize > 0) {
            uint64_t endPos = mRangeEnd > 0 ? mRangeEnd : mFileSize;
            if (offset >= endPos) {
                mConnection->setResume(offset);
                return offset;
            }
        }

        if (!mNeedReconnect) {
            if (mConnection->shortSeek(offset) >= 0) {
                SN_LOGI("short seek success: offset = %ld", offset);
                return offset;
            } else {
                SN_LOGI("short seek failed: offset = %ld", offset);
            }
        } else {
            lock.unlock();
            closeConnection(true);
        }
        lock.unlock();
        int64_t ret = trySeekByNewConnection(offset);
        mNeedReconnect = false;
        return ret;
    }

    void CurlDataSource::close() {
    }

    std::shared_ptr<CurlConnection> CurlDataSource::initConnection() {
        auto connection = std::make_shared<CurlConnection>(
                &mConfig,
                mConnectionManager,
                nullptr);
        connection->setSource(mUri, mHeaderList);
        connection->setPost(false, nullptr, 0);
        return std::move(connection);
    }


    void CurlDataSource::closeConnection(bool forbidReuse) {
        std::unique_lock<std::shared_mutex> lock{mSharedMutex};
        mConnection->closeConnection(forbidReuse);
        mConnection = nullptr;
    }

    std::string CurlDataSource::getUri() {
        return mUri;
    }

    int64_t CurlDataSource::trySeekByNewConnection(int64_t offset) {
        std::unique_lock<std::shared_mutex> lock{mSharedMutex};
        if (mConnection) {
            mConnection->closeConnection(false);
        }
        auto connection = initConnection();
        connection->setInterrupt(mInterrupt);
        connection->setResume(mRangeStart != -1 ? mRangeStart : offset);
        SN_LOGI("trySeekByNewConnection start...");
        auto openTime = SNTimer::getSteadyTimeMs();
        int64_t ret = connection->startConnect();
        if (ret == 0) {
            mConnection = connection;
            openTime = SNTimer::getSteadyTimeMs() - openTime;
            SN_LOGI("trySeekByNewConnection success. cost time = %ldms", openTime);
            return offset;
        } else {
            SN_LOGE("trySeekByNewConnection failed.");
            return -1;
        }
    }
}