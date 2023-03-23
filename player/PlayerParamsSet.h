//
// Created by sivin on 23-1-15.
//

#ifndef SIVINPLAYER_PLAYERPARAMSSET_H
#define SIVINPLAYER_PLAYERPARAMSSET_H

#include "string"


namespace Sivin {

  /**
   * 用于缓存播放器的一些内部状态变量
   */
  class PlayerParamsSet {
  public:
    PlayerParamsSet();

    ~PlayerParamsSet();

    void reset();

  public:

    std::string url{};

    int timeoutMs{15000};


  };

} // Sivin

#endif //SIVINPLAYER_PLAYERPARAMSSET_H
