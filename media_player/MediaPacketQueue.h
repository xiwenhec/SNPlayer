#ifndef SIVINPLAYER_MEDIAPACKETQUEUE_H
#define SIVINPLAYER_MEDIAPACKETQUEUE_H

#include "base/media/SNPacket.h"
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
namespace Sivin {

  enum class BufferType {
    NONE = 0,
    VIDEO = 1,
    AUDIO = (1 << 1),
    SUBTILE = (1 << 2),
    AV = (VIDEO | AUDIO),
    ALL = (VIDEO | AUDIO | SUBTILE),
  };

  //TODO:需要对packet如果没有duration的额外情况进行处理
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

    int64_t getKeyPacketTimePositionBefore(int64_t pts);

    // int64_t getKeyPacketTimePositionBeforeUTCTime(int64_t pts);

    int64_t getFirstKeyPacketPts(int64_t pts);

    //获取距离当前最近的关键战的timePos
    int64_t getLastKeyPacketTimePos();

    int64_t clearPacketBeforeTimePos(int64_t pts);

    int64_t clearPacketBeforePTS(int64_t pts);

    void rewind();

    int64_t getLastPacketTimePos();

    int64_t getFirstPacketTimePos();

    //获取队列里最后一个packet的pts
    int64_t getLastPacketPts();

    int64_t findSeamlessPointTimePosition(int &count);

    void clearPacketAfterTimePosition(int64_t pts);

    void setMaxBackwardDuration(uint64_t duration);

  public:
    BufferType mBufferType{BufferType::NONE};

  private:
    void popFrontPacket();

  private:
    std::list<MediaPacket> mQueue{};

    std::list<MediaPacket>::iterator mCurrent;

    std::recursive_mutex mMutex{};

    int64_t mDuration{0};

    int64_t mTotalDuration{0};

    uint64_t mMaxBackwardDuration{0};

    uint8_t *mDropedExtraData{nullptr};

    int mDropedExtraDataSize{0};
  };


}// namespace Sivin


#endif// SIVINPLAYER_MEDIAPACKETQUEUE_H