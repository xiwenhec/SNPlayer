//
// Created by sivin on 23-1-15.
//

#ifndef SIVINPLAYER_PLAYERPARAMS_H
#define SIVINPLAYER_PLAYERPARAMS_H

#include "string"


namespace Sivin {

  /**
   * 用于缓存播放器参数的类
   */
  class PlayerParams {
  public:
    PlayerParams();

    ~PlayerParams();

    void reset();

  public:
    std::string url{};

    int timeoutMs{15000};

    bool loopPlay{false};

    bool disableAudio{false};

    bool disableVideo{false};

    bool mute{false};

    uint64_t lowMemSize{0};

    //起播缓冲时长,单位(ms)
    int64_t startBufferDuration{0};
    //播放器最大缓冲时长,单位(ms)
    int64_t maxBufferDuration{0};

    //以音频优先，如果音频比视频短，则播放到音频结束后就停止。
    bool preferAudio{false};

  };

}// namespace Sivin

#endif//SIVINPLAYER_PLAYERPARAMS_H
