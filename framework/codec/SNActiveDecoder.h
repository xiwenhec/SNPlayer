#ifndef SN_FRAMEWORK_SNACTIVEDECODER_H
#define SN_FRAMEWORK_SNACTIVEDECODER_H

#include "codec/IDecoder.h"
#include "utils/SNThread.h"
#include "utils/SNConcurQueue.h"
#include <condition_variable>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace Sivin {

  class SNActiveDecoder : public IDecoder {
  public:
    explicit SNActiveDecoder();

    virtual ~SNActiveDecoder();

    SNRetStatus open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) override;
    void close() override;

    SNRetStatus sendPacket(std::unique_ptr<SNPacket> &packet, uint64_t timeout) override;

    SNRetStatus getFrame(std::unique_ptr<SNFrame> &frame, uint64_t timeout) override;

    void pause(bool pause) override;

    void prePause() override;

    int holdOn(bool hold) override;

    void flush() override;

    uint32_t getInputQueueSize() override;


  private:
    int decodeFunc();

    int extractDecoder();

    bool needDrop(std::unique_ptr<SNPacket> &packet);

  private:
    virtual SNRetStatus initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) = 0;

    virtual SNRetStatus enqueueDecoder(std::unique_ptr<SNPacket> &pPacket) = 0;

    virtual SNRetStatus dequeueDecoder(std::unique_ptr<SNFrame> &frame) = 0;

    virtual void flushDecoder() = 0;

  protected:
    std::unique_ptr<SNThread> mDecodeThread{nullptr};
    std::atomic_bool mRunning{false};

  private:
    //这个api用来检测所有调用改decoder的api都应该来自同一个线程
    std::thread::id mSourceThreadId;

    SNCodecID mCodecId;

    int mMaxOutQueueSize;
    int mMaxInQueueSize;
    SNConcurQueue<std::unique_ptr<SNPacket>> mInputQueue;
    SNConcurQueue<std::unique_ptr<SNFrame>> mOutputQueue;

    std::atomic_bool mDecoderEos{false};
    std::atomic_bool mInputEos{false};

    std::mutex mWaitMutex;
    std::condition_variable mWaitCond;
    bool mHaveSendEosToDecoder{false};
    bool mNeedKeyFrame{true};
    int64_t keyPts{INT64_MIN};


    //移动端APP进入后台，不需要解码画面，缓存一个GOP的数据
    //一边进入前台后能快速解码
    std::mutex mHoldMutex;
    std::queue<std::unique_ptr<SNPacket>> mHoldingQueue{};
    std::atomic_bool mHolding{false};
  };


}// namespace Sivin
#endif