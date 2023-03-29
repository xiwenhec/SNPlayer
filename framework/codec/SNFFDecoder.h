#ifndef SN_FRAMEWORK_SNFFDECODER_H
#define SN_FRAMEWORK_SNFFDECODER_H
#include "base/media/SNMediaInfo.h"
#include "codec/SNActiveDecoder.h"
#include <cstdint>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace Sivin {

  class SNFFDecoder : public SNActiveDecoder {
  private:
    struct InternalDecoder {
      AVCodecContext *codecContext{nullptr};
      const AVCodec *codec{nullptr};
      AVFrame *avFrame{nullptr};
      struct {
        int pix_fmt{AV_PIX_FMT_NONE};
        int width{0};
        int height{0};
      } videoInfo;
    };

  public:
    explicit SNFFDecoder();
    virtual ~SNFFDecoder();

  private:
    int initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags);

  private:
    std::unique_ptr<InternalDecoder> mDecoder{nullptr};
  };


}// namespace Sivin


#endif//SN_FRAMEWORK_SNFFDECODER_H