#ifndef SIVINPLAYER_DEVICEMANAGER_H
#define SIVINPLAYER_DEVICEMANAGER_H

#include "base/media/SNFrame.h"
#include "base/media/SNPacket.h"
#include "codec/IDecoder.h"
#include <cstdint>
#include <memory>
#include <mutex>


#define DEVICE_STATUS_EOS -10
#define STATUS_RETRY_IN -11
namespace Sivin {


  enum class DeviceType {
    VIDEO = 1,
    AUDIO = 1 << 1,
    ADVD = VIDEO | AUDIO
  };


  class DeviceManager {
  public:
    struct DecoderHandle {
      std::unique_ptr<IDecoder> decoder;
      bool valid{false};
    };

  public:
    explicit DeviceManager();
    ~DeviceManager();

  public:
    const std::unique_ptr<IDecoder> &getDecoder(DeviceType type) const;

    bool isDecoderValid(DeviceType type) const;

    bool isVideoRenderValid() const;
    
    bool isAudioRenderValid() const;
    
    void invalidDevice(DeviceType type);
    bool fluchDevice(DeviceType type);

    int getFrame(std::unique_ptr<SNFrame> &frame, DeviceType type, uint64_t timeout);

    int sendPacket(std::unique_ptr<SNPacket> &packet, DeviceType type, uint64_t timeout);

    int renderVideoFrame(std::unique_ptr<SNFrame> &frame);

    void setVoluem(float volume);
    void setMute(bool mute);


  private:
    DecoderHandle mVideoDecodeHandle;
    DecoderHandle mAudioDecodeHandle;

    bool mAudioRenderValid{false};
    bool mVideoRenderValid{false};
  };

}// namespace Sivin
#endif// SIVINPLAYER_DEVICEMANAGER_H