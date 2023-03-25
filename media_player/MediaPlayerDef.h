//
// Created by sivin on 23-1-15.
//

#ifndef SIVINPLAYER_MEDIAPLAYERDEF_H
#define SIVINPLAYER_MEDIAPLAYERDEF_H

#include <deque>
#include <base/media/SNMediaInfo.h>
#include <memory>
#include "base/media/IMediaFrame.h"
namespace Sivin {

  enum class SeekMode {
    ACCURATE = 0x01,
    INACCURATE = 0x10,
  };

  enum class PlayerStatus {
    IDLE = 0,
    INITIALIZED,
    PREPARED_INIT,
    PREPARING,
    PREPARED,
    PLAYING,
    PAUSED,
    STOPPED,
    COMPLETION,
    ERROR = 101,
  };

  using OnRenderFrameCallback = bool(void *userData, IMediaFrame *frame);

  using PlayerCallbackType0 = void (*)(void *userData);

  struct PlayerListener {
    PlayerCallbackType0 onPreparedCallback{nullptr};
    void *userData{nullptr};
  };


  class MediaInfo {
  public:
    int64_t totalBitrate{0};

    std::deque<std::unique_ptr<SNStreamInfo>> mStreamInfoQueue{};
  };

}// namespace Sivin

#endif// SIVINPLAYER_MEDIAPLAYERDEF_H
