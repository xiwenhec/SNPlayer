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

    explicit MediaPacketQueue(BufferType type);

    ~MediaPacketQueue();

    void setOnePacketDuration(int64_t duration);

    int64_t getOnePacketDuration();


  public:
    void clear();

    void addPacket(MediaPacket packet);

    MediaPacket getPacket();

    int getSize();

    int64_t getDuration();

    //获取队列里第一个包含关键帧的pkt的timePostion，且不超过指定的timePosition
    int64_t getKeyPacketTimePositionBefore(int64_t timePosition);

    //获取队列里第一个包含关键帧的pkt的pts，且不超过指定的pts
    int64_t getFirstKeyPacketPtsBefore(int64_t pts);

    //获取缓冲队列里第一个关键帧关键帧的packet的timeposition
    int64_t getFristKeyPacketTimePosition();

    //获取队列缓存中第一个pacekt的pts
    int64_t getFirstPacketPts();

    //获取队列里最后一个packet的pts
    int64_t getLastPacketPts();

    int64_t getLastPacketTimePos();

    int64_t getFirstPacketTimePos();

    //清除掉（cur---end）间所有小于该timeposition的packet
    //返回清除掉的pkt个数
    int64_t clearPacketBeforeTimePos(int64_t pos);

    int64_t clearPacketBeforePts(int64_t pts);

    void clearPacketAfterTimePosition(int64_t timePosition);

    int64_t findSeamlessPointTimePosition(int &count);

    void rewind();

    void setMaxBackwardDuration(uint64_t backwardDuration);

    BufferType type() const {
      return mBufferType;
    }

  private:
    void popFrontPacket();

  private:
    BufferType mBufferType{BufferType::NONE};

    std::list<MediaPacket> mQueue{};

    /*始终指向下一个要被取出的packet，如果不需要backward,并且队列不为空，
     *则值应该始终指向mQueue.begin()
     */
    std::list<MediaPacket>::iterator mCurrent;

    std::recursive_mutex mMutex{};

    int64_t mPacketDuration{0};

    //播放缓冲区是否设置回退时长
    uint64_t mMaxBackwardDuration{0};

    //当前队列中没有解码播放的数据包时长
    int64_t mDuration{0};

    /*整个缓冲队列数据的总时长，如果不需要回退缓冲，则和mDuration相等
    否则 mTotalDuration = mDuration + 队列中已经播放过的数据时长*/
    int64_t mTotalDuration{0};


    uint8_t *mDropedExtraData{nullptr};
    int mDropedExtraDataSize{0};
  };


}// namespace Sivin


#endif// SIVINPLAYER_MEDIAPACKETQUEUE_H