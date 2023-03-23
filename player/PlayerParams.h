//
// Created by sivin on 23-1-15.
//

#ifndef SIVINPLAYER_PLAYERPARAMS_H
#define SIVINPLAYER_PLAYERPARAMS_H

#include "string"


namespace Sivin {

  /**
   * 用于缓存播放器的一些内部状态变量
   */
  class PlayerParams {
  public:
    PlayerParams();

    ~PlayerParams();

    void reset();

  public:

    std::string url{};

    int timeoutMs{15000};


  };

} // Sivin

#endif //SIVINPLAYER_PLAYERPARAMS_H
