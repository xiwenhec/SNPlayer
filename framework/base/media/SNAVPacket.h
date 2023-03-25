//
// Created by sivin on 12/25/22.
//

#ifndef FRAMKEWORK_BASE_MEDIA_SNAVPACKET_H
#define FRAMKEWORK_BASE_MEDIA_SNAVPACKET_H

extern "C" {
#include <libavcodec/packet.h>
};

#include "base/media/SNPacket.h"
#include <string>


namespace Sivin {

//SNPacket的FFmpeg实现
class SNAVPacket : public SNPacket {

public:
  explicit SNAVPacket(AVPacket **pkt);

  ~SNAVPacket();

  uint8_t *getData() override;

  int64_t getSize() override;

private:
  AVPacket *mpkt{nullptr};

  bool mIsProtected;
  
  std::string mMagicKey{};

  void copyInfo();
};

} // namespace Sivin

#endif // FRAMKEWORK_BASE_MEDIA_SNAVPACKET_H
