//
// Created by sivin on 1/2/23.
//

#ifndef SN_FRAMEWORK_IDECODER_H
#define SN_FRAMEWORK_IDECODER_H
#include "base/media/SNFrame.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "base/SNRet.h"
#include <cstdint>
#include <string>
#include <memory>

namespace Sivin {


  class IDecoder {
  public:
    explicit IDecoder() = default;
    virtual ~IDecoder() = default;

  public:
    virtual SNRet open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) = 0;

    virtual void close() = 0;

    virtual SNRet sendPacket(std::unique_ptr<SNPacket> &packet, uint64_t timeout) = 0;

    virtual SNRet getFrame(std::unique_ptr<SNFrame> &frame, uint64_t timeout) = 0;

    virtual void pause(bool pause) = 0;

    virtual void prePause() = 0;

    virtual int holdOn(bool hold) = 0;

    virtual void flush() = 0;

    virtual uint32_t getInputQueueSize() = 0;

  protected:
    std::string name;
    int mFlags{0};
  };
}// namespace Sivin


#endif//SN_FRAMEWORK_IDECODER_H
