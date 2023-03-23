//
// Created by sivin on 12/25/22.
//

#include "SNAVPacket.h"

namespace Sivin {
  SNAVPacket::SNAVPacket(AVPacket **pkt) {
    mpkt = *pkt;
    *pkt = nullptr;
    copyInfo();
  }

  SNAVPacket::~SNAVPacket() {
    if (mpkt) {
      av_packet_free(&mpkt);
      mpkt = nullptr;
    }
  }

  uint8_t *SNAVPacket::getData() {
    return mpkt->data;
  }

  void SNAVPacket::copyInfo() {
    mInfo.duration = mpkt->duration;
    mInfo.pts = mpkt->pts;
    mInfo.dts = mpkt->dts;
    mInfo.flags = 0;
    if (mpkt->flags & AV_PKT_FLAG_KEY) {
      mInfo.flags |= SN_PKT_FLAG_KEY;
    }
    if (mpkt->flags & AV_PKT_FLAG_CORRUPT) {
      mInfo.flags |= SN_PKT_FLAG_CORRUPT;
    }
    if (mpkt->flags & AV_PKT_FLAG_DISCARD) {
      //            setDiscard(true);
    }

    mInfo.streamIndex = mpkt->stream_index;
    mInfo.timePosition = INT64_MIN;
    mInfo.pos = mpkt->pos;
  }

  int64_t SNAVPacket::getSize() {
    return mpkt ? mpkt->size : 0;
  }


}// namespace Sivin