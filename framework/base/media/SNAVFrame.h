#ifndef SN_FRAMEWORK_SNAVFRAME_H
#define SN_FRAMEWORK_SNAVFRAME_H

#include "base/media/SNFrame.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/encryption_info.h>
}

namespace Sivin {

  class SNAVFrame : public SNFrame {
  public:
    explicit SNAVFrame(AVFrame *frame, FrameType type = FrameType::UnKnown);

    virtual std::unique_ptr<SNFrame> clone();

    virtual uint8_t **getData();

    virtual int *getLineSize();

    virtual FrameType getType();
  };

}// namespace Sivin

#endif//