#include "MediaPacketQueue.h"
#include "PlayerMsgController.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "utils/SNLog.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <cassert>

#define ADD_LOCK std::unique_lock<std::recursive_mutex> lock(mMutex)

namespace Sivin {

  MediaPacketQueue::MediaPacketQueue(BufferType type) {
    mBufferType = type;
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

  /*discard的数据包不会被加入时长计算*/
  void MediaPacketQueue::addPacket(MediaPacket packet) {
    ADD_LOCK;

    bool empty = mQueue.empty();

    if (packet->getInfo().duration > 0) {

      if (mPacketDuration == 0) {
        mPacketDuration = packet->getInfo().duration;
      }
      if (!packet->isDiscard()) {
        mDuration += packet->getInfo().duration;
        mTotalDuration += packet->getInfo().duration;
      }
    } else if (mPacketDuration > 0) {
      packet->getInfo().duration = mPacketDuration;
      if (!packet->isDiscard()) {
        mDuration += mPacketDuration;
        mTotalDuration += mPacketDuration;
      }
    }

    if (mBufferType == BufferType::AUDIO &&
        !mQueue.empty() && packet->getInfo().pts != SN_UNKNOWN_VALUE &&
        packet->getInfo().pts < mQueue.back()->getInfo().pts) {
      SN_LOGE("pts revert.");
    }

    mQueue.push_back(std::move(packet));

    //调整current指针
    if (empty) {
      mCurrent = mQueue.begin();
    }

    if (mCurrent == mQueue.end()) {
      --mCurrent;
    }
  }


  std::unique_ptr<SNPacket> MediaPacketQueue::getPacket() {
    ADD_LOCK;

    //检测判断如果不需要回退缓冲，则后面的条件应该成立，否则会出现异常
    assert(mMaxBackwardDuration != 0 || (mTotalDuration == mDuration && mCurrent == mQueue.begin()));

    if (mQueue.empty()) {
      return nullptr;
    }
    if (mCurrent == mQueue.end()) {
      return nullptr;
    }

    std::unique_ptr<SNPacket> packet{};

    //如果没有设置回退时长，则取出数据包后，就直接从队列里移除数据
    if (mMaxBackwardDuration == 0) {
      packet = std::move(mQueue.front());
      mQueue.pop_front();
      mCurrent = mQueue.begin();
      if (packet && packet->getInfo().duration > 0 && !packet->isDiscard()) {
        mTotalDuration -= packet->getInfo().duration;
      }
    } else {
      //如果需要回退，则将当前数据进行拷贝，更新当前指针
      packet = (*mCurrent)->clone();
      ++mCurrent;
    }

    if (packet && packet->getInfo().duration > 0 && !packet->isDiscard()) {
      mDuration -= packet->getInfo().duration;
    }

    if (mMaxBackwardDuration > 0) {
      while (mTotalDuration - mDuration > mMaxBackwardDuration) {
        assert(!mQueue.empty());
        bool isBegin = mCurrent == mQueue.begin();
        if (mQueue.front()->getInfo().duration > 0 && !mQueue.front()->isDiscard()) {
          mTotalDuration -= mQueue.front()->getInfo().duration;
        }
        mQueue.pop_front();
        if (isBegin) {
          mCurrent = mQueue.begin();
        }
      }
    }
    return packet;
  }

  //TODO:每次都要重新遍历，这里是否可以优化
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
    if ((mBufferType == BufferType::AUDIO || mBufferType == BufferType::VIDEO) && mPacketDuration == 0) {
      if (mQueue.empty()) {
        return 0;
      }
      if (mCurrent == mQueue.end()) {
        return 0;
      }
      return -1;
    }
    return mDuration;
  }


  int64_t MediaPacketQueue::getKeyPacketTimePositionBefore(int64_t timePosition) {
    ADD_LOCK;
    for (auto iter = mQueue.begin(); iter != mQueue.end(); ++iter) {
      auto &packet = *iter;
      if (packet && (packet->getInfo().flags & SN_PKT_FLAG_KEY) && packet->getInfo().timePosition < timePosition) {
        return packet->getInfo().timePosition;
      }
      if (packet == *mCurrent) {
        break;
      }
    }
    return -1;
  }

  int64_t MediaPacketQueue::getFirstKeyPacketPtsBefore(int64_t pts) {
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


  int64_t MediaPacketQueue::getFristKeyPacketTimePosition() {
    ADD_LOCK;
    for (auto iter = mQueue.rbegin(); iter != mQueue.rend(); ++iter) {
      auto &packet = *iter;
      if (packet && packet->getInfo().flags & SN_PKT_FLAG_KEY) {
        return packet->getInfo().timePosition;
      }
      if (packet == *mCurrent) {
        break;
      }
    }
    return -1;
  }

  int64_t MediaPacketQueue::getFirstPacketPts() {
    ADD_LOCK;
    if (mQueue.empty()) {
      return -1;
    }
    if (mCurrent == mQueue.end()) {
      return -1;
    }
    return mQueue.front()->getInfo().pts;
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


  int64_t MediaPacketQueue::clearPacketBeforeTimePos(int64_t pos) {
    ADD_LOCK;
    int dropCount = 0;
    while (mCurrent != mQueue.end()) {
      auto &packet = *mCurrent;
      if (packet && packet->getInfo().timePosition < pos) {
        popFrontPacket();
        dropCount++;
      } else {
        break;
      }
    }
    return dropCount;
  }

  int64_t MediaPacketQueue::clearPacketBeforePts(int64_t pts) {
    ADD_LOCK;
    int dropCount = 0;
    while (mCurrent != mQueue.end()) {
      auto &packet = *mCurrent;
      if (packet && packet->getInfo().pts < pts) {
        popFrontPacket();
        dropCount++;
      } else {
        break;
      }
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

    if (mMaxBackwardDuration == 0) {
      if (!mQueue.front()->isDiscard()) {
        mTotalDuration -= mQueue.front()->getInfo().duration;
      }
      mQueue.pop_front();
      mCurrent = mQueue.begin();
    } else {
      ++mCurrent;
    }
    assert(mMaxBackwardDuration != 0 || (mTotalDuration == mDuration && mCurrent == mQueue.begin()));
  }

  void MediaPacketQueue::clearPacketAfterTimePosition(int64_t timePosition) {
    ADD_LOCK;
    bool found = false;
    while (mQueue.back() != *mCurrent && !found) {
      auto &packet = mQueue.back();

      if (packet == nullptr) {
        mQueue.pop_back();
        continue;
      }

      if (packet->getInfo().timePosition <= timePosition) {
        found = true;
      }

      if (packet->getInfo().duration > 0 && !packet->isDiscard()) {
        mDuration -= packet->getInfo().duration;
        mTotalDuration -= packet->getInfo().duration;
      }
      mQueue.pop_back();
    }// while
  }

  int64_t MediaPacketQueue::findSeamlessPointTimePosition(int &count) {
    return 0;
  }


  void MediaPacketQueue::rewind() {
    mCurrent = mQueue.begin();
    mDuration = mTotalDuration;
  }


  void MediaPacketQueue::setMaxBackwardDuration(uint64_t backwardDuration) {
    mMaxBackwardDuration = backwardDuration;
  }

}// namespace Sivin