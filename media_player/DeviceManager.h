#ifndef SIVINPLAYER_DEVICEMANAGER_H
#define SIVINPLAYER_DEVICEMANAGER_H

#include "codec/IDecoder.h"


namespace Sivin {

  enum class DeviceType {
    DEVICE_VIDEO = 1,
    DEVICE_AUDIO = 1 << 1,
  };

  class DeviceManager {

  public:
    explicit DeviceManager();
    ~DeviceManager();
    IDecoder *getDecoder(DeviceType type) const;
  };

}// namespace Sivin
#endif// SIVINPLAYER_DEVICEMANAGER_H