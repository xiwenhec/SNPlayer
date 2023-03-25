#ifndef SIVINPLAYER_BUFFERCONTROLLER_H
#define SIVINPLAYER_BUFFERCONTROLLER_H

#include <cstdint>

namespace Sivin {

  enum class BufferType {
    BUFFER_TYPE_VIDEO = 1,
    BUFFER_TYPE_AUDIO = (1 << 1),
    BUFFER_TYPE_SUBTILE = (1 << 2),
    BUFFER_TYPE_AV = (BUFFER_TYPE_VIDEO | BUFFER_TYPE_AUDIO),
    BUFFER_TYPE_ALL = (BUFFER_TYPE_VIDEO | BUFFER_TYPE_AUDIO | BUFFER_TYPE_SUBTILE)
  };

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