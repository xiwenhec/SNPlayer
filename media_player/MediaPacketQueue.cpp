#include "MediaPacketQueue.h"
#include "PlayerMsgController.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "utils/SNLog.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#define ADD_LOCK std::unique_lock<std::recursive_mutex> lock(mMutex)

namespace Sivin {

  MediaPacketQueue::MediaPacketQueue() {
    mCurrent = mQueue.end();
  }

  MediaPacketQueue::~MediaPacketQueue() {
    clear();
    delete mDropedExtraData;
  }

  void MediaPacketQueue::clear() {
    ADD_LOCK;
    mQueue.clear();
    mDuration = 0;
    mTotalDuration = 0;
  }

  void MediaPacketQueue::addPacket(MediaPacket packet) {
    ADD_LOCK;

    //TODO: 有些packet没有duration时该如何处理
    if (packet->getInfo().duration > 0) {
      if (!packet->isDiscard()) {
        mDuration += packet->getInfo().duration;
        mTotalDuration += packet->getInfo().duration;
      }
    }

    if (mBufferType == BufferType::AUDIO && !mQueue.empty() &&
        packet->getInfo().pts != SN_UNKNOWN_VALUE && packet->getInfo().pts < mQueue.back()->getInfo().pts) {
      SN_LOGE("pts revert.");
    }

    mQueue.push_back(std::move(packet));
  }


  int64_t MediaPacketQueue::getLastKeyPacketTimePos() {
    ADD_LOCK;
    int64_t lastPos = -1;
    for (auto iter = mQueue.begin(); iter != mQueue.end(); ++iter) {
      MediaPacket &packet = *iter;
      if (packet.get() == (*mCurrent).get()) {
        break;
      }
      if (packet && packet->getInfo().flags & SN_PKT_FLAG_KEY) {
        return packet->getInfo().timePosition;
      }
    }
    return -1;
  }


  int64_t MediaPacketQueue::getLastPacketPts() {
    ADD_LOCK;
    if (mQueue.empty()) {
      return -1;
    }

    if (mCurrent == mQueue.end()) {
      return -1;
    }

    return mQueue.back()->getInfo().pts;
  }

  int64_t MediaPacketQueue::getLastPacketTimePos() {
    ADD_LOCK;
    if (mQueue.empty()) {
      return -1;
    }
    if (mCurrent == mQueue.end()) {
      return -1;
    }
    return mQueue.back()->getInfo().timePosition;
  }

  int64_t MediaPacketQueue::getFirstPacketTimePos() {
    ADD_LOCK;
    if (mQueue.empty()) {
      return -1;
    }
    if (mCurrent == mQueue.end()) {
      return -1;
    }
    return mQueue.front()->getInfo().timePosition;
  }


  int64_t MediaPacketQueue::getFirstKeyPacketPts(int64_t pts) {
    ADD_LOCK;
    for (auto &packet: mQueue) {
      if (packet && (packet->getInfo().flags & SN_PKT_FLAG_KEY) && (packet->getInfo().pts <= pts)) {
        return packet->getInfo().pts;
      }
      if (packet.get() == (*mCurrent).get()) {
        break;
      }
    }
    return -1;
  }


  MediaPacketQueue::MediaPacket MediaPacketQueue::getPacket() {
    ADD_LOCK;
    MediaPacket packet;

    return std::move(packet);
  }

  int MediaPacketQueue::getSize() {
    ADD_LOCK;
    int size = 0;
    for (auto item = mCurrent; item != mQueue.end(); ++item) {
      size++;
    }
    return size;
  }

  int64_t MediaPacketQueue::getDuration() {
    ADD_LOCK;
    //TODO:Sivin
    // if ((mBufferType == BufferType::BUFFER_TYPE_VIDEO || mBufferType == BufferType::BUFFER_TYPE_AUDIO) && )
    return mDuration;
  }

  int64_t MediaPacketQueue::getPts() {
    ADD_LOCK;
    if (mQueue.empty()) {
      return -1;
    }
    if (mCurrent == mQueue.end()) {
      return -1;
    }
    return mQueue.front()->getInfo().pts;
  }

  //TODO:这里里传递的是pts还是什么，如果是pts，这可能有问题，因为它对比的是timePosition
  int64_t MediaPacketQueue::getKeyPacketTimePositionBefore(int64_t pts) {
    ADD_LOCK;
    for (auto iter = mQueue.begin(); iter != mQueue.end(); ++iter) {
      MediaPacket &packet = *iter;
      if (packet && (packet->getInfo().flags & SN_PKT_FLAG_KEY) && packet->getInfo().timePosition < pts) {
        return packet->getInfo().timePosition;
      }
      if (packet.get() == (*mCurrent).get()) {
        break;
      }
    }
    return -1;
  }

  // int64_t MediaPacketQueue::getKeyPacketTimePositionBeforeUTCTime(int64_t pts) {
  // }

  int64_t MediaPacketQueue::clearPacketBeforeTimePos(int64_t pos) {
    ADD_LOCK;
    int dropCount = 0;

    while (mCurrent != mQueue.end()) {
    }
    return dropCount;
  }

  void MediaPacketQueue::popFrontPacket() {
    ADD_LOCK;
    if (mQueue.empty()) {
      return;
    }

    if (mCurrent == mQueue.end()) {
      return;
    }

    if ((*mCurrent) && (*mCurrent)->getInfo().duration > 0 && !(*mCurrent)->isDiscard()) {
      mDuration -= (*mCurrent)->getInfo().duration;
    }
    
  }


}// namespace Sivin