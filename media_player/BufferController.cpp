#include <cassert>
#define LOG_TAG "BufferController"

#include "BufferController.h"
#include "MediaPacketQueue.h"
#include "base/media/SNPacket.h"
#include "utils/SNLog.h"
#include <cstdint>
#include <memory>
#include <utility>


namespace Sivin {


  MediaPacketQueue &BufferController::getMeidaQueue(BufferType type) {
    switch (type) {
      case BufferType::AUDIO:
        return mAudioPacketQueue;
      case BufferType::VIDEO:
        return mVideoPacketQueue;
      default:
        return dummy;
    }
  }

  void BufferController::addPacket(std::unique_ptr<SNPacket> packet, BufferType type) {
    if (packet == nullptr) {
      return;
    }
    auto &queue = getMeidaQueue(type);
    if (queue.type() == BufferType::NONE) {
      SN_LOGE("don't support type:%d", type);
      return;
    }
    queue.addPacket(std::move(packet));
  }

  std::unique_ptr<SNPacket> BufferController::getPacket(BufferType type) {
    auto &queue = getMeidaQueue(type);
    if (queue.type() == BufferType::NONE) {
      SN_LOGE("don't support type:%d", type);
      return nullptr;
    }
    return queue.getPacket();
  }

  int BufferController::getPacketCount(BufferType type) {
    auto &queue = getMeidaQueue(type);
    if (queue.type() == BufferType::NONE) {
      SN_LOGE("don't support type:%d", type);
      return 0;
    }
    return queue.getSize();
  }

  int64_t BufferController::getPacketDuration(BufferType type) {
    auto &queue = getMeidaQueue(type);
    if (queue.type() == BufferType::NONE) {
      SN_LOGE("don't support type:%d", type);
      return 0;
    }
    return queue.getDuration();
  }

  int64_t BufferController::getLastPacketPts(BufferType type) {
    auto &queue = getMeidaQueue(type);
    if (queue.type() == BufferType::NONE) {
      SN_LOGE("don't support type:%d", type);
      return 0;
    }
    return queue.getLastPacketPts();
  }

  int64_t BufferController::getFirstPacketPts(BufferType type) {
    auto &queue = getMeidaQueue(type);
    if (queue.type() == BufferType::NONE) {
      SN_LOGE("don't support type:%d", type);
      return 0;
    }
    return queue.getFirstPacketPts();
  }

}// namespace Sivin