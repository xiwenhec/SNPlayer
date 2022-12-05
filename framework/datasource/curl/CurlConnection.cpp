//
// Created by sivin on 11/28/22.
//

#define LOG_TAG "CurlConnection"

#include "CurlConnection.h"

#include <utility>
#include <utils/SNLog.h>
#include <utils/SNTimer.h>
#include "datasource/curl/CurlConnectionManager.h"

#define MAX_HEADER_SIZE 1024
#define RINGBUFFER_SIZE (1024 * 256)
#define RINGBUFFER_BACK_SIZE (1024 * 512)

#define CURL_READ_BUFFER_SIZE CURL_MAX_WRITE_SIZE//(1024 * 16)

#define TRUE 1L
#define FALSE 0L

namespace Sivin {

    static int getErrorCode(const CURLcode &CURLResult) {
        //TODO:Sivin ERROR CODE;
        return -1;
    }

    CurlConnection::CurlConnection(std::shared_ptr<IDataSource::SourceConfig> config,
                                   std::shared_ptr<CurlConnectionManager> connectionManager,
                                   std::shared_ptr<ConnectionListener> listener) :
            mConfig(std::move(config)),
            mConnectionManager(std::move(connectionManager)),
            mListener(std::move(listener)),
            mBuffer(std::make_shared<RingBuffer>(RINGBUFFER_SIZE + RINGBUFFER_BACK_SIZE,
                                                 RINGBUFFER_BACK_SIZE)) {
        isFirstLoop = true;
        mHttpHandle = curl_easy_init();

        if (mConfig != nullptr) {
            if (mConfig->lowSpeedLimit && mConfig->lowSpeedTimeMs) {
                NS_LOGD("set lowSpeedLimit to %d\n", mConfig->lowSpeedLimit);
                NS_LOGD("set lowSpeedTime to %d(ms)\n", mConfig->lowSpeedTimeMs);
                curl_easy_setopt(mHttpHandle, CURLOPT_LOW_SPEED_LIMIT, (long) mConfig->lowSpeedLimit);
                curl_easy_setopt(mHttpHandle, CURLOPT_LOW_SPEED_TIME, (long) mConfig->lowSpeedTimeMs / 1000);
            }

            if (mConfig->connectTimeoutMs) {
                NS_LOGD("set connect time to %d(ms)\n", mConfig->connectTimeoutMs);
                curl_easy_setopt(mHttpHandle, CURLOPT_CONNECTTIMEOUT, (long) mConfig->connectTimeoutMs / 1000);
            }
        }
        setEasyHandleCommonOpt();
    }


    CurlConnection::~CurlConnection() {
        if (mHttpHandle) {
            curl_easy_cleanup(mHttpHandle);
            mHttpHandle = nullptr;
        }

        delete[] mResponseHeader;
        mResponseHeader = nullptr;
    }


