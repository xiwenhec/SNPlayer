//
// Created by Sivin on 2022-11-26.
//

#ifndef SN_FRAMEWORK_SNTHREAD_H
#define SN_FRAMEWORK_SNTHREAD_H

#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>

#define NEW_SN_THREAD(func, name) (new SNThread([this]() -> int { return this->func(); }, name))
#define MAKE_UNIQUE_THREAD(func, name) ( std::make_unique<SNThread>([=]() -> int { \
return this->func();                                                                  \
}, name))

namespace Sivin {

  class SNThread {
  public:
    enum SNThreadStatus {
      THREAD_STATUS_IDLE = 0,
      THREAD_STATUS_STOPPED,
      THREAD_STATUS_RUNNING,
      THREAD_STATUS_PAUSED,
    };

    using ThreadBeginCallback = std::function<int()>;


  public:
    explicit SNThread(std::function<int()> userFunc, const char *threadName = "");

    ~SNThread();

    void start();

    void pause();

    void prePause();

    void stop();

    SNThreadStatus getStatus() {
      return mThreadStatus;
    }

    void setBeginCallback(const std::function<void()> &callback);

    void setEndCallback(const std::function<void()> &callback);

    std::thread::id getId();


  private:
    void threadRun();

  private:
    std::string mName;
    std::function<int()> mUserFunc;
    //外部多线程同步
    std::mutex mMutex;
    //内部暂停唤醒使用
    std::mutex mWaitMutex;
    std::condition_variable mWaitCond;
    std::unique_ptr<std::thread> mThread;
    SNThreadStatus mThreadStatus{THREAD_STATUS_IDLE};

    std::function<void()> mThreadBeginCallback;
    std::function<void()> mThreadEndCallback;

    std::atomic_bool mTryPause{false};
    //内部使用，需要线程暂停
    std::atomic_bool mWaitPaused{false};
  };
}


#endif //SN_FRAMEWORK_SNTHREAD_H
