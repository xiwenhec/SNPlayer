#ifndef SIVINPLAYER_IMEDIAPLAYER_H
#define SIVINPLAYER_IMEDIAPLAYER_H

#include "MediaPlayerDef.h"
#include <string>


namespace Sivin {

  class IMediaPlayer {

  public:
    IMediaPlayer() = default;

    virtual ~IMediaPlayer() = default;

    virtual std::string getName() = 0;

    virtual void setListener(const PlayerListener &listener) = 0;

    // virtual void setOnVideoRenderCallback(OnRenderFrameCallback callback, void *userData) = 0;

    // virtual void setOnAudioRenderCallback(OnRenderFrameCallback callback, void *userData) = 0;

    virtual void setView(void *view) = 0;

    virtual void setDataSource(const char *url) = 0;

    virtual void prepare() = 0;

    virtual void start() = 0;

    virtual void pause() = 0;

    virtual void seekTo(int64_t seekPos, bool isAccurate) = 0;

    virtual void stop() = 0;
  };

};// namespace Sivin

#endif// SIVINPLAYER_IMEDIAPLAYER_H