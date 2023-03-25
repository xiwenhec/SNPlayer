#include "PlayerNotifier.h"
#include "MediaPlayerDef.h"
#include "utils/SNThread.h"
#include <memory>
#include <mutex>
#include <utility>

#define LOG_TAG "PlayerNotifier"


namespace Sivin {
  class PlayerEvent {
  public:
    explicit PlayerEvent(PlayerCallbackType0 func)
        : data(nullptr), argsNum(0) {
      mFunc.f0 = func;
    }

  public:
    void onEvent(void *userData) {
      switch (argsNum) {
        case 0:
          mFunc.f0(userData);
          break;
      }
    }

  private:
    union {
      PlayerCallbackType0 f0;
    } mFunc;

    //func参数的个数，userData不计算在内
    int argsNum{0};

    void *data{nullptr};
  };


  PlayerNotifier::PlayerNotifier() {
    mNotifyThread = MAKE_UNIQUE_THREAD(postLoop, LOG_TAG);
  }
  PlayerNotifier::~PlayerNotifier(){};

  void PlayerNotifier::setEnable(bool enable) {
    mEnable = enable;
  }

  void PlayerNotifier::setListener(const PlayerListener &listener) {
    mListener = listener;
  }

  void PlayerNotifier::notifyPrepared() {
    if (!mEnable || mListener.onPreparedCallback == nullptr) {
      return;
    }
    notifyEventType0(mListener.onPreparedCallback);
  }

  void PlayerNotifier::notifyEventType0(PlayerCallbackType0 callback) {
    if (!mEnable) {
      return;
    }
    pushEvent(std::unique_ptr<PlayerEvent>(new PlayerEvent(callback)));
  }

  void PlayerNotifier::pushEvent(std::unique_ptr<PlayerEvent> event) {
    std::unique_lock<std::mutex> lock{mMutex};
    mEventQueue.push_back(std::move(event));
    mCond.notify_one();
  }

  int PlayerNotifier::postLoop() {
    if (!mRunning) {
      return -1;
    }

    std::unique_ptr<PlayerEvent> event = nullptr;
    {
      std::unique_lock<std::mutex> lock{mMutex};
      if (mEventQueue.empty()) {
        mCond.wait(lock, [=] { return !mRunning || !mEventQueue.empty(); });
        if (mEventQueue.empty()) {
          return 0;
        }
      }
      event = std::move(mEventQueue.front());
      mEventQueue.pop_front();
    }

    if (event) {
      event->onEvent(mListener.userData);
    }
    return 0;
  }

}// namespace Sivin