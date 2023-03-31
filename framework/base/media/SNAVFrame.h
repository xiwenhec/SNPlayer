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
    explicit SNAVFrame(AVFrame *frame);
    ~SNAVFrame();

  public:
    virtual uint8_t **getData() override;

    virtual int *getLineSize() override;

    virtual FrameType getType() override;

    virtual std::unique_ptr<SNFrame> clone() override;

  private:
    void copyInfo();

  private:
    AVFrame *mAVFrame{nullptr};
  };

}// namespace Sivin

#endif//