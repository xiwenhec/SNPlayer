#include "SNActiveDecoder.h"
#include "base/SNRet.h"
#include "base/media/SNFrame.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "codec/IDecoder.h"
#include "utils/SNLog.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#define MAX_OUTPUT_SIZE 10
#define MAX_INPUT_SIZE 16

static std::thread::id empty_tid;

#define THREAD_CHECK                                         \
  {                                                          \
    if (mSourceThreadId == empty_tid) {                      \
      mSourceThreadId = std::this_thread::get_id();          \
    } else {                                                 \
      assert(mSourceThreadId == std::this_thread::get_id()); \
    }                                                        \
  }

namespace Sivin {

  SNActiveDecoder::SNActiveDecoder() : mMaxInQueueSize(MAX_INPUT_SIZE), mInputQueue(mMaxInQueueSize),
                                       mMaxOutQueueSize(MAX_OUTPUT_SIZE), mOutputQueue(mMaxOutQueueSize) {
  }

  SNActiveDecoder::~SNActiveDecoder() {
  }

  SNRet SNActiveDecoder::open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) {
    mSourceThreadId = empty_tid;
    mCodecId = streamInfo->codeId;
    auto ret = initDecoder(streamInfo, surface, flags);
    if (ret != SNRet::Status::SUCCESS) {
      close();
      return ret;
    }
    SN_LOGI("init code codec success.");
    mRunning = true;
    mDecodeThread = MAKE_UNIQUE_THREAD(decodeFunc, "decodeThread");
    mDecodeThread->start();
    return SNRet::Status::SUCCESS;
  }

  void SNActiveDecoder::close() {
    THREAD_CHECK;

    mRunning = false;
    mWaitCond.notify_one();

    if (mDecodeThread) {
      mDecodeThread->pause();
    }

    closeDecoder();

    while (!mInputQueue.empty()) {
      mInputQueue.pop();
    }

    while (!mOutputQueue.empty()) {
      mOutputQueue.pop();
    }
  }


  int SNActiveDecoder::decodeFunc() {
    if (mDecoderEos) {
      SNTimer::sleepUs(10000);
      return 0;
    }

    int needWait = 0;

    while (!mInputQueue.empty() && mOutputQueue.size() < mMaxOutQueueSize && mRunning) {
      //从解码器里抽取出所有解码完成的数据，放入framequeue中
      int ret = extractDecoder();
      needWait = 0;
      if (ret == 0) {
        needWait++;
      }

      auto &packet = *(mInputQueue.peek());
      if (!packet) {
        mInputQueue.pop();
        continue;
      }

      //送入解码器解码
      auto retStatus = enqueueDecoder(packet);
      if (retStatus == SNRet::Status::AGAIN) {
        needWait++;
      } else {
        mInputQueue.pop();
        if (retStatus == SNRet::Status::EOS) {
          mDecoderEos = true;
        } else {
          //TODO:处理解码失败
        }
      }

      //需要等待解码器
      if (needWait > 0) {
        std::unique_lock<std::mutex> waitLock{mWaitMutex};
        mWaitCond.wait_for(waitLock, std::chrono::milliseconds(5 * needWait), [this] { return !mRunning; });
      }
    }//while end

    if (mInputEos && mInputQueue.empty()) {
      if (!mHaveSendEosToDecoder) {
        std::unique_ptr<SNPacket> pPacket{};
        auto retStatus = enqueueDecoder(pPacket);

        if (retStatus != SNRet::Status::AGAIN) {
          mHaveSendEosToDecoder = true;
        }
        if (retStatus == SNRet::Status::EOS) {
          mDecoderEos = true;
        }
      }
      extractDecoder();
    }

    if (needWait == 0) {
      std::unique_lock<std::mutex> waitLock{mWaitMutex};
      mWaitCond.wait_for(waitLock, std::chrono::milliseconds(5), [this] { return !mRunning; });
    }

    return 0;
  }


  int SNActiveDecoder::extractDecoder() {
    SNRet ret;
    int count = 0;
    while (mOutputQueue.size_approx() < mMaxOutQueueSize && mRunning) {
      std::unique_ptr<SNFrame> frame{};
      ret = dequeueDecoder(frame);
      if (ret != SNRet::Status::SUCCESS) {
        if (ret == SNRet::Status::EOS) {
          SN_LOGD("decoder out put eos");
          mDecoderEos = true;
        } else if (ret == SNRet::Status::AGAIN) {
          return 0;
        } else {
          SN_LOGE("extractDecoder error");
        }
        return 0;
      }
      if (frame) {
        mOutputQueue.enqueue(std::move(frame));
        count++;
      }
    }
    return count;
  }

  SNRet SNActiveDecoder::sendPacket(std::unique_ptr<SNPacket> &packet, uint64_t timeout) {

    THREAD_CHECK;

    if (needDrop(packet)) {
      packet = nullptr;
      return SNRet::Status::SUCCESS;
    }

    if (mInputEos) {
      return SNRet::Status::EOS;
    }

    if (packet == nullptr) {
      mInputEos = true;
      mWaitCond.notify_one();
      return SNRet::Status::SUCCESS;
    }

    if (mHolding) {
      //新的GOP开始，清楚掉之前的缓存
      if (packet->getInfo().flags & SN_PKT_FLAG_KEY) {
        {
          //TODO:如果能够确保和holdOn调用在同一个线程，这里可以不用加锁
          // std::unique_lock<std::mutex> lock{mHoldMutex};
          while (!mHoldingQueue.empty()) {
            mHoldingQueue.pop();
          }
        }

        //刷新解码器
        flushDecoder();
      }
      mHoldingQueue.push(std::move(packet));
      return SNRet::Status::SUCCESS;
    }

    if ((mInputQueue.size() >= mMaxInQueueSize) || (mOutputQueue.size() >= mMaxOutQueueSize)) {
      return SNRet::Status::AGAIN;
    } else {
      mInputQueue.enqueue(std::move(packet));
      mWaitCond.notify_one();
    }
    return SNRet::Status::SUCCESS;
  }

  bool SNActiveDecoder::needDrop(std::unique_ptr<SNPacket> &packet) {
    if (packet == nullptr) {
      return false;
    }
    if (packet->getInfo().flags & SN_PKT_FLAG_CORRUPT) {
      return true;
    }
    if (mNeedKeyFrame) {
      if ((packet->getInfo().flags & SN_PKT_FLAG_KEY) == 0) {
        return true;
      } else {
        mNeedKeyFrame = false;
        keyPts = packet->getInfo().pts;
        return false;
      }
    } else if (packet->getInfo().flags & SN_PKT_FLAG_KEY) {
      //TODO:第二个关键帧到来后，不需要下面在做检测？
      keyPts = INT64_MIN;
    }

    if (packet->getInfo().pts != INT64_MIN && packet->getInfo().pts < keyPts) {
      SN_LOGW("key pts is %lld,pts is %lld\n", keyPts, packet->getInfo().pts);
      return true;
    }

    return false;
  }

  SNRet SNActiveDecoder::getFrame(std::unique_ptr<SNFrame> &frame, uint64_t timeout) {
    THREAD_CHECK;
    if (!mOutputQueue.empty()) {
      frame = std::move(*mOutputQueue.peek());
      mOutputQueue.pop();
      return SNRet::Status::SUCCESS;
    } else if (mDecoderEos) {
      return SNRet::Status::EOS;
    }
    return SNRet::Status::AGAIN;
  }

  void SNActiveDecoder::prePause() {
    mRunning = false;
    mWaitCond.notify_one();
    if (mDecodeThread) {
      mDecodeThread->tryPause();
    }
  }

  void SNActiveDecoder::pause(bool pause) {
    if (pause) {
      mRunning = false;
      mWaitCond.notify_one();
      if (mDecodeThread) {
        mDecodeThread->pause();
      }
    } else {
      mRunning = true;
      if (mDecodeThread) {
        mDecodeThread->start();
      }
    }
  }

  int SNActiveDecoder::holdOn(bool hold) {
    THREAD_CHECK;

    if (mHolding) {
      return 0;
    }

    if (hold) {
      mRunning = false;
      if (mDecodeThread) {
        mDecodeThread->pause();
      }
      {
        //TODO：如果能够确保和sendPacket函数在同一线程，这里可以不用加锁
        // std::unique_lock<std::mutex> lock{mHoldMutex};
        while (!mInputQueue.empty()) {
          auto &packet = *mInputQueue.peek();
          packet->setDiscard(true);
          mHoldingQueue.push(std::move(packet));
          mInputQueue.pop();
        }
      }

      while (!mOutputQueue.empty()) {
        mOutputQueue.pop();
      }

    } else {
      if (mMaxInQueueSize - mInputQueue.size() < mHoldingQueue.size()) {
        SN_LOGW("mHoldingQueue too big:(%lld), please increase iput queue size", mHoldingQueue.size());
      }
      while (!mHoldingQueue.empty()) {
        auto &packet = mHoldingQueue.front();
        mInputQueue.enqueue(std::move(packet));
        mHoldingQueue.pop();
      }
    }
    mHolding = hold;
    mRunning = true;

    if (mDecodeThread) {
      mDecodeThread->start();
    }
    return 0;
  }

  void SNActiveDecoder::flush() {

    THREAD_CHECK;

    bool threadRuning = false;
    if (mDecodeThread) {
      threadRuning = mDecodeThread->getStatus() == SNThread::Status::RUNNING;
    }
    mRunning = false;
    if (mDecodeThread) {
      mDecodeThread->pause();
    }

    while (!mInputQueue.empty()) {
      mInputQueue.pop();
    }

    while (!mOutputQueue.empty()) {
      mOutputQueue.pop();
    }

    while (!mHoldingQueue.empty()) {
      mHoldingQueue.pop();
    }

    flushDecoder();
    mInputEos = false;
    mDecoderEos = false;
    mHaveSendEosToDecoder = false;
    if (threadRuning) {
      mRunning = true;
    }
    mNeedKeyFrame = true;
    if (threadRuning) {
      mDecodeThread->start();
    }
  }

  uint32_t SNActiveDecoder::getInputQueueSize() {
    return mInputQueue.size();
  }
}// namespace Sivin