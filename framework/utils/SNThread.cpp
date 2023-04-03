#include "SNThread.h"
#include "utils/SNLog.h"
#include <cassert>
#include <cstdio>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#ifdef ANDROID

#include <cassert>
#include <cstring>
#include <sys/prctl.h>
#include <utils/NSLog.h>

#define MAX_TASK_NAME_LEN (16)
#endif

namespace Sivin {

#define ADD_LOCK \
  std::unique_lock<std::mutex> lock { mMutex }

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

  SNThread::SNThread(std::function<int(void)> func, std::string name)
      : mUserFunc(func), mName(std::move(name)), mInnerThread(nullptr) {}


  SNThread::~SNThread() {
    ADD_LOCK;
    mTryPause = false;
    {
      std::unique_lock<std::mutex> waitLock{mWaitMutex};
      mStatus = Status::IDLE;
    }
    //如果当前线程处于pause状态，则唤醒线程，使其自然运行结束。
    mWaitCond.notify_one();

    if (mInnerThread && mInnerThread->joinable()) {
      if (mInnerThread->get_id() != std::this_thread::get_id()) {
        mInnerThread->join();
      } else {
        mInnerThread->detach();
      }
    }
    mInnerThread.reset();
    SN_LOGD("%s (%s) end", __func__, mName.c_str());
  }


  void SNThread::start() {
    ADD_LOCK;
    if (mInnerThread == nullptr) {
      mStatus = Status::RUNNING;
      mInnerThread = std::make_unique<std::thread>(&SNThread::runLoop, this);
    } else {
      mStatus = Status::RUNNING;
      mWaitCond.notify_one();
    }
  }

  void SNThread::pause() {
    ADD_LOCK;
    if (mStatus == Status::RUNNING) {
      //这里的锁时为了互斥mWaitPaused变量
      std::unique_lock<std::mutex> waitLock{mWaitMutex};
      mWaitPaused = true;
      mWaitCond.wait(waitLock, [=] {
        return !mWaitPaused;
      });
    }
  }


  void SNThread::tryPause() {
    if (mMutex.try_lock()) {
      if (mStatus == Status::RUNNING) {
        mTryPause = true;
      }
      mMutex.unlock();
    }
  }

  void SNThread::stop() {
    ADD_LOCK;

    SN_LOGD("%s (%s) start", __func__, mName.c_str());
    mTryPause = false;
    {
      std::unique_lock<std::mutex> waitLock{mWaitMutex};
      mStatus = Status::STOPED;
    }
    //如果当前线程处于pause状态，则唤醒线程，使其自然运行结束。
    mWaitCond.notify_one();

    if (mInnerThread && mInnerThread->joinable()) {
      if (mInnerThread->get_id() != std::this_thread::get_id()) {
        mInnerThread->join();
      } else {
        mInnerThread->detach();
      }
    }
    mInnerThread.reset();
    assert(mInnerThread == nullptr);
    SN_LOGD("%s (%s) end", __func__, mName.c_str());
  }


  void SNThread::runLoop() {
    if (mThreadBeginCallback != nullptr) {
      mThreadBeginCallback();
    }
    assert(mUserFunc != nullptr);

    if (mName.length() > 0) {
      threadSetSelfName(mName.c_str());
    }

    while (mStatus > Status::STOPED) {

      if (mWaitPaused) {
        mStatus = Status::PAUSED;
        std::unique_lock<std::mutex> waitLock{mWaitMutex};
        mWaitPaused = false;
        //通知调用pause，暂停成功
        mWaitCond.notify_one();
      }

      if (mStatus == Status::PAUSED) {
        std::unique_lock<std::mutex> mWaitLock{mWaitMutex};
        //二次判断，防止进入pause判断流程后其他线程调用stop导致stop无效，线程无法释放
        if (mStatus == Status::PAUSED) {
          mWaitCond.wait(mWaitLock, [=] {
            return mStatus != Status::PAUSED;
          });
        }
      } else if (mStatus == Status::RUNNING) {
        int ret = mUserFunc();
        if (ret < 0) {
          mTryPause = true;
        }
      }

      if (mTryPause) {
        if (mMutex.try_lock()) {
          if (mStatus == Status::RUNNING) {
            mStatus = Status::PAUSED;
          }
          mMutex.unlock();
        }
        mTryPause = false;
      }
    }
    //线程运行结束
    if (mThreadEndCallback != nullptr) {
      mThreadEndCallback();
    }
  }

  std::thread::id SNThread::getId() {
    if (mInnerThread) {
      return mInnerThread->get_id();
    }
    return {};
  }

  void SNThread::setBeginCallback(const std::function<void()> &callback) {
    mThreadBeginCallback = callback;
  }

  void SNThread::setEndCallback(const std::function<void()> &callback) {
    mThreadEndCallback = callback;
  }

}// namespace Sivin