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

    void addPacket(std::unique_ptr<SNPacket> packet, BufferType type);

    std::unique_ptr<SNPacket> getPacket(BufferType type);

    int getPacketCount(BufferType type);

    int64_t getPacketDuration(BufferType type);

    int64_t getLastPacketPts(BufferType type);

    int64_t getFirstPacketPts(BufferType type);

  private:
    MediaPacketQueue &getMeidaQueue(BufferType type);

  private:
    MediaPacketQueue mVideoPacketQueue{BufferType::VIDEO};
    MediaPacketQueue mAudioPacketQueue{BufferType::AUDIO};
    MediaPacketQueue dummy{BufferType::NONE};
  };
}// namespace Sivin

#endif//SIVINPLAYER_BUFFERCONTROLLER_H