    void CurlConnection::setResume(int64_t pos) {
        mFilePos = pos;
        if (sendRange && this->mFilePos == 0) {
            curl_easy_setopt(mHttpHandle, CURLOPT_RANGE, "0-");
        } else {
            curl_easy_setopt(mHttpHandle, CURLOPT_RANGE, NULL);
            sendRange = 0;
        }
        curl_easy_setopt(mHttpHandle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t) mFilePos);
    }


    void CurlConnection::clearCurlOpt() {
        if (mHttpHandle) {
            //TODO:Sivin
            curl_easy_setopt(mHttpHandle, CURLOPT_VERBOSE, FALSE);
            curl_easy_setopt(mHttpHandle, CURLOPT_WRITEDATA, nullptr);
            //   curl_easy_setopt(mHttp_handle, CURLOPT_WRITEFUNCTION, nullptr);
            //    curl_easy_setopt(mHttp_handle, CURLOPT_HEADERFUNCTION, nullptr);
            curl_easy_setopt(mHttpHandle, CURLOPT_HEADERDATA, nullptr);
            //  curl_easy_setopt(mHttp_handle, CURLOPT_SOCKOPTFUNCTION, nullptr);
            curl_easy_setopt(mHttpHandle, CURLOPT_SOCKOPTDATA, nullptr);
            // curl_easy_setopt(mHttp_handle, CURLOPT_DEBUGFUNCTION, nullptr);
            curl_easy_setopt(mHttpHandle, CURLOPT_DEBUGDATA, nullptr);
        }
    }

    void CurlConnection::setEasyHandleCommonOpt() {
        curl_easy_setopt(mHttpHandle, CURLOPT_VERBOSE, TRUE);
        curl_easy_setopt(mHttpHandle, CURLOPT_FOLLOWLOCATION, 1L);
        //最大重定向次数
        curl_easy_setopt(mHttpHandle, CURLOPT_MAXREDIRS, 5L);
        curl_easy_setopt(mHttpHandle, CURLOPT_NOSIGNAL, 1L);
        //设置curl接收缓冲区大小，默认是16k，最大不能超过512k
        curl_easy_setopt(mHttpHandle, CURLOPT_BUFFERSIZE, CURL_READ_BUFFER_SIZE);
        //如果设置为1则会阻止xferinfo函数调用，
        curl_easy_setopt(mHttpHandle, CURLOPT_NOPROGRESS, 0);

        curl_easy_setopt(mHttpHandle, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(mHttpHandle, CURLOPT_WRITEDATA, this);

        curl_easy_setopt(mHttpHandle, CURLOPT_HEADERFUNCTION, headerCallback);
        curl_easy_setopt(mHttpHandle, CURLOPT_HEADERDATA, this);

        curl_easy_setopt(mHttpHandle, CURLOPT_SOCKOPTFUNCTION, sockoptCallback);
        curl_easy_setopt(mHttpHandle, CURLOPT_SOCKOPTDATA, this);

        curl_easy_setopt(mHttpHandle, CURLOPT_XFERINFOFUNCTION, xferinfo);
        curl_easy_setopt(mHttpHandle, CURLOPT_XFERINFODATA, this);

        curl_easy_setopt(mHttpHandle, CURLOPT_DEBUGFUNCTION, connectTrace);
        curl_easy_setopt(mHttpHandle, CURLOPT_DEBUGDATA, this);

        curl_easy_setopt(mHttpHandle, CURLOPT_SSL_VERIFYPEER, FALSE);
        // openssl not verify host, otherwise will return CURLE_PEER_FAILED_VERIFICATION
        curl_easy_setopt(mHttpHandle, CURLOPT_SSL_VERIFYHOST, FALSE);
    }

    size_t CurlConnection::writeCallback(char *buffer, size_t size, size_t nmemb, void *userdata) {
        if (!userdata) {
            return CURL_WRITEFUNC_PAUSE;
        }
        auto *connect = (CurlConnection *) userdata;
        auto amount = (uint32_t) (size * nmemb);

        assert(!connect->mPaused);

        std::lock_guard<std::mutex> lock{connect->mMutex};
        //缓存空间小于传输数据，则暂停传输
        if (connect->mBuffer->getMaxWriteableDataSize() < amount) {
            connect->mPaused = true;
            return CURL_WRITEFUNC_PAUSE;
        }

        if (connect->mBuffer->writeData(buffer, amount) != amount) {
            NS_LOGE("write ring buffer error %u %u\n", amount, connect->mBuffer->getMaxWriteableDataSize());
            assert(0);
        }

        if (connect->mConfig && connect->mConfig->listener) {
            connect->mConfig->listener->onNetWorkInput(amount, IDataSource::Listener::BITSTREAM_TYPE_MEDIA);
        }
        return amount;
    }

    size_t CurlConnection::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
        if (!userdata) {
            return size * nitems;
        }
        auto *connect = (CurlConnection *) userdata;
        if (connect->mResponseHeader == nullptr) {
            connect->mResponseHeader = new char[MAX_HEADER_SIZE];
            memset(connect->mResponseHeader, 0, MAX_HEADER_SIZE);
            connect->mResponseHeaderSize = 0;
        }

        if (connect->mResponseHeaderSize + size * nitems < MAX_HEADER_SIZE) {
            memcpy(connect->mResponseHeader + connect->mResponseHeaderSize, buffer, size * nitems);
            connect->mResponseHeaderSize += size * nitems;
        } else {
            NS_LOGE("responseHeader filed can not hold header response: "
                    "headerSize = %ld, size * nitems = %ld, maxHaderSize = %ld",
                    connect->mResponseHeaderSize, size * nitems, MAX_HEADER_SIZE);
        }

        return size * nitems;
    }

    int CurlConnection::sockoptCallback(void *clientp, curl_socket_t curlfd, curlsocktype purpose) {
        auto *connect = (CurlConnection *) clientp;
        if (purpose == CURLSOCKTYPE_IPCXN) { //表示socket创建成功，CURLSOCKTYPE_IPCXN表示主动创建
            int rcv_buffer = connect->mConfig ? connect->mConfig->sockReceiveBufferSize : 0;
            if (rcv_buffer > 0) {
                setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (const char *) &rcv_buffer, sizeof(rcv_buffer));
            }

            if (connect->mListener) {
                connect->mListener->onDNSResolved();
            }
            connect->mDNSResolved = true;
        }
        return CURL_SOCKOPT_OK;
    }

    int CurlConnection::connectTrace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp) {
        const char *text;
        auto connect = (CurlConnection *) userp;
        switch (type) {
            case CURLINFO_TEXT:
                NS_LOGD("curl info : %s", data);
                break;

            case CURLINFO_HEADER_OUT:
                text = "=> Send header";
                break;
            case CURLINFO_DATA_OUT:
                text = "=> Send data";
                break;
            case CURLINFO_SSL_DATA_OUT:
                text = "=> Send SSL data";
                break;
            case CURLINFO_HEADER_IN:
                text = "<= Recv header";
                break;
            case CURLINFO_DATA_IN:
                text = "<= Recv data";
                break;
            case CURLINFO_SSL_DATA_IN:
                text = "<= Recv SSL data";
                break;
            default:
                return 0;
        }
        return 0;
    }

    //CURL 传输数据回调：
    //dltotal:期望这次传输中下载的总字节数，dlnow目前已经下载的字节数，ultotal:期望上传的总字节数，ulnow：目前已经上传的总字节数
    int CurlConnection::xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                                 curl_off_t ulnow) {
        if (!clientp) {
            return 0;
        }
        auto *connect = (CurlConnection *) clientp;
        //暂停结束
        if (connect->mPaused && connect->mBuffer->getMaxWriteableDataSize() > CURL_READ_BUFFER_SIZE) {
            connect->mPaused = false;
            curl_easy_pause(connect->mHttpHandle, CURLPAUSE_CONT);
        }
        return 0;
    }


    void CurlConnection::setSource(const std::string &uri, curl_slist *headerList) {
        if (headerList) {
            curl_easy_setopt(mHttpHandle, CURLOPT_HTTPHEADER, headerList);
        } else {
            curl_easy_setopt(mHttpHandle, CURLOPT_HTTPHEADER, NULL);
        }
        mUri = uri;
        curl_easy_setopt(mHttpHandle, CURLOPT_URL, uri.c_str());

        //TODO:Sivin
//        CURLSH *sh = nullptr;
//        if (reSolveList) {
//            curl_slist_free_all(reSolveList);
//        }
//
//        reSolveList = CURLShareInstance::Instance()->getHosts(uri, &sh);
//        assert(sh != nullptr);
//        curl_easy_setopt(mHttpHandle, CURLOPT_SHARE, sh);
//
//        if (reSolveList != nullptr) {
//            curl_easy_setopt(mHttpHandle, CURLOPT_RESOLVE, reSolveList);
//        }
    }

    void CurlConnection::setPost(bool isPost, const uint8_t *data, int64_t size) {
        if (isPost) {
            curl_easy_setopt(mHttpHandle, CURLOPT_POST, 1);
            curl_easy_setopt(mHttpHandle, CURLOPT_POSTFIELDS, data);
            curl_easy_setopt(mHttpHandle, CURLOPT_POSTFIELDSIZE, (long) size);
        } else {
            curl_easy_setopt(mHttpHandle, CURLOPT_POST, 0);
        }
    }

    void CurlConnection::onConnectDone(bool eos, CURLcode code) {
        mEos = eos;
        mStatus = code;
        mStillRunning = false;
    }

    CURL *CurlConnection::getCurlHandle() const {
        return mHttpHandle;
    }

    //TODO:Sivin 待优化
    int64_t CurlConnection::fillBuffer(int64_t want, const std::atomic<bool> &needReconnect) {
        int64_t startTime = SNTimer::getSteadyTimeMs();
        //需要继续等待填充数据
        while (mBuffer->getMaxReadableDataSize() < want && mBuffer->getMaxWriteableDataSize() > 0) {
            //TODO:Sivin needReconnect这里有什么含义
            if (mInterrupted || needReconnect) {
                //TODO:Sivin 处理错误码
                NS_LOGW("connect error exit");
                return -1;
            }
            if (mEos) { //流结束，直接返回填充数据成功
                return 0;
            }

            if (mPaused && mBuffer->getMaxWriteableDataSize() > CURL_READ_BUFFER_SIZE) {
                mPaused = false;
                mConnectionManager->resumeConnection(shared_from_this());
            }

            CURLcode code = mStatus;
            bool error = code != CURLE_OK;
            switch (code) {
                case CURLE_OK:
                    break;
                case CURLE_OPERATION_TIMEDOUT:
                case CURLE_PARTIAL_FILE:
                case CURLE_COULDNT_CONNECT:
                case CURLE_RECV_ERROR:
                case CURLE_COULDNT_RESOLVE_HOST:
                case CURLE_HTTP2:
                    error = false;
                    break;
                case CURLE_HTTP_RETURNED_ERROR:
                    long httpCode;
                    curl_easy_getinfo(mHttpHandle, CURLINFO_RESPONSE_CODE, &httpCode);
                    break;
                default:
                    break;
            }

            if (error) {
                if (mBuffer->getMaxReadableDataSize() > 0) {
                    return 0;
                }
                //TODO:Sivin handle error
                return -1;
            }

            bool reconnect = false;
            //处理不等于OK，但是认为不是错误情况
            if (code != CURLE_OK) {
                //connectManger内部会会同步删除出错或者流结束的connection
                reset();
                if (mConfig && mConfig->listener) {
                    IDataSource::Listener::NetWorkRetryStatus status;
                    do {
                        status = mConfig->listener->onNetWorkRetry(getErrorCode(code));
                        if (mInterrupted) {
                            //TODO:Sivin处理错误码
                            return -1;
                        }
                        SNTimer::sleepMs(10);
                    } while (status == IDataSource::Listener::NETWORK_RETRY_STATUS_PENDING);
                    //TODO:Sivin 一定要放到这里吗？
                    reconnect = true;
                } else if (mConfig && SNTimer::getSteadyTimeMs() - startTime > mConfig->connectTimeoutMs) {
                    mFilePos = 0;
                    //TODO:Sivin 处理错误码
                    return -1;
                }
                SNTimer::sleepMs(10);
                setResume(mFilePos);
                applyReconnect(true);
                addToManager();
                continue;
            }

            if (reconnect || isFirstLoop) {
                double length = 0.0;
                if (curl_easy_getinfo(mHttpHandle, CURLINFO_SIZE_DOWNLOAD, &length) == CURLE_OK) {
                    if (length > 0) {
                        reconnect = false;
                        if (mConfig && mConfig->listener) {
                            mConfig->listener->onNetWorkConnected();
                        }
                    }
                }
            }

            if (isFirstLoop && mBuffer->getMaxReadableDataSize() > 0) {
                isFirstLoop = false;
            }
        }

        if (mFileSize < 0) {
            double length;
            if (CURLE_OK == curl_easy_getinfo(mHttpHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length)) {
                if (length > 0.0) {
                    mFileSize = mFilePos + (int64_t) length;
                    NS_LOGI("mFileSize = %llf", length);
                } else {
                    mFileSize = 0;
                }
            }
        }
        return 0;
    }

    void CurlConnection::reset() {
        if (mBuffer) mBuffer->clear();

        mEos = false;
        mStatus = CURLE_OK;

        mResponseHeaderSize = 0;
        delete[] mResponseHeader;
        mResponseHeader = nullptr;
    }

    void CurlConnection::applyReconnect(bool reconnect) {
        curl_easy_setopt(mHttpHandle, CURLOPT_FRESH_CONNECT, reconnect);
    }

    void CurlConnection::addToManager() {
        mConnectionManager->addConnection(shared_from_this());
        mStillRunning = true;
    }

    void CurlConnection::removeFromManager() {
        mConnectionManager->removeConnection(shared_from_this());
    }

    void CurlConnection::setInterrupt(const std::atomic_bool &interrupt) {
        mInterrupted.store(interrupt);
    }

    int64_t CurlConnection::readBuffer(void *outBuffer, int64_t size) {
        std::lock_guard<std::mutex> lockGuard{mMutex};
        int64_t want = std::min(mBuffer->getMaxReadableDataSize(), size);
        if (want > 0 && mBuffer->readData((char *) outBuffer, want) == want) {
            mFilePos += want;
            return want;
        }
        /* check if we finished prematurely */
        if (!mStillRunning && (mFileSize > 0 && mFilePos != mFileSize)) {
            NS_LOGE("%s - Transfer ended before entire file was retrieved pos %lld, size %lld", __FUNCTION__, mFilePos,
                    mFileSize);
            //   return -1;
        }
        return 0;
    }

    int64_t CurlConnection::shortSeek(int64_t off) {
        int64_t delta = off - mFilePos;
        std::lock_guard<std::mutex> lockGuard{mMutex};
        //向后回退
        if (delta < 0) {
            if (mBuffer->skipBytes(delta)) {
                mFilePos = off;
                return 0;
            }
            return -1;
        }

        //向前seek
        if (mBuffer->skipBytes(delta)) {
            mFilePos = off;
            return 0;
        }

        //向前seek失败，表示可读取空间小于delta
        uint32_t shortBufferSize = 1024 * 64;
        //判断off是否超过设定的阈值,如果超过阈值，则直接判定为无法shortSeek,return -1;
        if (off < mFilePos + shortBufferSize) {
            int64_t ringBufReadSize = mBuffer->getMaxReadableDataSize();
            //先seek缓存空间
            if (ringBufReadSize > 0) {
                mBuffer->skipBytes(ringBufReadSize);
                mFilePos += ringBufReadSize;
            }
            int64_t ret = 0;
            if ((ret = fillBuffer(shortBufferSize, mNeedReconnect)) < 0) {
                //获取shortBufferSize数据失败,撤销之前的缓存回退
                if (ringBufReadSize && !mBuffer->skipBytes(-ringBufReadSize)) {
                    NS_LOGE("%s - Failed to restore position after failed filled", __FUNCTION__);
                } else {
                    //撤销成功
                    mFilePos -= ringBufReadSize;
                }
                return ret;
            }

            //填充成功
            NS_LOGI("read buffer size = %lld, need is %ld", mBuffer->getMaxReadableDataSize(),
                    (delta - ringBufReadSize));
            if (!mBuffer->skipBytes((delta - ringBufReadSize))) {
                NS_LOGE("%s - Failed to skip to position after having filled buffer", __FUNCTION__);
                if (ringBufReadSize && !mBuffer->skipBytes(-ringBufReadSize)) {
                    NS_LOGE("%s - Failed to restore position after failed filled", __FUNCTION__);
                } else {
                    //撤销成功
                    mFilePos -= ringBufReadSize;
                }
                return -1;
            }
            mFilePos = off;
            return 0;
        }
        return -1;
    }


}