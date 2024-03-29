//
// Created by sivin on 12/25/22.
//

#ifndef SN_FRAMKEWORK_SNAVPACKET_H
#define SN_FRAMKEWORK_SNAVPACKET_H

extern "C" {
#include <libavcodec/packet.h>
}

#include <base/media/SNPacket.h>
#include <string>


namespace Sivin {

  //SNPacket的FFmpeg实现
  class SNAVPacket : public SNPacket {

  public:
    explicit SNAVPacket(AVPacket **pkt);

    SNAVPacket(const SNAVPacket &packet);

    ~SNAVPacket();

    virtual uint8_t *getData() override;

    virtual int64_t getSize() override;

    virtual std::unique_ptr<SNPacket> clone() override;

    AVPacket *toFFPacket();

  private:
    AVPacket *mpkt{nullptr};

    bool mProtected;

    std::string mMagicKey{};

    void copyInfo();
  };

}// namespace Sivin

#endif// SN_FRAMKEWORK_SNAVPACKET_H
