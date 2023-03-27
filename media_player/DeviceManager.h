#ifndef SIVINPLAYER_DEVICEMANAGER_H
#define SIVINPLAYER_DEVICEMANAGER_H

#include "codec/IDecoder.h"
#include <mutex>


namespace Sivin {

  enum class DeviceType {
    DEVICE_VIDEO = 1,
    DEVICE_AUDIO = 1 << 1,
    DEVICE_ADVD = DEVICE_VIDEO | DEVICE_AUDIO,
  };

  class DeviceManager {

  public:
    explicit DeviceManager();
    ~DeviceManager();

  public:
    IDecoder *getDecoder(DeviceType type) const;
    bool isDecoderValid(DeviceType type) const;
    bool isAudioRenderValid() const;
    bool invalidDevice(DeviceType type);
    bool fluchDevice(DeviceType type);
    int getFrame(std::unique_lock<SNFrame>)
  };

}// namespace Sivin
#endif// SIVINPLAYER_DEVICEMANAGER_H