#ifndef SIVINPLAYER_MEDIAPACKETQUEUE_H
#define SIVINPLAYER_MEDIAPACKETQUEUE_H

#include "base/media/SNPacket.h"
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
namespace Sivin {

  class MediaPacketQueue {
  public:
    using MediaPacket = std::unique_ptr<SNPacket>;

    explicit MediaPacketQueue();

    ~MediaPacketQueue();

    void setOnePacketDuration(int64_t duration);

    int64_t getOnePacketDuration();


  public:
    void clear();

    void addPacket(MediaPacket packet);

    MediaPacket getPacket();

    int getSize();

    int64_t getDuration();

    int64_t getPts();

    int64_t getKeyTimePositionBefore(int64_t pts);

    int64_t getKeyTimePositionBeforeUTCTime(int64_t pts);

    int64_t getFirstKeyPTS(int64_t pts);

    int64_t getLastKeyTimePos();

    int64_t clearPacketBeforeTimePos(int64_t pts);

    int64_t clearPacketBeforePTS(int64_t pts);

    void rewind();

    int64_t getLastTimePos();

    int64_t getFirstTimePos();

    int64_t getLastPTS();

    int64_t findSeamlessPointTimePosition(int &count);

    void clearPacketAfterTimePosition(int64_t pts);

    void setMaxBackwardDuration(uint64_t duration);

  public:
    int mediaType{0};

  private:
    void popFrontPacket();

  private:
    std::list<MediaPacket> mQueue{};
    std::recursive_mutex mMutex{};
    int64_t mPacketDuration{0};
    int64_t mDuration{0};
    int64_t mTotalDuration{0};
    uint64_t mMaxBackwardDuration{0};

    uint8_t *mDropedExtraData{nullptr};
    int mDropedExtraDataSize{0};
  };


}// namespace Sivin


#endif// SIVINPLAYER_MEDIAPACKETQUEUE_H