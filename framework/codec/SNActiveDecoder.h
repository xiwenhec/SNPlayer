#ifndef SN_FRAMEWORK_SNACTIVEDECODER_H
#define SN_FRAMEWORK_SNACTIVEDECODER_H

#include "base/media/SNFrame.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "codec/IDecoder.h"
#include "utils/SNSpscQueue.h"
#include "utils/SNThread.h"

#include <memory>
#include <queue>
namespace Sivin {
  class SNActiveDecoder : public IDecoder {
  public:
    explicit SNActiveDecoder();

    virtual ~SNActiveDecoder();

    int open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags);
    void close();

  private:
    virtual int initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) = 0;

  private:
    int decodeFunc();

  protected:
    std::unique_ptr<SNThread> mDeocdeThread{nullptr};
    std::atomic_bool mRunning{false};

  private:
    SNSpscQueue<std::unique_ptr<SNPacket>> mInputQueue;
    SNSpscQueue<std::unique_ptr<SNFrame>> mOutputQueue;
    
    std::atomic_bool mDecoderEOS{false};
  };


}// namespace Sivin
#endif