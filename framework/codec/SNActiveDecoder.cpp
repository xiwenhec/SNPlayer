#include "SNActiveDecoder.h"
#include "base/media/SNFrame.h"
#include "base/media/SNPacket.h"
#include "codec/IDecoder.h"
#include "utils/SNLog.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"
#include <chrono>
#include <mutex>
#include <utility>

#define MAX_OUTPUT_SIZE 10
#define MAX_INPUT_SIZE 16

namespace Sivin {

  SNActiveDecoder::SNActiveDecoder() : mInputQueue(MAX_INPUT_SIZE), mOutputQueue(MAX_OUTPUT_SIZE) {
  }

  SNActiveDecoder::~SNActiveDecoder() {
  }

  DecodeRet SNActiveDecoder::open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) {
    auto ret = initDecoder(streamInfo, surface, flags);
    if (ret != DecodeRet::SUCCESS) {
      close();
      return ret;
    }
    mRunning = true;
    mDeocdeThread = MAKE_UNIQUE_THREAD(decodeFunc, "decodeThread");
    mDeocdeThread->start();
    return DecodeRet::SUCCESS;
  }

  void SNActiveDecoder::close() {
  }


  int SNActiveDecoder::decodeFunc() {
    if (mDecoderEos) {
      SNTimer::sleepUs(10000);
      return 0;
    }

    int needWait = 0;
    while (!mInputQueue.empty() && mOutputQueue.size() < mMaxOutQueueSize && mRunning) {
      int ret = extractDecoder();
      needWait = 0;
      if (ret == 0) {
        needWait++;
      }

      //获取一个待解码的数据包，这里并没有
      auto &packet = *(mInputQueue.peek());
      if (!packet) {
        mInputQueue.pop();
        continue;
      }

      //送入解码器解码
      auto retStatus = enqueueDecoder(packet);
      if (retStatus == DecodeRet::AGAIN) {
        needWait++;
      } else {
        mInputQueue.pop();
        if (retStatus == DecodeRet::EOS) {
          mDecoderEos = true;
        } else {
          //TODO:处理解码失败
        }
      }

      //表示需要等待解码器
      if (needWait > 0) {
        std::unique_lock<std::mutex> waitLock{mWaitMutex};
        mWaitCond.wait_for(waitLock, std::chrono::milliseconds(5 * needWait), [this] { return !mRunning; });
      }
    }//while end

    if (mInputEos && mInputQueue.empty()) {
      if (!mHaveSendEosToDecoder) {
        std::unique_ptr<SNPacket> pPacket{};
        auto retStatus = enqueueDecoder(pPacket);

        if (retStatus != DecodeRet::AGAIN) {
          mHaveSendEosToDecoder = true;
        }
        if (retStatus == DecodeRet::EOS) {
          mDecoderEos = true;
        }
      }
      extractDecoder();
    }

    if (needWait > 0) {
      std::unique_lock<std::mutex> waitLock{mWaitMutex};
      mWaitCond.wait_for(waitLock, std::chrono::milliseconds(5), [this] { return !mRunning; });
    }

    return 0;
  }


  int SNActiveDecoder::extractDecoder() {
    DecodeRet ret;
    int count = 0;
    while (mOutputQueue.size_approx() < mMaxOutQueueSize && mRunning) {
      std::unique_ptr<SNFrame> frame{};
      ret = dequeueDecoder(frame);
      if (ret != DecodeRet::SUCCESS) {
        if (ret == DecodeRet::EOS) {
          SN_LOGD("decoder out put eos");
          mDecoderEos = true;
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

  int SNActiveDecoder::sendPacket(std::unique_ptr<SNPacket> &packet, uint64_t timeout) {
    if (needDrop(packet)) {
    }
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
      
    }
  }


}// namespace Sivin