#ifndef SIVINPLAYER_MEDIAPLAYERUTIL_H
#define SIVINPLAYER_MEDIAPLAYERUTIL_H

#include "MediaPlayer.h"
#include <cstdint>
#include <atomic>
namespace Sivin {

  enum class StatisticEvent {
    LOOP,
    GOT_PACKET,
    NETWORK
  };
  /*
    统计当前的数据包的下载速度
  */
  class PlayerStatistic {
  public:
  public:
    explicit PlayerStatistic() = default;
    ~PlayerStatistic() = default;

  public:
    void stat(StatisticEvent event, uint64_t size);

  private:
    //上一次读取的时间戳(us)
    int64_t mLastReadTime{0};
    //当前读取的次数
    int64_t mReadTimes{0};

    //当前的下载速度
    float mCurrentDownloadSpeed{0.0};

    //当前获取的字节数
    std::atomic<uint64_t> mReadGotSize{0};
  };


}// namespace Sivin

#endif// SIVINPLAYER_MEDIAPLAYERUTIL_H