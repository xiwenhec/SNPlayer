#ifndef SN_FRAMEWORK_SNACTIVEDECODER_H
#define SN_FRAMEWORK_SNACTIVEDECODER_H

#include "base/media/SNFrame.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "codec/IDecoder.h"
#include "utils/SNThread.h"
#include "utils/SNConcurQueue.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
namespace Sivin {
  class SNActiveDecoder : public IDecoder {
  public:
    explicit SNActiveDecoder();

    virtual ~SNActiveDecoder();

    DecodeRet open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags);
    void close();
    int sendPacket(std::unique_ptr<SNPacket> &packet, uint64_t timeout);

  private:
    bool needDrop(std::unique_ptr<SNPacket> &packet);

  private:
    virtual DecodeRet initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) = 0;
    virtual DecodeRet enqueueDecoder(std::unique_ptr<SNPacket> &pPacket) = 0;
    virtual DecodeRet dequeueDecoder(std::unique_ptr<SNFrame> &frame) = 0;

  private:
    int decodeFunc();
    int extractDecoder();

  protected:
    std::unique_ptr<SNThread> mDeocdeThread{nullptr};
    std::atomic_bool mRunning{false};

  private:
    int mMaxOutQueueSize{10};
    int mMaxInQueueSize{16};
    SNConcurQueue<std::unique_ptr<SNPacket>> mInputQueue;
    SNConcurQueue<std::unique_ptr<SNFrame>> mOutputQueue;
    std::atomic_bool mDecoderEos{false};
    std::atomic_bool mInputEos{false};
    std::mutex mWaitMutex;
    std::condition_variable mWaitCond;
    bool mHaveSendEosToDecoder{false};
    bool mNeedKeyFrame{true};
    int64_t keyPts{INT64_MIN};
  };


}// namespace Sivin
#endif