//
// Created by sivin on 12/3/22.
//
#define LOG_TAG "CurlConnectionManager"

#include "CurlConnectionManager.h"
#include "CurlConnection.h"
#include <memory>
#include <utils/SNLog.h>
#include "utils/SNTimer.h"

namespace Sivin {

    class CurlHandleStatus {
    public:
        bool eof{false};
        CURLcode status{CURLE_OK};
    };

    CurlConnectionManager::CurlConnectionManager() {
        mMultiHandle = curl_multi_init();
        curl_multi_setopt(mMultiHandle, CURLMOPT_PIPELINING, CURLPIPE_NOTHING);
        mLoopThread = MAKE_UNIQUE_THREAD(loop, "CurlConnectionManager");
    }

    CurlConnectionManager::~CurlConnectionManager() {
        if (mMultiHandle) {
            curl_multi_cleanup(mMultiHandle);
        }
    }

    int CurlConnectionManager::loop() {
        applyPending();
        CURLMcode code;
        do {
            code = curl_multi_perform(mMultiHandle, &mStillRunning);
        } while (code == CURLM_CALL_MULTI_PERFORM);

        int msgNum = 0;
        CURLMsg *msg = nullptr;
        CurlHandleStatus status{};
        //读取multiHandle中包含的easy_handle已经完成或者出错的消息
        //多次调用会返回不同的消息，直到msg为NULL
        while ((msg = curl_multi_info_read(mMultiHandle, &msgNum))) {
            CurlConnection *connection = nullptr;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &connection);
            assert(connection != nullptr);
            //当msg->msg == CURLMSG_DONE标识着该传输已经完成
            if (msg->msg == CURLMSG_DONE) {
                //data->result是当前传输完成的easy_handle的返回码
                if (msg->data.result == CURLE_OK) {
                    status.eof = true;
                }
                status.status = msg->data.result;
            } else {
                if (!mStillRunning && msg->data.result == CURLE_OK) {
                    // we assume eof
                    assert(0);
                    status.eof = true;
                    status.status = CURLE_OK;
                    SN_LOGW("assume a abnormal eos\n");
                }
            }

            //该传输通道结束，或者该通道传输出现异常
            if (status.eof || status.status != CURLE_OK) {
                connection->onConnectDone(status.eof, status.status);
                curl_multi_remove_handle(mMultiHandle, connection->getCurlHandle());
            }
        }

        if (!code && mStillRunning) {
            //poll,所有easy_handle,如果没有任何数据，则会阻塞，直到任意一个easy_handle有数据，或者超时
            //或者其他线程调用curl_multi_wakeup
            code = curl_multi_poll(mMultiHandle, nullptr, 0, 1000, nullptr);
        }

        if (code != CURLM_OK) {
            SN_LOGE("curl_multi_poll error %d\n", code);
        }

        if (!mStillRunning) {
            SNTimer::sleepMs(10);
        }
        return 0;
    }

    void CurlConnectionManager::applyPending() {
        {
            std::lock_guard<std::mutex> lockGuard(mMutex);
            for (auto &connect: mAddList) {
                curl_multi_add_handle(mMultiHandle, connect->getCurlHandle());
            }
            mAddList.clear();
        }

        {
            std::lock_guard<std::mutex> lockGuard(mMutex);
            for (auto &connect: mResumeList) {
                curl_multi_add_handle(mMultiHandle, connect->getCurlHandle());
                curl_easy_pause(connect->getCurlHandle(), CURLPAUSE_CONT);
            }
            mResumeList.clear();
        }

        std::list<std::shared_ptr<CurlConnection>> tempList;
        {
            std::lock_guard<std::mutex> lockGuard(mMutex);
            for (auto &connect: mRemoveList) {
                connect->clearCurlOpt();
                tempList.push_back(connect);
            }
            mRemoveList.clear();
        }

        for (auto &connect: tempList) {
            curl_multi_remove_handle(mMultiHandle, connect->getCurlHandle());
        }

    }

    void CurlConnectionManager::addConnection(const std::shared_ptr<CurlConnection> &connection) {
        if (mLoopThread->getStatus() == SNThread::THREAD_STATUS_IDLE) {
            mLoopThread->start();
        }
        std::lock_guard<std::mutex> lockGuard{mMutex};
        for (auto &connect: mRemoveList) {
            if (connection == connect) {
                mRemoveList.remove(connect);
                break;
            }
        }
        curl_easy_setopt(connection->getCurlHandle(), CURLOPT_PRIVATE, connection.get());
        mAddList.push_back(connection);
        //唤醒当前因调用curl_multi_poll而sleep的线程
        curl_multi_wakeup(mMultiHandle);
    }

    void CurlConnectionManager::removeConnection(const std::shared_ptr<CurlConnection> &connection) {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        for (auto &connect: mAddList) {
            if (connect == connection) {
                mAddList.remove(connect);
                break;
            }
        }
        mRemoveList.push_back(connection);
        curl_multi_wakeup(mMultiHandle);
    }

    void CurlConnectionManager::resumeConnection(const std::shared_ptr<CurlConnection> &connection) {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mResumeList.push_back(connection);
        curl_multi_wakeup(mMultiHandle);
    }

} // Sivin