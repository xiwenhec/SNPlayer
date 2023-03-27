#include "BufferController.h"
#include "MediaPacketQueue.h"
#include "utils/SNLog.h"
#include <cstdint>
#include <utility>

namespace Sivin {


  int64_t BufferController::getPacketDuration(BufferType type) {

    return 0;
  }


  void BufferController::addPacket(std::unique_ptr<SNPacket> packet, BufferType type) {
    if (packet == nullptr) {
      return;
    }
    switch (type) {
      case BufferType::AUDIO:
        return mAudioPacketQueue.addPacket(std::move(packet));
      case BufferType::VIDEO:
        return mVideoPacketQueue.addPacket(std::move(packet));
      default:
        SN_LOGE("don't support type:%d", type);
        return;
    }
  }

}// namespace Sivin