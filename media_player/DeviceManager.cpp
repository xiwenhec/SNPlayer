
#include "DeviceManager.h"
#include "codec/IDecoder.h"
#include <cassert>
#include <memory>
namespace Sivin {

  static std::unique_ptr<IDecoder> emptyDecoder{};

  const std::unique_ptr<IDecoder> &DeviceManager::getDecoder(DeviceType type) const {
    if (type == DeviceType::VIDEO) {
      return mVideoDecoderHandle.decoder;
    } else if (type == DeviceType::AUDIO) {
      return mAudioDecoderHandle.decoder;
    }
    return emptyDecoder;
  }

  DeviceManager::DecoderHandle *DeviceManager::getDecoderHandle(DeviceType type) {
    if (type == DeviceType::AUDIO) {
      return &mAudioDecoderHandle;
    } else if (type == DeviceType::VIDEO) {
      return &mVideoDecoderHandle;
    }
    return nullptr;
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