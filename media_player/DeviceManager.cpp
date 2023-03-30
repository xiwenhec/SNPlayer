
#include "DeviceManager.h"
#include "codec/IDecoder.h"
#include <memory>
namespace Sivin {

  static std::unique_ptr<IDecoder> emptyDecoder{};

  const std::unique_ptr<IDecoder> &DeviceManager::getDecoder(DeviceType type) const {
    if (type == DeviceType::VIDEO) {
      return mVideoDecodeHandle.decoder;
    } else if (type == DeviceType::AUDIO) {
      return mAudioDecodeHandle.decoder;
    }
    return emptyDecoder;
  }

  bool DeviceManager::isDecoderValid(DeviceType type) const {
    if (type == DeviceType::VIDEO) {
      return mVideoDecodeHandle.valid;
    } else if (type == DeviceType::AUDIO) {
      return mAudioDecodeHandle.valid;
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
    if (type == DeviceType::AUDIO || type == DeviceType::ADVD) {
      if (mAudioDecodeHandle.decoder) {
        mAudioDecodeHandle.decoder->prePause();
      }
      
    }
  }

}// namespace Sivin