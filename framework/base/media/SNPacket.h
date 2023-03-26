//
// Created by sivin on 12/6/22.
//

#ifndef FRAMKEWORK_BASE_MEDIA_SNPACKET_H
#define FRAMKEWORK_BASE_MEDIA_SNPACKET_H

#include "base/media/SNMediaInfo.h"
#include <cstdint>


namespace Sivin {

#define SN_PKT_FLAG_KEY 0x0001    // The packet contains a keyframe
#define SN_PKT_FLAG_CORRUPT 0x0002// The packet content is corrupted

  /**
    解封装数据packet的抽象
  */
  class SNPacket {
  public:
    class PacketInfo {
    public:
      PacketInfo &operator=(const PacketInfo &other);

    public:
      int streamIndex{SN_UNKNOWN_VALUE};

      int64_t pts{SN_UNKNOWN_VALUE};

      int64_t dts{SN_UNKNOWN_VALUE};

      //值等于当前pkt.pts-first.pts
      int64_t timePosition{SN_UNKNOWN_VALUE};

      int64_t duration{0};

      int flags{0};

      int64_t pos{0};
    };

  public:
    virtual ~SNPacket(){};

    virtual uint8_t *getData() = 0;

    virtual PacketInfo &getInfo() {
      return mInfo;
    };

    virtual int64_t getSize() = 0;

    virtual bool isDiscard() {
      return mDiscard;
    }

    virtual void setDiscard(bool discard) {
      mDiscard = discard;
    }

  protected:
    PacketInfo mInfo{};
    bool mDiscard{};
  };

}// namespace Sivin

#endif// FRAMKEWORK_BASE_MEDIA_SNPACKET_H
