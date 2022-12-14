//
// Created by Sivin on 2022-11-26.
//

#define LOG_TAG "SNThread"

#include "SNThread.h"
#include "NSLog.h"

#include <memory>
#include <utility>
#include <cassert>

#ifdef ANDROID

#include <sys/prctl.h>
#include <cassert>
#include <cstring>
#include <utils/NSLog.h>

#define MAX_TASK_NAME_LEN (16)
#endif


namespace Sivin {


#ifdef ANDROID

    static void getName(char *name) {
        if (prctl(PR_GET_NAME, (unsigned long) name, 0, 0, 0) != 0) {
            strcpy(name, "<name unknown>");
        } else {
            // short names are null terminated by prctl, but the man page
            // implies that 16 byte names are not.
            name[MAX_TASK_NAME_LEN] = 0;
        }
        SN_LOGD("thread_name is %s", name);
    }

    static void setName(const char *name) {
        char threadName[MAX_TASK_NAME_LEN + 1];
        size_t size = strlen(name) > MAX_TASK_NAME_LEN ? MAX_TASK_NAME_LEN : strlen(name);
        strncpy(threadName, name, size);
        threadName[size] = 0;
        prctl(PR_SET_NAME, (unsigned long) threadName, 0, 0, 0);
    }

#endif


    static void threadSetSelfName(const char *name) {
#ifdef __APPLE__
        pthread_setname_np(name);
#elif defined(ANDROID)
        setName(name);
#endif
    }


    SNThread::SNThread(std::function<int()> runFunc, const char *threadName) :
            mUserFunc(std::move(runFunc)), mName(threadName), mThread(nullptr) {}

    void SNThread::start() {
        std::lock_guard<std::mutex> lockGuard{mMutex};
        if (mThread == nullptr) {
            mThread = std::make_unique<std::thread>(&SNThread::threadRun, this);
        } else {
            mWaitCond.notify_one();
        }
        mThreadStatus = THREAD_STATUS_RUNNING;
    }


    void SNThread::threadRun() {
        if (mThreadBeginCallback != nullptr) {
            mThreadBeginCallback();
        }
        assert(mUserFunc != nullptr);

        if (mName.length() > 0) {
            threadSetSelfName(mName.c_str());
        }

        while (mThreadStatus > THREAD_STATUS_STOPPED) {

            if (mWaitPaused) {
                mThreadStatus = THREAD_STATUS_PAUSED;
                mWaitPaused = false;
                mWaitCond.notify_one();
            }

            if (mThreadStatus == THREAD_STATUS_PAUSED) {
                std::unique_lock<std::mutex> mWaitLock{mWaitMutex};
                //???????????????????????????pause????????????????????????????????????stop?????? stop???????????????????????????
                if (mThreadStatus == THREAD_STATUS_PAUSED) {
                    mWaitCond.wait(mWaitLock, [this]() {
                        return mThreadStatus != THREAD_STATUS_PAUSED;
                    });
                }

            } else if (mThreadStatus == THREAD_STATUS_RUNNING) {
                int ret = mUserFunc();
                if (ret < 0) {
                    mTryPause = true;
                }
            }

            if (mTryPause) {
                //????????????stop????????????????????????????????????????????????????????????
                if (mMutex.try_lock()) {
                    if (mThreadStatus == THREAD_STATUS_RUNNING) {
                        mThreadStatus = THREAD_STATUS_PAUSED;
                    }
                    mMutex.unlock();
                }
                mTryPause = false;
            }
        }

        if (mThreadEndCallback != nullptr) {
            mThreadEndCallback();
        }
    }

    void SNThread::pause() {
        SN_LOGD("%s (%s) start", __func__, mName.c_str());
        std::lock_guard<std::mutex> lockGuard{mMutex};
        if (mThreadStatus == THREAD_STATUS_RUNNING) {
            mWaitPaused = true;
            //????????????pause??????
            std::unique_lock<std::mutex> uniqueLock{mWaitMutex};
            mWaitCond.wait(uniqueLock, [this]() {
                return !mWaitPaused;
            });
        }
        SN_LOGD("%s (%s) end", __func__, mName.c_str());
    }

    void SNThread::stop() {
        SN_LOGD("%s (%s) start", __func__, mName.c_str());
        std::lock_guard<std::mutex> lockGuard{mMutex};
        mTryPause = false;
        {
            std::unique_lock<std::mutex> uniqueLock{mWaitMutex};
            mThreadStatus = THREAD_STATUS_STOPPED;
        }
        //????????????????????????pause??????????????????????????????????????????????????????
        mWaitCond.notify_one();
        if (mThread && mThread->joinable()) {
            if (mThread->get_id() != std::this_thread::get_id()) {
                mThread->join();
            } else {
                mThread->detach();
            }
        }
        mThread.reset(nullptr);
        assert(mThread == nullptr);
        SN_LOGD("%s (%s) end", __func__, mName.c_str());
    }

    void SNThread::prePause() {
        if (mMutex.try_lock()) {
            if (THREAD_STATUS_RUNNING == mThreadStatus) {
                mTryPause = true;
            }
            mMutex.unlock();
        }
    }

    void SNThread::setBeginCallback(const std::function<void()> &callback) {
        mThreadBeginCallback = callback;
    }

    void SNThread::setEndCallback(const std::function<void()> &callback) {
        mThreadEndCallback = callback;
    }

    std::thread::id SNThread::getId() {
        if (mThread) {
            return mThread->get_id();
        }
        return {};
    }

    SNThread::~SNThread() {
        std::lock_guard<std::mutex> lockGuard{mMutex};
        mTryPause = false;
        {
            std::unique_lock<std::mutex> uniqueLock{mWaitMutex};
            mThreadStatus = THREAD_STATUS_IDLE;
        }
        //????????????????????????pause??????????????????????????????????????????????????????
        mWaitCond.notify_one();
        if (mThread && mThread->joinable()) {
            if (mThread->get_id() != std::this_thread::get_id()) {
                mThread->join();
            } else {
                mThread->detach();
            }
        }
        mThread.reset(nullptr);
    }


}