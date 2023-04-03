
#include "DeviceManager.h"
#include "base/SNRet.h"
#include "codec/DecoderFactory.h"
#include "codec/IDecoder.h"
#include <cassert>
#include <memory>
namespace Sivin {

  static std::unique_ptr<IDecoder> emptyDecoder{};


  DeviceManager::DecoderHandle *DeviceManager::getDecoderHandle(DeviceType type) {
    if (type == DeviceType::AUDIO) {
      return &mAudioDecoderHandle;
    } else if (type == DeviceType::VIDEO) {
      return &mVideoDecoderHandle;
    }
    return nullptr;
  }


  int DeviceManager::setup(const std::unique_ptr<SNStreamInfo> &streamInfo, DeviceType type, void *surface, uint64_t flags) {
    auto *decoderHandle = getDecoderHandle(type);
    if (decoderHandle == nullptr) {
      return -1;
    }
    if (decoderHandle->valid) {
      return 0;
    }

    if (decoderHandle->decoder) {
      //TODO:
    }

    decoderHandle->decoder = DecoderFactory::create();

    if (decoderHandle->decoder) {
      auto ret = decoderHandle->decoder->open(streamInfo, surface, flags);
      if (ret != SNRet::Status::SUCCESS) {
        //不成功，释放解码器
        decoderHandle->decoder = nullptr;
        return -1;
      }
      decoderHandle->valid = true;
    }
    return 0;
  }

  const std::unique_ptr<IDecoder> &DeviceManager::getDecoder(DeviceType type) const {
    if (type == DeviceType::VIDEO) {
      return mVideoDecoderHandle.decoder;
    } else if (type == DeviceType::AUDIO) {
      return mAudioDecoderHandle.decoder;
    }
    return emptyDecoder;
  }


  bool DeviceManager::isDecoderValid(DeviceType type) const {
    if (type == DeviceType::VIDEO) {
      return mVideoDecoderHandle.valid;
    } else if (type == DeviceType::AUDIO) {
      return mAudioDecoderHandle.valid;
    }
    return false;
  }

  bool DeviceManager::isAudioRenderValid() const {
    return mAudioRenderValid;
  }

  bool DeviceManager::isVideoRenderValid() const {
    return mVideoRenderValid;
  }

  void DeviceManager::invalidDevice(DeviceType type) {
    if (type == DeviceType::AUDIO) {
      if (mAudioDecoderHandle.decoder) {
        mAudioDecoderHandle.decoder->prePause();
      }
      if (mAudioRender) {
        mAudioRender->prePause();
        mAudioRender->mute(true);
      }
      mAudioDecoderHandle.valid = false;
      mAudioRenderValid = false;
    } else if (type == DeviceType::VIDEO) {
      if (mVideoDecoderHandle.decoder) {
        mVideoDecoderHandle.decoder->prePause();
      }
      mVideoDecoderHandle.valid = false;
      mVideoRenderValid = false;
    }
  }

  SNRet DeviceManager::sendPacket(std::unique_ptr<SNPacket> &packet, DeviceType type, uint64_t timeout) {
    auto decoderHandle = getDecoderHandle(type);
    if (decoderHandle == nullptr || !decoderHandle->valid) {
      return SNRet::Status::ERROR;
    }
    assert(decoderHandle->decoder);
    return decoderHandle->decoder->sendPacket(packet, timeout);
  }

  SNRet DeviceManager::getFrame(std::unique_ptr<SNFrame> &frame, DeviceType type, uint64_t timeout) {
    auto decoderHandle = getDecoderHandle(type);
    if (decoderHandle == nullptr || !decoderHandle->valid) {
      return SNRet::Status::ERROR;
    }
    assert(decoderHandle->decoder);
    return decoderHandle->decoder->getFrame(frame, timeout);
  }

}// namespace Sivin