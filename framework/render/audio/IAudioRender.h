#ifndef SN_FRAMEWORK_IAUDIORENDER_H
#define SN_FRAMEWORK_IAUDIORENDER_H

#include "base/media/SNFrame.h"
namespace Sivin {

  class IAudioRender {
  public:
    explicit IAudioRender() = default;
    virtual ~IAudioRender() = default;

  public:
    virtual int init(const SNFrame::AudioInfo *info) = 0;

    virtual int renderFrame(std::unique_ptr<SNFrame> &frame, int timeOut) = 0;

    virtual int64_t getPosition() = 0;

    virtual void mute(bool bMute) = 0;

    virtual int setVolume(float volume) = 0;

    virtual int setSpeed(float speed) = 0;

    virtual void prePause() = 0;

    virtual void pause(bool bPause) = 0;

    virtual void flush() = 0;

    virtual uint64_t getQueDuration() = 0;
  };


}// namespace Sivin

#endif