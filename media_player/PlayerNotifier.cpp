#include "PlayerNotifier.h"
#include "MediaPlayerDef.h"
#include "utils/SNThread.h"
#include <memory>
#include <mutex>
#include <utility>

#define LOG_TAG "PlayerNotifier"


namespace Sivin {

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
    pushEvent(std::unique_ptr<NotifyEvent>(new NotifyEvent(callback)));
  }

  void PlayerNotifier::pushEvent(std::unique_ptr<NotifyEvent> event) {
    std::unique_lock<std::mutex> lock{mMutex};
    mEventQueue.push_back(std::move(event));
    mCond.notify_one();
  }

  int PlayerNotifier::postLoop() {
    if (!mRunning) {
      return -1;
    }

    std::unique_ptr<NotifyEvent> event = nullptr;
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

  void PlayerNotifier::notifyEvent(PlayerEventCode code, const char *desc) {
  }

}// namespace Sivin