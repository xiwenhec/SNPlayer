#ifndef SN_FRAMEWORK_SNFFDECODER_H
#define SN_FRAMEWORK_SNFFDECODER_H

#include "base/media/SNMediaInfo.h"
#include "codec/SNActiveDecoder.h"
#include <cstdint>
#include <memory>

struct AVCodecContext;
struct AVCodec;
struct AVFrame;

namespace Sivin {

  class SNFFDecoder : public SNActiveDecoder {
  private:
    struct InternalDecoder {
      AVCodecContext *codecCtx{nullptr};
      const AVCodec *codec{nullptr};
      AVFrame *avFrame{nullptr};
      struct {
        int pix_fmt{-1};
        int width{0};
        int height{0};
      } videoInfo;
    };

  public:
    explicit SNFFDecoder();
    virtual ~SNFFDecoder();

  private:
    virtual SNRet initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) override;

    virtual SNRet enqueueDecoder(std::unique_ptr<SNPacket> &pPacket) override;

    virtual SNRet dequeueDecoder(std::unique_ptr<SNFrame> &frame) override;

    virtual void flushDecoder() override;

    virtual void closeDecoder() override;


  private:
    std::unique_ptr<InternalDecoder> mDecoder{nullptr};
  };


}// namespace Sivin


#endif//SN_FRAMEWORK_SNFFDECODER_H