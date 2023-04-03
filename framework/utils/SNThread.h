#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>
namespace Sivin {

#define NEW_SN_THREAD(func, name) (new SNThread([this]() -> int { return this->func(); }, name))

#define MAKE_UNIQUE_THREAD(func, name) (      \
    std::make_unique<SNThread>([=]() -> int { \
      return this->func();                    \
    },                                        \
        name))

  class SNThread {

  public:
    enum class Status {
      IDLE = 0,
      STOPED,
      RUNNING,
      PAUSED,
    };

  public:
    explicit SNThread(std::function<int(void)> func, std::string name);

    ~SNThread();

    void start();

    void tryPause();

    void pause();

    void stop();

  public:
    std::thread::id getId();

    Status getStatus() {
      return mStatus;
    }

    void setBeginCallback(const std::function<void()> &callback);

    void setEndCallback(const std::function<void()> &callback);


  private:
    void runLoop();

  private:
    std::string mName;
    std::function<int(void)> mUserFunc;
    Status mStatus{Status::IDLE};
    std::unique_ptr<std::thread> mInnerThread{nullptr};

    std::mutex mMutex;

    //内部暂停唤醒使用
    std::mutex mWaitMutex;
    std::condition_variable mWaitCond;

    std::atomic_bool mWaitPaused{false};

    bool mTryPause{false};

    std::function<void()> mThreadBeginCallback;
    std::function<void()> mThreadEndCallback;
  };

}// namespace Sivin