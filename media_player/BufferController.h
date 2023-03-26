#ifndef SIVINPLAYER_BUFFERCONTROLLER_H
#define SIVINPLAYER_BUFFERCONTROLLER_H

#include <cstdint>
#include "MediaPacketQueue.h"

namespace Sivin {

  class BufferController {
  public:
    explicit BufferController();
    ~BufferController();

    int64_t getPacketDuration(BufferType type);

  private:
    // MediaPacketQueue mVideoPacketQueue;
    // MediaPacketQueue mAudioPacketQueue;
    // MediaPacketQueue mSubtitlePacketQueue;
  };
}// namespace Sivin

#endif//SIVINPLAYER_BUFFERCONTROLLER_H