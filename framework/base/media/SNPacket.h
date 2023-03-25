//
// Created by sivin on 12/6/22.
//

#ifndef FRAMKEWORK_BASE_MEDIA_SNPACKET_H
#define FRAMKEWORK_BASE_MEDIA_SNPACKET_H

#include <cstdint>


namespace Sivin {

#define SN_PKT_FLAG_KEY 0x0001    // The packet contains a keyframe
#define SN_PKT_FLAG_CORRUPT 0x0002// The packet content is corrupted

  class SNPacket {
  public:
    class PacketInfo {
    public:
      PacketInfo &operator=(const PacketInfo &other);

    public:
      int streamIndex{-1};

      int64_t pts{-1};

      int64_t dts{-1};

      int64_t timePosition{-1};

      int64_t duration{-1};

      int flags{-1};

      int64_t pos{-1};
    };

  public:
    virtual ~SNPacket(){};

    virtual uint8_t *getData() = 0;

    virtual PacketInfo &getInfo() {
      return mInfo;
    };

    virtual int64_t getSize() = 0;

  protected:
    PacketInfo mInfo{};
  };

}// namespace Sivin

#endif// FRAMKEWORK_BASE_MEDIA_SNPACKET_H
