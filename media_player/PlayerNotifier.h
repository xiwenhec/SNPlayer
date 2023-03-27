#ifndef SIVINPLAYER_PLAYERNOTIFIER_H
#define SIVINPLAYER_PLAYERNOTIFIER_H

#include "MediaPlayerDef.h"
#include "utils/SNThread.h"
#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace Sivin {

  enum class PlayerEventCode {
    LOW_MEMORY = 1,
  };

  class NotifyEvent {
  public:
    explicit NotifyEvent(PlayerCallbackType0 func)
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

  class PlayerNotifier {
  public:
    explicit PlayerNotifier();
    ~PlayerNotifier();

  public:
    void setEnable(bool enable);

    void setListener(const PlayerListener &listener);

    void notifyPrepared();

    void notifyEvent(PlayerEventCode code, const char *desc);

  private:
    void notifyEventType0(PlayerCallbackType0 callback);
    void pushEvent(std::unique_ptr<NotifyEvent> event);
    int postLoop();

  private:
    std::atomic_bool mRunning{true};
    bool mEnable{true};
    PlayerListener mListener{};
    std::mutex mMutex;
    std::condition_variable mCond;
    std::list<std::unique_ptr<NotifyEvent>> mEventQueue{};
    std::unique_ptr<SNThread> mNotifyThread{nullptr};
  };


};// namespace Sivin

#endif//SIVINPLAYER_PLAYERNOTIFIER_H