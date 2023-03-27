#ifndef SIVINPLAYER_BUFFERCONTROLLER_H
#define SIVINPLAYER_BUFFERCONTROLLER_H

#include <cstdint>
#include <memory>
#include "MediaPacketQueue.h"
#include "base/media/SNPacket.h"

namespace Sivin {

  class BufferController {
  public:
    explicit BufferController();
    
    ~BufferController();

    int64_t getPacketDuration(BufferType type);

    void addPacket(std::unique_ptr<SNPacket> packet, BufferType type);

  private:
    MediaPacketQueue mVideoPacketQueue;
    MediaPacketQueue mAudioPacketQueue;
  };
}// namespace Sivin

#endif//SIVINPLAYER_BUFFERCONTROLLER_H